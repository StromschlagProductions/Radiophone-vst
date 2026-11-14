// RadiophoneDSP.h - Klang-Engine fuer "Radiophone" (Stromschlag Audio)
// Reines C++ ohne JUCE, damit die Engine unabhaengig vom Plugin getestet werden kann.
//
// Signalweg (mono, das Plugin legt das Ergebnis auf beide Seiten):
//   Eingang -> Vorlauf (80 ms) -> Klangkette (Bandbreite, Kompressor, Saettigung) -> Sende-/Leitungspegel -> +
//   Geraeusche (Squelch, Rauschen, Knacken, Telefontoene) -> Leitungsfilter -> Geraeusche-Pegel -> +  -> Ausgang
//
// Der Vorlauf sorgt dafuer, dass das Oeffnungsgeraeusch beim Funk VOR dem ersten Wort liegt. Der Host gleicht
// die Verzoegerung automatisch aus (Plugin-Latenz).
//
// Saettigung: Sinus-Kennlinie nach Airwindows "Density" von Chris Johnson (MIT-Lizenz, github.com/airwindows).
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace rph
{
constexpr float kPi = 3.14159265358979f;
constexpr float kHalfPi = 1.57079633f;

inline float dbToLin (float db) { return db <= -79.0f ? 0.0f : std::pow (10.0f, db * 0.05f); }

//==============================================================================
// Biquad (RBJ-Kochbuch), Transposed Direct Form II
struct Biquad
{
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;

    void reset() { z1 = z2 = 0; }
    float process (float x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return (float) y;
    }
    void setLowpass (double fs, double f, double q)  { calc (fs, f, q, 0, 0); }
    void setHighpass (double fs, double f, double q) { calc (fs, f, q, 0, 1); }
    void setBandpass (double fs, double f, double q) { calc (fs, f, q, 0, 2); }
    void setPeak (double fs, double f, double q, double gainDb) { calc (fs, f, q, gainDb, 3); }

private:
    void calc (double fs, double f, double q, double gainDb, int type)
    {
        f = std::clamp (f, 10.0, fs * 0.45);
        const double w = 2.0 * 3.14159265358979 * f / fs, c = std::cos (w), s = std::sin (w);
        const double alpha = s / (2.0 * q);
        double B0, B1, B2, A0, A1, A2;
        if (type == 0)      { B0 = (1 - c) / 2; B1 = 1 - c; B2 = (1 - c) / 2; A0 = 1 + alpha; A1 = -2 * c; A2 = 1 - alpha; }
        else if (type == 1) { B0 = (1 + c) / 2; B1 = -(1 + c); B2 = (1 + c) / 2; A0 = 1 + alpha; A1 = -2 * c; A2 = 1 - alpha; }
        else if (type == 2) { B0 = alpha; B1 = 0; B2 = -alpha; A0 = 1 + alpha; A1 = -2 * c; A2 = 1 - alpha; }
        else
        {
            const double A = std::pow (10.0, gainDb / 40.0);
            B0 = 1 + alpha * A; B1 = -2 * c; B2 = 1 - alpha * A;
            A0 = 1 + alpha / A; A1 = -2 * c; A2 = 1 - alpha / A;
        }
        b0 = B0 / A0; b1 = B1 / A0; b2 = B2 / A0; a1 = A1 / A0; a2 = A2 / A0;
    }
};

// Bandbegrenzung: Hochpass + Tiefpass, je 4. Ordnung (zwei Biquads), dazu eine Anhebung (Praesenz/Lautsprecher)
struct Band
{
    Biquad hp1, hp2, lp1, lp2, peak, body;
    void set (double fs, float hpHz, float lpHz, float peakHz, float peakDb, float bodyHz = 0, float bodyDb = 0)
    {
        hp1.setHighpass (fs, hpHz, 0.54); hp2.setHighpass (fs, hpHz, 1.31);
        lp1.setLowpass (fs, lpHz, 0.54);  lp2.setLowpass (fs, lpHz, 1.31);
        peak.setPeak (fs, peakHz, 1.1, peakDb);
        body.setPeak (fs, bodyHz > 0 ? bodyHz : 500.0, 1.4, bodyHz > 0 ? bodyDb : 0.0);
    }
    void reset() { hp1.reset(); hp2.reset(); lp1.reset(); lp2.reset(); peak.reset(); body.reset(); }
    float process (float x) { return body.process (peak.process (lp2.process (lp1.process (hp2.process (hp1.process (x)))))); }
};

// Zufall (xorshift), reproduzierbar: gleiche Wiedergabe = gleiches Ergebnis
struct Rng
{
    uint32_t s = 0x9E3779B9u;
    void seed (uint32_t v) { s = v ? v : 0x9E3779B9u; }
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float uni() { return (float) (next() >> 8) * (1.0f / 16777216.0f); }   // 0 .. 1
    float bi() { return uni() * 2.0f - 1.0f; }                              // -1 .. 1
};

// Airwindows Density (Chris Johnson, MIT): Sinus-Saettigung, ganze Stufen voll, Rest gemischt
inline float density (float x, float amount)
{
    auto stage = [] (float v)
    {
        float b = std::min (std::abs (v) * kHalfPi, kHalfPi);
        b = std::sin (b);
        return v > 0.0f ? b : -b;
    };
    while (amount > 1.0f) { x = stage (x); amount -= 1.0f; }
    return x * (1.0f - amount) + stage (x) * amount;
}

//==============================================================================
// Modi und Auswahllisten (Reihenfolge = Reihenfolge in den Auswahllisten des Plugins)
enum Mode { kFunk = 0, kRadio = 1, kTelefon = 2 };
enum Device { kWalkie = 0, kAutofunk = 1, kTischfunk = 2 };
enum Key { kKeyAuto = 0, kKeyDown = 1, kKeyUp = 2 };
enum Line { kGespraech = 0, kAufgelegt = 1, kWaehlton = 2, kWaehlen = 3, kFreizeichen = 4, kBesetzt = 5 };
enum DialType { kScheibe = 0, kTasten = 1 };

// Klang je Geraet (fest abgestimmt, keine Regler)
struct Voicing
{
    float hpHz, lpHz;          // Bandbreite
    float peakHz, peakDb;      // Praesenz (Mikrofon)
    float bodyHz, bodyDb;      // kleiner Lautsprecher
    float compThrDb, ratio;    // Kompressor (Funk-/Sendekompression)
    float drive;               // Saettigung (Density-Stufen)
    float makeupDb;            // Ausgleich, damit es etwa so laut bleibt wie vorher
    float hissDb;              // Grundrauschen (Kanal / Sender / Leitung)
};

inline const Voicing& voicing (int mode, int device)
{
    static const Voicing funk[3] = {
        //  hp    lp     peak  dB   body  dB   thr    ratio drive makeup  hiss
        { 480, 2700,  1600, 6.0f,  900, 2.0f, -26.0f, 4.0f, 2.2f, -1.9f, -40.0f },   // Walkie-Talkie
        { 360, 3100,  1400, 4.5f,  600, 2.0f, -24.0f, 3.5f, 1.6f, -1.5f, -44.0f },   // Autofunk
        { 260, 3600,  1300, 3.5f,  380, 3.0f, -22.0f, 3.0f, 1.2f, -1.9f, -47.0f },   // Tischfunk
    };
    static const Voicing radio = { 190, 4600, 1100, 3.0f, 420, 3.0f, -24.0f, 3.0f, 1.25f, -0.8f, -48.0f };
    static const Voicing phone = { 380, 3300, 2000, 5.0f, 900, 1.5f, -24.0f, 3.0f, 1.5f, -1.3f, -62.0f };
    if (mode == kRadio) return radio;
    if (mode == kTelefon) return phone;
    return funk[std::clamp (device, 0, 2)];
}

struct Params
{
    int mode = kFunk;
    int device = kWalkie;
    int key = kKeyAuto;
    float pauseSec = 0.6f;     // Automatik: so lange Stille, dann gilt die Taste als losgelassen
    float sfxDb = 0.0f;        // Lautstaerke aller eingebauten Geraeusche
    float gate = 0.0f;         // Funk: 0 = aus, 1 = stark abgehackt
    int tuneCount = 0;         // Radio: jede Aenderung = ein Senderwechsel
    int line = kGespraech;     // Telefon: Zustand der Leitung
    int dialType = kScheibe;
};

//==============================================================================
// Eingebaute Aufnahmen (optional). Ohne sie klingt alles synthetisch.
struct Samples
{
    double rate = 44100.0;                 // Abtastrate der Rohdaten
    std::vector<float> sqOpen[3], sqClose[3], tune;   // je Geraet: Oeffnen und Schliessen, dazu Senderwechsel
};

class Engine
{
public:
    static constexpr float kLookaheadSec = 0.08f;
    static constexpr float kSqSampleGain = 0.5f;     // Pegel der Squelch-Aufnahmen
    static constexpr float kTuneSampleGain = 0.9f;   // Pegel der Senderwechsel-Aufnahme

    // nach prepare() aufrufen: rechnet die Aufnahmen auf die Abtastrate der DAW um
    void setSamples (const Samples& in)
    {
        if (fs <= 0.0) return;
        auto conv = [&] (const std::vector<float>& v)
        {
            std::vector<float> o;
            if (v.empty()) return o;
            const double ratio = in.rate / fs;
            const size_t n = (size_t) std::max (1.0, std::floor ((double) v.size() / ratio));
            o.resize (n);
            for (size_t i = 0; i < n; ++i)
            {
                const double pos = (double) i * ratio;
                const size_t a = (size_t) pos, b = std::min (a + 1, v.size() - 1);
                const float f = (float) (pos - (double) a);
                o[i] = v[a] * (1.0f - f) + v[b] * f;
            }
            return o;
        };
        for (int d = 0; d < 3; ++d)
        {
            smp.sqOpen[d] = conv (in.sqOpen[d]);
            smp.sqClose[d] = conv (in.sqClose[d]);
        }
        smp.tune = conv (in.tune);
        hasSamples = true;
    }

    void prepare (double sampleRate)
    {
        fs = sampleRate;
        lookahead = (int) std::lround (kLookaheadSec * fs);
        delay.assign ((size_t) lookahead + 1, 0.0f);
        configured = false;
        reset();
    }

    int latencySamples() const { return lookahead; }
    bool isTransmitting() const { return txOpen; }
    bool isTuning() const { return tuneLeft > 0; }

    void reset()
    {
        std::fill (delay.begin(), delay.end(), 0.0f);
        dpos = 0;
        rng.seed (0x51A7u); dialRng.seed (0x0E7Bu);
        band.reset(); fxBand.reset(); sqBand.reset(); mechBand.reset();
        compEnv = 0; detEnv = 0; gateEnv = 0;
        voiceGain = 0; voiceTarget = 0; gateGain = 1;
        txOpen = false; silentSamples = 0; pendingClose = -1;
        firstBlock = true;
        squelch = {}; mech[0] = mech[1] = {};
        crackleLeft = 0; dipLeft = 0; dipLen = 1; tuneLeft = 0;
        toneT = 0; toneOn = false; dialStep = 0; dialTimer = 0; dialPulsesLeft = 0; dialDigitsLeft = 0;
        dtmfLeft = 0; clickLeft = 0;
        hissFlutter = 1.0f;
    }

    // mono: liest und schreibt buf
    void process (float* buf, int n, const Params& p)
    {
        configure (p);
        handleEvents (p);
        const Voicing& v = voicing (p.mode, p.device);
        const float makeup = dbToLin (v.makeupDb);
        const float sfx = dbToLin (p.sfxDb);
        const float hiss = dbToLin (v.hissDb);
        const float gateThr = p.gate > 0.001f ? dbToLin (-52.0f + 34.0f * p.gate) : 0.0f;
        const float compAtt = coef (0.004f), compRel = coef (0.120f);
        const float detCoef = coef (0.010f);
        const float gAtt = coef (0.0005f), gRel = coef (0.012f);
        const float rampOpen = 1.0f / (0.004f * (float) fs), rampClose = 1.0f / (0.002f * (float) fs);
        const float gateRamp = 1.0f / (0.0012f * (float) fs);
        const int pauseSamples = (int) (p.pauseSec * fs);
        const float detOpen = dbToLin (-44.0f), detQuiet = dbToLin (-50.0f);

        for (int i = 0; i < n; ++i)
        {
            const float in = buf[i];

            // --- Vorlauf: x ist das Signal, das jetzt ausgegeben wird (80 ms alt)
            const float x = delay[(size_t) dpos];
            delay[(size_t) dpos] = in;
            if (++dpos >= (int) delay.size()) dpos = 0;

            // --- Funk: Sprechtaste (Automatik sieht 80 ms in die Zukunft)
            if (p.mode == kFunk)
            {
                detEnv = in * in + detCoef * (detEnv - in * in);
                const float lvl = std::sqrt (detEnv);
                if (p.key == kKeyAuto)
                {
                    if (lvl > detOpen) { silentSamples = 0; if (! txOpen) openTx (p); }
                    else if (txOpen)
                    {
                        if (lvl < detQuiet) ++silentSamples; else silentSamples = 0;
                        if (silentSamples > pauseSamples) closeTx (p);
                    }
                }
                if (pendingClose >= 0 && --pendingClose < 0) closeTx (p);
            }

            // --- Klangkette
            float y = band.process (x);
            const float ay = std::abs (y);
            compEnv = ay + (ay > compEnv ? compAtt : compRel) * (compEnv - ay);
            const float envDb = 20.0f * std::log10 (compEnv + 1e-9f);
            const float over = envDb - v.compThrDb;
            if (over > 0.0f) y *= dbToLin (-over * (1.0f - 1.0f / v.ratio));
            y = density (y * 1.4f, v.drive) * makeup / 1.4f;

            // Pegel der Stimme (Sendetaste, Telefonzustand, Senderwechsel, Schwund beim Radio)
            voiceGain += std::clamp (voiceTarget - voiceGain, -rampClose, rampOpen);
            float vg = voiceGain;

            // --- Geraeusche, die durch den Leitungsfilter gehen
            float fx = 0.0f;
            float sq = 0.0f;   // Squelch hat einen eigenen Filter
            float mc = 0.0f;   // Mechanik (Hoerer) mit breiterem Filter

            if (p.mode == kFunk)
            {
                // Abhacken (Gate): wenn die Stimme leise wird, schneidet der Empfaenger hart ab
                if (gateThr > 0.0f)
                {
                    const float ax = std::abs (x);
                    gateEnv = ax + (ax > gateEnv ? gAtt : gRel) * (gateEnv - ax);
                    const float tgt = gateEnv > gateThr ? 1.0f : 0.0f;
                    gateGain += std::clamp (tgt - gateGain, -gateRamp, gateRamp);
                }
                else gateGain = 1.0f;
                vg *= gateGain;
                fx += rng.bi() * hiss * 2.0f * voiceGain * gateGain;   // Kanalrauschen nur waehrend gesendet wird
                sq = nextSquelch();
            }
            else if (p.mode == kRadio)
            {
                vg *= radioTick (fx, hiss);
            }
            else
            {
                fx += rng.bi() * hiss * 2.0f;
                fx += phoneTick();
                mc = nextMech();
            }

            const float out = y * vg
                            + fxBand.process (fx) * sfx
                            + (lastSqSample ? sq : density (sqBand.process (sq), 1.0f) * 0.75f) * sfx   // Aufnahmen ungefiltert
                            + mechBand.process (mc) * sfx;
            buf[i] = std::isfinite (out) ? out : 0.0f;
        }
    }

private:
    //==========================================================================
    float coef (float sec) const { return std::exp (-1.0f / (sec * (float) fs)); }

    void configure (const Params& p)
    {
        if (configured && p.mode == lastMode && p.device == lastDevice) return;
        const Voicing& v = voicing (p.mode, p.device);
        band.set (fs, v.hpHz, v.lpHz, v.peakHz, v.peakDb, v.bodyHz, v.bodyDb);
        fxBand.set (fs, v.hpHz, v.lpHz, v.peakHz, v.peakDb * 0.5f);
        // Squelch-Rauschen: gemessen an echten Aufnahmen (kurz und mittig bis lang und hell)
        if (p.device == kWalkie)        sqBand.set (fs, 600, 3000, 1500, 3.0f);
        else if (p.device == kAutofunk) sqBand.set (fs, 480, 1300, 850, 3.0f);
        else                            sqBand.set (fs, 400, 2200, 1100, 2.0f);
        mechBand.set (fs, 150, 6000, 1200, 2.0f);
        if (configured && p.mode != lastMode)
        {
            // Moduswechsel: alles Laufende stoppen, Zustand neu
            txOpen = false; pendingClose = -1; squelch = {}; tuneLeft = 0; dipLeft = 0;
            toneOn = false; dialDigitsLeft = 0; dialPulsesLeft = 0; dtmfLeft = 0;
            firstBlock = true;
        }
        lastMode = p.mode; lastDevice = p.device;
        configured = true;
    }

    //--------------------------------------------------------------------------
    // Parameter-Aenderungen in Ereignisse uebersetzen
    void handleEvents (const Params& p)
    {
        if (firstBlock)
        {
            // Beim Start keine Geraeusche ausloesen, nur den Zustand uebernehmen
            firstBlock = false;
            lastKey = p.key; lastTune = p.tuneCount; lastLine = p.line;
            if (p.mode == kFunk)
            {
                txOpen = (p.key == kKeyDown);
                voiceTarget = voiceGain = txOpen ? 1.0f : 0.0f;
            }
            else if (p.mode == kTelefon)
            {
                voiceTarget = voiceGain = (p.line == kGespraech) ? 1.0f : 0.0f;
                enterLine (p, false);
            }
            else voiceTarget = voiceGain = 1.0f;
            return;
        }

        if (p.mode == kFunk && p.key != lastKey)
        {
            if (p.key == kKeyDown && ! txOpen) { pendingClose = -1; openTx (p); }
            else if (p.key == kKeyUp && txOpen) pendingClose = lookahead;   // erst schliessen, wenn das letzte Wort durch ist
            else if (p.key == kKeyDown) pendingClose = -1;
            lastKey = p.key;
        }
        if (p.mode != kFunk) lastKey = p.key;

        if (p.mode == kRadio && p.tuneCount != lastTune) startTuning();
        lastTune = p.tuneCount;

        if (p.mode == kTelefon && p.line != lastLine)
        {
            const int prev = lastLine;
            lastLine = p.line;
            if (prev == kAufgelegt) startMech (false);
            else if (p.line == kAufgelegt) startMech (true);
            else clickLeft = (int) (0.004 * fs);   // Leitung schaltet um: kurzes Knacken
            enterLine (p, true);
        }
        if (p.mode != kTelefon) lastLine = p.line;
    }

    //--------------------------------------------------------------------------
    // FUNK
    struct Squelch { int pos = 0, len = 0, body = 0; bool closing = false; float level = 0; const std::vector<float>* src = nullptr; };
    Samples smp;
    bool hasSamples = false;
    bool lastSqSample = false;
    Squelch squelch;

    void openTx (const Params& p)
    {
        txOpen = true;
        voiceTarget = 1.0f;
        if (hasSamples && ! smp.sqOpen[p.device].empty())
        {
            squelch = {};
            squelch.src = &smp.sqOpen[p.device];
            squelch.len = (int) squelch.src->size();
            squelch.level = kSqSampleGain;
            silentSamples = 0;
            return;
        }
        // Oeffnen: kurzes "Tschk" (Klick + kurzer Rauschstoss)
        squelch.pos = 0;
        squelch.closing = false;
        squelch.body = (int) ((p.device == kAutofunk ? 0.035 : 0.045) * fs);
        squelch.len = squelch.body + (int) (0.012 * fs);
        squelch.level = p.device == kAutofunk ? 0.5f : 0.35f;
        silentSamples = 0;
    }

    void closeTx (const Params& p)
    {
        txOpen = false;
        pendingClose = -1;
        voiceTarget = 0.0f;
        // Schliessen: flacher Rauschstoss mit hartem Ende (das typische "Kschhh")
        if (hasSamples && ! smp.sqClose[p.device].empty())
        {
            squelch = {};
            squelch.src = &smp.sqClose[p.device];
            squelch.len = (int) squelch.src->size();
            squelch.level = kSqSampleGain;
            return;
        }
        const double body = p.device == kWalkie ? 0.30 : p.device == kAutofunk ? 0.085 : 0.17;
        squelch.pos = 0;
        squelch.closing = true;
        squelch.body = (int) (body * fs);
        squelch.len = squelch.body + (int) (0.045 * fs);
        // Pegel je Geraet so, dass das "Kschhh" etwa so laut ist wie die Stimme (schmaler Filter = mehr Pegel)
        squelch.level = p.device == kWalkie ? 0.7f : p.device == kAutofunk ? 1.6f : 0.6f;
    }

    float nextSquelch()
    {
        lastSqSample = squelch.src != nullptr;
        if (squelch.src)
            return squelch.pos < squelch.len ? (*squelch.src)[(size_t) squelch.pos++] * squelch.level : 0.0f;
        if (squelch.pos >= squelch.len) return 0.0f;
        const int i = squelch.pos++;
        const float t = (float) i / (float) fs;
        float env;
        if (i < squelch.body)
        {
            env = std::min (1.0f, t / 0.0015f);
            env *= 1.0f + 0.08f * std::sin (2.0f * kPi * 23.0f * t);   // leichtes Flattern
        }
        else
        {
            // hartes Abreissen (~12 ms), dann ein leiser Rest
            const float r = (float) (i - squelch.body) / (float) fs;
            env = std::exp (-r / 0.004f) + 0.03f * std::exp (-r / 0.02f);
        }
        float s = rng.bi() * env * squelch.level;
        if (i < (int) (0.0015 * fs)) s += (squelch.closing ? 0.25f : 0.5f) * (1.0f - t / 0.0015f);   // Klick am Anfang
        return s;
    }

    //--------------------------------------------------------------------------
    // RADIO
    int crackleLeft = 0;
    float crackleAmp = 0;
    int dipLeft = 0, dipLen = 1;
    int tuneLeft = 0, tuneLen = 1;
    float tuneF1 = 0, tuneF2 = 0, tuneP1 = 0, tuneP2 = 0, tuneChop = 0, tuneChopRate = 10;
    Biquad tuneRes;
    float hissFlutter = 1.0f;

    void startTuning()
    {
        tuneLen = hasSamples && ! smp.tune.empty() ? (int) smp.tune.size() : (int) (1.25 * fs);
        tuneLeft = tuneLen;
        tuneF1 = 2600.0f + 900.0f * rng.uni();
        tuneF2 = 300.0f + 500.0f * rng.uni();
        tuneChopRate = 7.0f + 9.0f * rng.uni();
    }

    // liefert den Pegel fuer die Stimme, schreibt Rauschen/Knacken/Senderwechsel in fx
    float radioTick (float& fx, float hiss)
    {
        float voice = 1.0f;

        // Grundrauschen mit langsamem Wandern
        hissFlutter += 0.00002f * rng.bi();
        hissFlutter = std::clamp (hissFlutter, 0.7f, 1.3f);
        float noiseLvl = hiss * hissFlutter;

        // Knacken: zufaellig, im Mittel alle paar Sekunden
        if (crackleLeft <= 0 && rng.uni() < 0.35f / (float) fs)
        {
            crackleLeft = (int) ((0.01 + 0.06 * rng.uni()) * fs);
            crackleAmp = 0.05f + 0.12f * rng.uni();
        }
        if (crackleLeft > 0)
        {
            --crackleLeft;
            if (rng.uni() < 0.04f) fx += crackleAmp * rng.bi();
        }

        // Kurzer Empfangsschwund: Stimme sackt weg, Rauschen kommt hoch
        if (dipLeft <= 0 && tuneLeft <= 0 && rng.uni() < 0.05f / (float) fs)
        {
            dipLen = (int) ((0.3 + 0.5 * rng.uni()) * fs);
            dipLeft = dipLen;
        }
        if (dipLeft > 0)
        {
            const float ph = 1.0f - (float) dipLeft / (float) dipLen;
            const float d = std::sin (kPi * ph);
            voice *= 1.0f - 0.55f * d;
            noiseLvl *= 1.0f + 2.5f * d;
            --dipLeft;
        }

        // Senderwechsel: Signal weg, Pfeifen und Rauschen beim Durchdrehen, dann rastet der Sender ein
        if (tuneLeft > 0)
        {
            const float t = (float) (tuneLen - tuneLeft) / (float) fs;
            const float T = (float) tuneLen / (float) fs;
            const float fadeOut = std::clamp (1.0f - t / 0.08f, 0.0f, 1.0f);
            const float fadeIn = std::clamp ((t - (T - 0.3f)) / 0.3f, 0.0f, 1.0f);
            voice *= std::max (fadeOut, fadeIn);

            const float mid = std::sin (kPi * std::clamp (t / (T - 0.15f), 0.0f, 1.0f));
            // zwei Pfeiftoene, die beim Drehen durchrutschen
            const float f1 = tuneF1 * std::pow (0.08f, t / T);
            const float f2 = tuneF2 + 1800.0f * std::sin (kPi * t / T);
            tuneP1 += 2.0f * kPi * f1 / (float) fs; if (tuneP1 > 2.0f * kPi) tuneP1 -= 2.0f * kPi;
            tuneP2 += 2.0f * kPi * f2 / (float) fs; if (tuneP2 > 2.0f * kPi) tuneP2 -= 2.0f * kPi;
            tuneChop += tuneChopRate / (float) fs; if (tuneChop > 1.0f) tuneChop -= 1.0f;
            const float chop = tuneChop < 0.55f ? 1.0f : 0.35f;
            // Rauschen durch einen wandernden Resonanzfilter
            tuneRes.setBandpass (fs, 400.0 + 2500.0 * std::abs (std::sin (kPi * 1.7f * t)), 2.5);
            const float res = tuneRes.process (rng.bi());
            if (hasSamples && ! smp.tune.empty())
            {
                const int idx = tuneLen - tuneLeft;
                if (idx >= 0 && idx < (int) smp.tune.size())
                    fx += smp.tune[(size_t) idx] * kTuneSampleGain;
            }
            else
                fx += mid * (0.10f * std::sin (tuneP1) + 0.06f * std::sin (tuneP2)
                             + 0.35f * res * chop + 0.06f * rng.bi());
            --tuneLeft;
        }

        fx += rng.bi() * noiseLvl * 2.0f;
        return voice;
    }

    //--------------------------------------------------------------------------
    // TELEFON
    double toneT = 0;
    bool toneOn = false;
    int curLine = kGespraech;
    int dialStep = 0, dialTimer = 0, dialPulsesLeft = 0, dialDigitsLeft = 0, dialType = kScheibe;
    int dtmfLeft = 0; float dtmfLo = 0, dtmfHi = 0; double dtmfT = 0;
    int clickLeft = 0;
    Rng dialRng;

    struct Mech { int pos = 0, len = 0; bool hangup = false; };
    Mech mech[2];   // zwei, damit sich Auflegen und erneutes Abheben ueberlappen duerfen

    void enterLine (const Params& p, bool withSound)
    {
        curLine = p.line;
        dialType = p.dialType;
        toneT = 0;
        toneOn = (p.line == kWaehlton || p.line == kFreizeichen || p.line == kBesetzt);
        voiceTarget = (p.line == kGespraech) ? 1.0f : 0.0f;
        dialDigitsLeft = 0; dialPulsesLeft = 0; dtmfLeft = 0;
        if (p.line == kWaehlen && withSound)
        {
            dialDigitsLeft = 11;   // lange Nummer: gewaehlt wird, bis auf Freizeichen umgeschaltet wird
            dialTimer = (int) (0.35 * fs);
            dialStep = 0;
        }
    }

    void startMech (bool hangup)
    {
        Mech& m = mech[mech[0].pos >= mech[0].len ? 0 : 1];
        m.pos = 0; m.hangup = hangup;
        m.len = (int) ((hangup ? 0.45 : 0.30) * fs);
    }

    // Hoerer abnehmen / auflegen: Klick, dumpfer Schlag, ein Hauch Glocke (alte Telefone)
    float nextMech()
    {
        float s = 0.0f;
        for (auto& m : mech)
        {
            if (m.pos >= m.len) continue;
            const float t = (float) m.pos++ / (float) fs;
            const float a = m.hangup ? 1.0f : 0.7f;
            auto hit = [&] (float t0, float amp)
            {
                const float u = t - t0;
                if (u < 0.0f) return 0.0f;
                float v = 0.0f;
                if (u < 0.002f) v += 0.6f * rng.bi() * (1.0f - u / 0.002f);
                v += 0.55f * std::sin (2.0f * kPi * 170.0f * u) * std::exp (-u / 0.035f);
                v += 0.35f * std::sin (2.0f * kPi * 540.0f * u) * std::exp (-u / 0.018f);
                v += 0.20f * std::sin (2.0f * kPi * 1650.0f * u) * std::exp (-u / 0.007f);
                v += 0.05f * std::sin (2.0f * kPi * 1180.0f * u) * std::exp (-u / 0.22f);   // Glocke klingt mit
                v += 0.03f * std::sin (2.0f * kPi * 2930.0f * u) * std::exp (-u / 0.15f);
                return v * amp;
            };
            s += hit (0.0f, a);
            if (m.hangup) s += hit (0.055f, 0.45f);   // Gabel prellt nach
            else s += hit (0.11f, 0.25f);            // Hoerer wird angehoben, zweites Klacken
        }
        return s * 0.6f;
    }

    float tone425 (float level)
    {
        return level * std::sin (2.0 * 3.14159265358979 * 425.0 * toneT);
    }

    float phoneTick()
    {
        float s = 0.0f;
        const double dt = 1.0 / fs;

        if (clickLeft > 0) { s += 0.2f * rng.bi(); --clickLeft; }

        if (toneOn)
        {
            const float lvl = 0.18f;
            if (curLine == kWaehlton) s += tone425 (lvl);
            else if (curLine == kFreizeichen) { if (std::fmod (toneT, 5.0) < 1.0) s += tone425 (lvl); }
            else if (curLine == kBesetzt) { if (std::fmod (toneT, 0.96) < 0.48) s += tone425 (lvl); }
            toneT += dt;
        }

        // Waehlen: zufaellige Ziffern
        if (dialDigitsLeft > 0 || dialPulsesLeft > 0 || dtmfLeft > 0)
        {
            if (dialType == kScheibe)
            {
                if (dialPulsesLeft > 0)
                {
                    // 10 Impulse pro Sekunde: 60 ms offen, 40 ms geschlossen, jeder Wechsel knackt
                    const int period = (int) (0.1 * fs), open = (int) (0.06 * fs);
                    const int ph = dialStep % period;
                    if (ph == 0 || ph == open) clickLeft = (int) (0.0025 * fs);
                    if (++dialStep >= period) { dialStep = 0; if (--dialPulsesLeft == 0) dialTimer = (int) ((0.45 + 0.3 * dialRng.uni()) * fs); }
                }
                else if (dialDigitsLeft > 0 && --dialTimer <= 0)
                {
                    const int digit = (int) (dialRng.uni() * 10.0f);
                    dialPulsesLeft = digit == 0 ? 10 : digit;
                    dialStep = 0;
                    --dialDigitsLeft;
                }
            }
            else
            {
                if (dtmfLeft > 0)
                {
                    s += 0.12f * (float) (std::sin (2.0 * 3.14159265358979 * dtmfLo * dtmfT)
                                          + std::sin (2.0 * 3.14159265358979 * dtmfHi * dtmfT));
                    dtmfT += dt;
                    if (--dtmfLeft == 0) dialTimer = (int) ((0.12 + 0.25 * dialRng.uni()) * fs);
                }
                else if (dialDigitsLeft > 0 && --dialTimer <= 0)
                {
                    static const float lo[4] = { 697, 770, 852, 941 }, hi[3] = { 1209, 1336, 1477 };
                    dtmfLo = lo[(int) (dialRng.uni() * 4.0f) & 3];
                    dtmfHi = hi[std::min (2, (int) (dialRng.uni() * 3.0f))];
                    dtmfT = 0;
                    dtmfLeft = (int) (0.09 * fs);
                    --dialDigitsLeft;
                }
            }
        }
        return s;
    }

    //==========================================================================
    double fs = 44100.0;
    int lookahead = 0;
    std::vector<float> delay;
    int dpos = 0;
    Rng rng;

    Band band, fxBand, sqBand, mechBand;
    bool configured = false;
    int lastMode = -1, lastDevice = -1;

    float compEnv = 0, detEnv = 0, gateEnv = 0;
    float voiceGain = 0, voiceTarget = 0, gateGain = 1;

    bool txOpen = false;
    int silentSamples = 0;
    int pendingClose = -1;

    bool firstBlock = true;
    int lastKey = kKeyAuto, lastTune = 0, lastLine = kGespraech;
};
} // namespace rph
