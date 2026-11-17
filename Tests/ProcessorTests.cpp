#include "PluginProcessor.h"
#include "TestSupport.h"
#include <thread>

static void parameter (RadiophoneProcessor& p, const char* id, float value)
{
    auto* v = p.apvts.getParameter (id); CHECK (v != nullptr);
    v->setValueNotifyingHost (v->convertTo0to1 (value));
}
static void fill (juce::AudioBuffer<float>& b)
{
    for (int c = 0; c < b.getNumChannels(); ++c)
        for (int i = 0; i < b.getNumSamples(); ++i)
            b.setSample (c, i, 0.1f * std::sin (0.07f * (float) i));
}
static void layout (RadiophoneProcessor& p, int in, int out)
{
    auto buses = p.getBusesLayout();
    buses.inputBuses.set (0, in == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
    buses.outputBuses.set (0, out == 1 ? juce::AudioChannelSet::mono() : juce::AudioChannelSet::stereo());
    CHECK (p.setBusesLayout (buses));
}
int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    juce::MidiBuffer midi;
    for (double fs : { 22050., 44100., 48000., 96000., 192000. })
    for (int in : { 1, 2 })
    for (int out : { 1, 2 })
    {
        if (in > out) continue;
        RadiophoneProcessor p; layout (p, in, out); p.prepareToPlay (fs, 32);
        auto latency = p.getLatencySamples();
        CHECK (latency == rph::Engine::latencyFor (0, fs));
        for (int mode = 0; mode < 3; ++mode)
        {
            parameter (p, "mode", (float) mode); parameter (p, "key", 1);
            p.prepareToPlay (fs, 32);
            latency = rph::Engine::latencyFor (mode, fs);
            for (int n : { 0, 1, 3, 32, 4096, 4097, 65537, 7 })
            {
                juce::AudioBuffer<float> b (out, n); fill (b);
                { NoAllocations guard; p.processBlock (b, midi); }
                CHECK (p.getLatencySamples() == latency);
                for (int i = 0; i < n; ++i)
                {
                    CHECK (std::isfinite (b.getSample (0, i)));
                    if (out == 2) CHECK (b.getSample (0, i) == b.getSample (1, i));
                }
            }
        }
        for (int mode = 0; mode < 3; ++mode)
        {
            parameter (p, "mode", (float) mode); p.prepareToPlay (fs, 32);
            latency = rph::Engine::latencyFor (mode, fs);
        // Bypass is delayed by exactly the declared latency, including oversized blocks.
        juce::AudioBuffer<float> warm (out, 8192); warm.clear(); p.processBlockBypassed (warm, midi);
        p.reset(); juce::AudioBuffer<float> impulse (out, latency + 500); impulse.clear();
        for (int c = 0; c < in; ++c) impulse.setSample (c, 0, 0.5f);
        { NoAllocations guard; p.processBlockBypassed (impulse, midi); }
        for (int c = 0; c < out; ++c)
            for (int i = 0; i < impulse.getNumSamples(); ++i)
                CHECK (std::abs (impulse.getSample (c, i) - (i == latency ? 0.5f : 0.0f)) < 1e-6f);
        }
    }
    // Audio changes latency immediately; editor-independent processor timer informs the host.
    RadiophoneProcessor latencyA, latencyB;
    latencyA.prepareToPlay (48000, 32); parameter (latencyB, "mode", 1); latencyB.prepareToPlay (48000, 32);
    CHECK (latencyA.getLatencySamples() == 1440 && latencyB.getLatencySamples() == 0);
    juce::AudioBuffer<float> change (2, 64); change.clear();
    parameter (latencyA, "mode", 2); latencyA.processBlock (change, midi);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    CHECK (latencyA.getLatencySamples() == 0 && latencyB.getLatencySamples() == 0);
    parameter (latencyA, "mode", 0); latencyA.processBlock (change, midi);
    juce::MessageManager::getInstance()->runDispatchLoopUntil (100);
    CHECK (latencyA.getLatencySamples() == 1440 && latencyB.getLatencySamples() == 0);
    // Processor partition equivalence catches offset/downmix/copy errors in chunking.
    RadiophoneProcessor a, b; a.prepareToPlay (48000, 16); b.prepareToPlay (48000, 16);
    parameter (a, "key", 1); parameter (b, "key", 1);
    juce::AudioBuffer<float> whole (2, 65537), split (2, 65537); fill (whole); split.makeCopyOf (whole);
    a.processBlock (whole, midi);
    for (int i = 0; i < 65537; i += 31)
    {
        juce::AudioBuffer<float> view (split.getArrayOfWritePointers(), 2, i, std::min (31, 65537 - i));
        b.processBlock (view, midi);
    }
    for (int c = 0; c < 2; ++c) for (int i = 0; i < 65537; ++i) CHECK (whole.getSample (c, i) == split.getSample (c, i));
    parameter (a, "power", 0);
    parameter (a, "mode", 2); parameter (a, "sfx", -12); a.uiTheme = 1; a.uiWidth = 900; a.uiHeight = 556;
    juce::MemoryBlock state; a.getStateInformation (state); b.setStateInformation (state.getData(), (int) state.getSize());
    CHECK (b.apvts.getRawParameterValue ("power")->load() == 0);
    CHECK (b.apvts.getRawParameterValue ("mode")->load() == 2); CHECK (b.uiTheme == 1); CHECK (b.uiWidth == 900);
    b.setStateInformation ("broken", 6); CHECK (b.apvts.getRawParameterValue ("mode")->load() == 2);
    // GUI construction/destruction and independent metadata (no display peer required).
    for (int n = 0; n < 10; ++n)
    {
        std::unique_ptr<juce::AudioProcessorEditor> ea (a.createEditor()), eb (b.createEditor());
        ea->setSize (680, 420); eb->setSize (1020, 630);
        CHECK (a.uiWidth == 680 && b.uiWidth == 1020);
    }
    // Concurrent independent instances plus parameter automation / state restore.
    std::atomic<bool> done { false };
    std::thread audio ([&] {
        juce::AudioBuffer<float> block (2, 127); juce::MidiBuffer m;
        for (int n = 0; n < 1000; ++n) { fill (block); NoAllocations guard; a.processBlock (block, m); b.processBlock (block, m); }
        done = true;
    });
    while (! done.load()) { b.setStateInformation (state.getData(), (int) state.getSize()); parameter (a, "mode", 1); }
    audio.join();
    std::cout << "Processor: layouts, blocks, allocation, latency, bypass, state, GUI lifecycle and concurrent instances passed\n";
}
