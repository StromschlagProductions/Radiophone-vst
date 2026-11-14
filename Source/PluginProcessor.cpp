#include "PluginProcessor.h"
#include "PluginEditor.h"
#if RPH_HAS_SAMPLES
 #include <BinaryData.h>
#endif

static juce::String u8 (const char* s) { return juce::String (juce::CharPointer_UTF8 (s)); }

// Reihenfolgen muessen zu den enums in RadiophoneDSP.h passen
juce::StringArray RadiophoneProcessor::modeNames()   { return { "Funk", "Radio", "Telefon" }; }
juce::StringArray RadiophoneProcessor::deviceNames() { return { "Walkie-Talkie", "Autofunk", "Tischfunk" }; }
juce::StringArray RadiophoneProcessor::keyNames()    { return { "Automatik", u8 ("Gedr\xc3\xbc" "ckt"), "Losgelassen" }; }
juce::StringArray RadiophoneProcessor::lineNames()
{
    return { u8 ("Gespr\xc3\xa4" "ch"), "Aufgelegt", u8 ("W\xc3\xa4" "hlton"), u8 ("W\xc3\xa4" "hlen"), "Freizeichen", "Besetzt" };
}
juce::StringArray RadiophoneProcessor::dialNames()   { return { u8 ("W\xc3\xa4" "hlscheibe"), "Tasten" }; }

juce::AudioProcessorValueTreeState::ParameterLayout RadiophoneProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto seconds = AudioParameterFloatAttributes().withStringFromValueFunction (
        [] (float v, int) { return String (v, 1).replaceCharacter ('.', ',') + " s"; });
    auto decibel = AudioParameterFloatAttributes().withStringFromValueFunction (
        [] (float v, int) { return (v > 0 ? "+" : "") + String (v, 1).replaceCharacter ('.', ',') + " dB"; });
    auto gateText = AudioParameterFloatAttributes().withStringFromValueFunction (
        [] (float v, int) { const int pct = roundToInt (v * 100.0f); return pct <= 0 ? String ("aus") : String (pct) + " %"; });

    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Modus", modeNames(), 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "device", 1 }, u8 ("Funk: Ger\xc3\xa4t"), deviceNames(), 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "key", 1 }, "Funk: Sprechtaste", keyNames(), 0));
    NormalisableRange<float> pauseRange (0.2f, 3.0f, 0.05f);
    pauseRange.setSkewForCentre (0.8f);
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "pause", 1 }, "Funk: Pause bis Tastenende", pauseRange, 0.6f, seconds));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "gate", 1 }, "Funk: Abhacken (Gate)",
                                                       NormalisableRange<float> (0.0f, 1.0f), 0.0f, gateText));
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "tune", 1 }, "Radio: Senderwechsel", false));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "intf", 1 }, u8 ("Radio: St\xc3\xb6rungen"),
                                                       NormalisableRange<float> (0.0f, 1.0f), 0.5f, gateText));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "line", 1 }, "Telefon: Leitung", lineNames(), 0));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "dial", 1 }, u8 ("Telefon: W\xc3\xa4hlart"), dialNames(), 0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "sfx", 1 }, u8 ("Ger\xc3\xa4usche"),
                                                       NormalisableRange<float> (-24.0f, 6.0f, 0.1f), 0.0f, decibel));
    return layout;
}

RadiophoneProcessor::RadiophoneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "RADIOPHONE", createLayout())
{
    pMode = apvts.getRawParameterValue ("mode");
    pDevice = apvts.getRawParameterValue ("device");
    pKey = apvts.getRawParameterValue ("key");
    pPause = apvts.getRawParameterValue ("pause");
    pGate = apvts.getRawParameterValue ("gate");
    pSfx = apvts.getRawParameterValue ("sfx");
    pTune = apvts.getRawParameterValue ("tune");
    pLine = apvts.getRawParameterValue ("line");
    pDial = apvts.getRawParameterValue ("dial");
    pIntf = apvts.getRawParameterValue ("intf");
    loadSamples();
}

#if RPH_HAS_SAMPLES
static bool decodeWav (const void* data, int size, std::vector<float>& out, double& rate)
{
    juce::WavAudioFormat wav;
    auto stream = std::make_unique<juce::MemoryInputStream> (data, (size_t) size, false);
    std::unique_ptr<juce::AudioFormatReader> reader (wav.createReaderFor (stream.release(), true));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;
    rate = reader->sampleRate;
    juce::AudioBuffer<float> buf ((int) reader->numChannels, (int) reader->lengthInSamples);
    reader->read (&buf, 0, (int) reader->lengthInSamples, 0, true, true);
    out.assign (buf.getReadPointer (0), buf.getReadPointer (0) + buf.getNumSamples());
    return true;
}
#endif

