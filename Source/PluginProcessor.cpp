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
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { "power", 1 }, "Radio: Ein", true));
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
    pPower = apvts.getRawParameterValue ("power");
    pLine = apvts.getRawParameterValue ("line");
    pDial = apvts.getRawParameterValue ("dial");
    pIntf = apvts.getRawParameterValue ("intf");
    loadSamples();
    startTimerHz (30);
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
    load (BinaryData::tune_wav, BinaryData::tune_wavSize, rawSamples.tune[0]);
    load (BinaryData::tune2_wav, BinaryData::tune2_wavSize, rawSamples.tune[1]);
    load (BinaryData::tune3_wav, BinaryData::tune3_wavSize, rawSamples.tune[2]);
    load (BinaryData::radio_off_wav, BinaryData::radio_off_wavSize, rawSamples.power[0]);
    load (BinaryData::radio_on_wav, BinaryData::radio_on_wavSize, rawSamples.power[1]);
#endif
}

void RadiophoneProcessor::prepareToPlay (double sampleRate, int maxBlock)
{
    engine.prepare (sampleRate);
    engine.setSamples (rawSamples);
    mono.assign ((size_t) juce::jmax (maxBlock, 4096), 0.0f);
    for (auto& channel : dryDelay)
        channel.assign ((size_t) rph::Engine::latencyFor (rph::kFunk, sampleRate) + 1, 0.0f);
    engine.setMode (juce::roundToInt (pMode->load()));
    dryPosition = 0;
    bypassMix = 0.0f;
    bypassStep = 1.0f / (0.005f * (float) (std::isfinite (sampleRate) && sampleRate >= 1000.0 ? sampleRate : 44100.0));
    pendingLatency.store (engine.latencySamples());
    setLatencySamples (engine.latencySamples());
}

void RadiophoneProcessor::timerCallback()
{
    // Polling posts no OS messages from the audio callback; works without an editor.
    setLatencySamples (pendingLatency.load (std::memory_order_acquire));
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
    process (buffer, false);
}

void RadiophoneProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    process (buffer, true);
}

void RadiophoneProcessor::reset()
{
    engine.reset();
    for (auto& channel : dryDelay) std::fill (channel.begin(), channel.end(), 0.0f);
    dryPosition = 0;
}

void RadiophoneProcessor::process (juce::AudioBuffer<float>& buffer, bool bypassed)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();
    const int numIn = juce::jmin (getTotalNumInputChannels(), numCh);
    if (numSamples == 0 || numCh == 0)
        return;
    if (mono.empty()) { buffer.clear(); return; } // defensive: host called before prepare

    rph::Params p;
    auto choice = [] (float value, int maximum)
    { return std::isfinite (value) ? juce::roundToInt (juce::jlimit (0.0f, (float) maximum, value)) : 0; };
    p.mode = choice (pMode->load(), 2);
    p.device = choice (pDevice->load(), 2);
    p.key = choice (pKey->load(), 2);
    p.pauseSec = pPause->load();
    p.gate = pGate->load();
    p.sfxDb = pSfx->load();
    p.tuneCount = pTune->load() > 0.5f ? 1 : 0;   // jede Aenderung loest einen Senderwechsel aus
    p.line = choice (pLine->load(), 5);
    p.dialType = choice (pDial->load(), 1);
    p.radioOn = pPower->load() > 0.5f ? 1 : 0;
    p.interference = pIntf->load();

    // A bounded work buffer handles arbitrary host blocks without resizing.
    for (int offset = 0; offset < numSamples;)
    {
        const int count = juce::jmin ((int) mono.size(), numSamples - offset);
        float* m = mono.data();
        std::fill (m, m + count, 0.0f);
        for (int ch = 0; ch < numIn; ++ch)
        {
            const auto* src = buffer.getReadPointer (ch, offset);
            for (int i = 0; i < count; ++i)
                m[i] += (std::isfinite (src[i]) ? src[i] : 0.0f) / (float) numIn;
        }
        engine.process (m, count, p);
        const int latency = engine.latencySamples();
        pendingLatency.store (latency, std::memory_order_release);
        for (int i = 0; i < count; ++i)
        {
            bypassMix += juce::jlimit (-bypassStep, bypassStep,
                                      (bypassed ? 1.0f : 0.0f) - bypassMix);
            const int read = dryPosition + 1 == (int) dryDelay[0].size() ? 0 : dryPosition + 1;
            float dry[2] {};
            for (int ch = 0; ch < juce::jmin (numIn, 2); ++ch)
            {
                const float input = buffer.getSample (ch, offset + i);
                dryDelay[ch][(size_t) dryPosition] = std::isfinite (input) ? input : 0.0f;
                dry[ch] = latency == 0 ? dryDelay[ch][(size_t) dryPosition] : dryDelay[ch][(size_t) read];
            }
            for (int ch = 0; ch < numCh; ++ch)
                buffer.setSample (ch, offset + i, m[i] + bypassMix * (dry[numIn == 1 ? 0 : juce::jmin (ch, 1)] - m[i]));
            dryPosition = read;
        }
        offset += count;
    }
    transmitting.store (engine.isTransmitting(), std::memory_order_relaxed);
    tuning.store (engine.isTuning(), std::memory_order_relaxed);
}

void RadiophoneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty ("theme", uiTheme.load(), nullptr);
    state.setProperty ("uiW", uiWidth.load(), nullptr);
    state.setProperty ("uiH", uiHeight.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void RadiophoneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName ("RADIOPHONE"))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            uiTheme.store ((int) state.getProperty ("theme", 0));
            uiWidth.store (juce::jlimit (340, 2040, (int) state.getProperty ("uiW", 680)));
            uiHeight.store (juce::jlimit (210, 1260, (int) state.getProperty ("uiH", 420)));
            apvts.replaceState (state);
        }
}

juce::AudioProcessorEditor* RadiophoneProcessor::createEditor() { return new RadiophoneEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RadiophoneProcessor(); }
