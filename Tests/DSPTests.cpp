#include "RadiophoneDSP.h"
#include "TestSupport.h"
#include <limits>
#include <thread>

static std::vector<float> signal (int n, double fs)
{
    std::vector<float> v ((size_t) n);
    for (int i = 0; i < n; ++i) v[(size_t) i] = 0.1f * (float) std::sin (i * 1200.0 / fs);
    return v;
}
static void finite (const std::vector<float>& v) { for (float x : v) CHECK (std::isfinite (x) && std::abs (x) < 30.0f); }
static void run (double fs)
{
    NoDenormals denormals;
    for (int mode = 0; mode < 3; ++mode)
    for (int device = 0; device < 3; ++device)
    {
        rph::Engine a, b;
        a.prepare (fs); b.prepare (fs);
        rph::Params p; p.mode = mode; p.device = device; p.key = rph::kKeyDown;
        auto whole = signal (65537, fs), split = whole;
        { NoAllocations guard; a.process (whole.data(), (int) whole.size(), p); }
        const int blocks[] { 1, 3, 31, 128, 4097, 16384 };
        for (int pos = 0, block = 0; pos < (int) split.size(); ++block)
        {
            const int n = std::min (blocks[block % 6], (int) split.size() - pos);
            { NoAllocations guard; b.process (split.data() + pos, n, p); }
            pos += n;
        }
        CHECK (whole == split); finite (whole);
        CHECK (a.latencySamples() == (int) std::lround (rph::Engine::kLookaheadSec * fs));
        a.reset(); auto again = signal (65537, fs);
        a.process (again.data(), (int) again.size(), p); CHECK (whole == again);
    }
    rph::Engine e; e.prepare (fs);
    std::vector<float> block (257);
    rph::Params p;
    for (int k = 0; k < 1500; ++k)
    {
        p.mode = k % 3; p.device = (k / 3) % 3; p.key = (k / 9) % 3;
        p.line = k % 6; p.dialType = k % 2; p.gate = (float) (k % 2);
        p.tuneCount = (k / 3) % 2; p.sfxDb = k % 2 ? 6.0f : -24.0f;
        p.interference = (float) (k % 2); p.pauseSec = k % 2 ? 0.2f : 3.0f;
        std::fill (block.begin(), block.end(), k % 4 ? 1e-25f : 1.0f);
        { NoAllocations guard; e.process (block.data(), (int) block.size(), p); }
        finite (block);
    }
    block[0] = std::numeric_limits<float>::quiet_NaN(); block[1] = std::numeric_limits<float>::infinity();
    e.process (block.data(), (int) block.size(), p); finite (block);
    // Invalid control values cannot index sample banks or poison persistent state.
    p.device = 1000; p.mode = -4; p.pauseSec = std::numeric_limits<float>::quiet_NaN();
    p.gate = std::numeric_limits<float>::infinity();
    e.process (block.data(), (int) block.size(), p); finite (block);
    // Known one-shot must finish after leaving Funk (and without needing an editor).
    rph::Samples samples; samples.rate = fs; samples.sqOpen[0].assign (1000, 0.2f);
    e.prepare (fs); e.setSamples (samples); p = {}; p.key = rph::kKeyUp;
    float x = 0; e.process (&x, 1, p); p.key = rph::kKeyDown; e.process (&x, 1, p);
    p.mode = rph::kTelefon; p.line = rph::kAufgelegt;
    std::fill (block.begin(), block.end(), 0.0f); e.process (block.data(), 257, p);
    CHECK (block[200] > 0.09f);
    // Recorded tuning must continue to completion across modes, then reset reproducibly.
    samples.tune.assign (1000, 0.1f); e.prepare (fs); e.setSamples (samples);
    p = {}; p.mode = rph::kRadio; e.process (&x, 1, p); ++p.tuneCount; e.process (&x, 1, p);
    CHECK (e.isTuning()); p.mode = rph::kTelefon;
    e.process (block.data(), 257, p); CHECK (e.isTuning());
    std::vector<float> finish (1000); e.process (finish.data(), 1000, p); CHECK (!e.isTuning());
    // Device change must not clear audio that is still in the lookahead buffer.
    e.prepare (fs); p = {}; p.key = rph::kKeyDown; p.sfxDb = -80;
    x = 1; e.process (&x, 1, p); p.device = 1;
    std::vector<float> tail ((size_t) e.latencySamples() + 500, 0);
    e.process (tail.data(), (int) tail.size(), p);
    float energy = 0; for (float v : tail) energy += v * v; CHECK (energy > 1e-5f);
}
int main()
{
    for (double fs : { 22050., 44100., 48000., 96000., 192000. }) run (fs);
    std::thread a ([] { run (44100); }), b ([] { run (96000); }); a.join(); b.join();
    std::cout << "DSP: sample rates, partition invariance, reset, modes, tails, finite output and allocation checks passed\n";
}