void RadiophoneProcessor::loadSamples()
{
#if RPH_HAS_SAMPLES
    double rate = 44100.0;
    std::vector<float> v;
    auto load = [&] (const void* d, int sz, std::vector<float>& dst)
    {
        if (decodeWav (d, sz, v, rate)) { dst = v; rawSamples.rate = rate; }
    };
    load (BinaryData::sq_walkie_open_wav, BinaryData::sq_walkie_open_wavSize, rawSamples.sqOpen[rph::kWalkie]);
    load (BinaryData::sq_walkie_close_wav, BinaryData::sq_walkie_close_wavSize, rawSamples.sqClose[rph::kWalkie]);
    load (BinaryData::sq_auto_open_wav, BinaryData::sq_auto_open_wavSize, rawSamples.sqOpen[rph::kAutofunk]);
    load (BinaryData::sq_auto_close_wav, BinaryData::sq_auto_close_wavSize, rawSamples.sqClose[rph::kAutofunk]);
    load (BinaryData::sq_tisch_open_wav, BinaryData::sq_tisch_open_wavSize, rawSamples.sqOpen[rph::kTischfunk]);
    load (BinaryData::sq_tisch_close_wav, BinaryData::sq_tisch_close_wavSize, rawSamples.sqClose[rph::kTischfunk]);
    load (BinaryData::tune_wav, BinaryData::tune_wavSize, rawSamples.tune);
#endif
}

void RadiophoneProcessor::prepareToPlay (double sampleRate, int maxBlock)
{
    engine.prepare (sampleRate);
    engine.setSamples (rawSamples);
    mono.assign ((size_t) juce::jmax (maxBlock, 4096), 0.0f);
    engine.setMode (juce::roundToInt (pMode->load()));
    reportedLatency = engine.latencySamples();
    setLatencySamples (reportedLatency.load());   // nur im Funkmodus: Vorlauf fuer das Tschk, gleicht der Host aus
}

bool RadiophoneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    const auto in = layouts.getMainInputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    if (in == out) return true;
    return in == juce::AudioChannelSet::mono() && out == juce::AudioChannelSet::stereo();
}

void RadiophoneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    const int numIn = juce::jmin (getTotalNumInputChannels(), numCh);
    if (numSamples == 0 || numCh == 0)
        return;
    if ((int) mono.size() < numSamples)
        mono.resize ((size_t) numSamples);   // sollte nie passieren (Host haelt sich an die Blockgroesse)

    rph::Params p;
    p.mode = juce::roundToInt (pMode->load());
    p.device = juce::roundToInt (pDevice->load());
    p.key = juce::roundToInt (pKey->load());
    p.pauseSec = pPause->load();
    p.gate = pGate->load();
    p.sfxDb = pSfx->load();
    p.tuneCount = pTune->load() > 0.5f ? 1 : 0;   // jede Aenderung loest einen Senderwechsel aus
    p.line = juce::roundToInt (pLine->load());
    p.dialType = juce::roundToInt (pDial->load());
    p.interference = pIntf->load();

    // Funk, Radio und Telefon sind mono: Eingang zusammenmischen
    float* m = mono.data();
    std::fill (m, m + numSamples, 0.0f);
    if (numIn > 0)
    {
        const float g = 1.0f / (float) numIn;
        for (int ch = 0; ch < numIn; ++ch)
        {
            const float* src = buffer.getReadPointer (ch);
            for (int i = 0; i < numSamples; ++i)
                m[i] += src[i] * g;
        }
    }

    engine.process (m, numSamples, p);
    if (engine.latencySamples() != reportedLatency.load())
    {
        reportedLatency = engine.latencySamples();   // Moduswechsel: Host ueber neue Latenz informieren
        triggerAsyncUpdate();
    }
    transmitting.store (engine.isTransmitting());
    tuning.store (engine.isTuning());

    for (int ch = 0; ch < numCh; ++ch)
        buffer.copyFrom (ch, 0, m, numSamples);
}

void RadiophoneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RadiophoneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* RadiophoneProcessor::createEditor() { return new RadiophoneEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RadiophoneProcessor(); }
