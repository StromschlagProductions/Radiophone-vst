// Render real JUCE editor components; no image retouching. Args: output directory.
#include "PluginProcessor.h"
#include <iostream>

class Capture final : private juce::Timer
{
public:
    explicit Capture (juce::File destination) : output (std::move (destination))
    {
        output.createDirectory();

        startTimer (100);
    }
    int result = 0;
private:
    void parameter (const char* id, float value)
    {
        auto* p = processor.apvts.getParameter (id);
        if (p != nullptr) p->setValueNotifyingHost (p->convertTo0to1 (value));
    }
    void timerCallback() override
    {

        if (index >= 3)
        {
            auto* manifest = new juce::DynamicObject();
            manifest->setProperty ("version", PLUGIN_VERSION_TEXT);
            manifest->setProperty ("images", images);
            const juce::var data (manifest);
            if (! output.getChildFile ("screenshots.json").replaceWithText (juce::JSON::toString (data)))
            { fail ("Could not write screenshot manifest"); return; }
            stopTimer(); juce::MessageManager::getInstance()->stopDispatchLoop(); return;
        }
        if (! editor)
        {
            parameter ("mode", (float) index);
            juce::MemoryBlock state;
            processor.getStateInformation (state);
            auto xml = juce::AudioProcessor::getXmlFromBinary (state.getData(), (int) state.getSize());
            if (! xml) { fail ("Invalid processor state"); return; }
            xml->setAttribute ("theme", index == 2 ? 1 : 0);
            xml->setAttribute ("uiW", 680); xml->setAttribute ("uiH", 420);
            juce::AudioProcessor::copyXmlToBinary (*xml, state);
            processor.setStateInformation (state.getData(), (int) state.getSize());
            editor.reset (processor.createEditor());
            editor->setSize (680, 420); editor->setVisible (true);
            return; // allow attachments/timers to settle on the actual message loop
        }
        auto image = editor->createComponentSnapshot (editor->getLocalBounds(), true, 2.0f);
        auto file = output.getChildFile (index == 0 ? "oberflaeche-funk.png" : index == 1 ? "oberflaeche-radio.png" : "oberflaeche-telefon.png");
        file.deleteFile();
        juce::FileOutputStream stream (file);
        if (! stream.openedOk() || ! juce::PNGImageFormat().writeImageToStream (image, stream))
        { fail ("Could not write screenshot"); return; }
        std::cout << file.getFullPathName() << std::endl;
        images.add (file.getFileName());
        editor.reset(); ++index;
    }
    void fail (const char* reason)
    {
        std::cerr << reason << std::endl; result = 1; stopTimer();
        juce::MessageManager::getInstance()->stopDispatchLoop();
    }
    RadiophoneProcessor processor;
    juce::File output;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    juce::Array<juce::var> images;
    int index = 0;

};

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    if (argc != 2) { std::cerr << "Usage: Screenshot <output-directory>" << std::endl; return 1; }
    Capture capture (juce::File (juce::String::fromUTF8 (argv[1])));
    juce::MessageManager::getInstance()->runDispatchLoop();
    return capture.result;
}
