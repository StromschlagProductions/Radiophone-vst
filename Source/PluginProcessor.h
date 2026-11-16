#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "RadiophoneDSP.h"

class RadiophoneProcessor : public juce::AudioProcessor
{
public:
    RadiophoneProcessor();
    ~RadiophoneProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void reset() override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Radiophone"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::StringArray modeNames();
    static juce::StringArray deviceNames();
    static juce::StringArray keyNames();
    static juce::StringArray lineNames();
    static juce::StringArray dialNames();
    juce::AudioProcessorValueTreeState apvts;

    // fuer die Anzeige in der Oberflaeche
    std::atomic<bool> transmitting { false }, tuning { false };
    // GUI metadata never accesses the APVTS ValueTree concurrently with state restore.
    std::atomic<int> uiTheme { 0 }, uiWidth { 680 }, uiHeight { 420 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    rph::Engine engine;
    void process (juce::AudioBuffer<float>&, bool bypassed);
    std::vector<float> dryDelay[2];
    int dryPosition = 0;
    float bypassMix = 0.0f, bypassStep = 1.0f;
    rph::Samples rawSamples;      // dekodiert, Abtastrate der Datei
    void loadSamples();
    std::vector<float> mono;
    std::atomic<float>* pMode = nullptr; std::atomic<float>* pDevice = nullptr;
    std::atomic<float>* pKey = nullptr;  std::atomic<float>* pPause = nullptr;
    std::atomic<float>* pGate = nullptr; std::atomic<float>* pSfx = nullptr;
    std::atomic<float>* pTune = nullptr; std::atomic<float>* pLine = nullptr;
    std::atomic<float>* pDial = nullptr; std::atomic<float>* pIntf = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadiophoneProcessor)
};
