#pragma once
#include "PluginProcessor.h"

// Onboard-Hilfe: formatierter Text, scrollbar in einem Viewport (wie bei Locations)
class HelpView : public juce::Component
{
public:
    void setAttributed (const juce::AttributedString& s) { text = s; relayout(); }
    void setWidth (int w) { if (w != laidOutW) { laidOutW = w; relayout(); } }
    void paint (juce::Graphics& g) override { tl.draw (g, getLocalBounds().toFloat().reduced (14.0f, 10.0f)); }
private:
    void relayout()
    {
        const int w = juce::jmax (120, laidOutW);
        tl.createLayout (text, (float) (w - 28));
        setSize (w, (int) std::ceil (tl.getHeight()) + 24);
        repaint();
    }
    juce::AttributedString text;
    juce::TextLayout tl;
    int laidOutW = 0;
};

class HelpPanel : public juce::Viewport
{
public:
    juce::Colour fill, edge;
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (fill);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);
    }
};

class RadiophoneEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RadiophoneEditor (RadiophoneProcessor&);
    ~RadiophoneEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    struct Knob
    {
        juce::Slider slider; juce::Label label;
        std::unique_ptr<APVTS::SliderAttachment> attachment;
    };
    struct Theme { juce::Colour bg, panel, accent, text, dim, line; bool dark; };

    void timerCallback() override;
    void setupKnob (Knob&, const juce::String& paramId, const char* text);
    void setupBox (juce::ComboBox&, const juce::StringArray& items, const juce::String& paramId,
                   std::unique_ptr<APVTS::ComboBoxAttachment>&);
    void applyTheme();
    void updateModeVisibility();
    int currentMode() const;
    void setMode (int m);
    void triggerTuning();
    static Theme darkTheme();
    static Theme lightTheme();

    static constexpr int kBaseW = 680, kBaseH = 420;

    RadiophoneProcessor& processor;
    juce::LookAndFeel_V4 look;
    juce::Component content;
    float uiScale = 1.0f;
    Theme theme;
    bool sizeReady = false;
    juce::Image logo;

    juce::TextButton modeButtons[3];
    int shownMode = -1;
    bool lampOn = false;
    int lastLine = -1;
    int lastKeyShown = -1;

    // Funk
    juce::ComboBox deviceBox, keyBox;
    Knob gate, pause, intf;
    // Radio
    juce::TextButton tuneButton;
    juce::Label radioInfo;
    // Telefon
    juce::ComboBox lineBox, dialBox;
    juce::Label lineInfo, dialInfo;
    // alle
    Knob sfx;
    std::unique_ptr<APVTS::ComboBoxAttachment> attDevice, attKey, attLine, attDial;

    juce::TextButton themeButton, helpButton;
    HelpPanel helpScroll;
    HelpView helpText;
    juce::TooltipWindow tooltipWindow { this, 400 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RadiophoneEditor)
};
