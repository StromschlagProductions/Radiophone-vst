#include "PluginEditor.h"
#include <BinaryData.h>

namespace
{
juce::String u8 (const char* s) { return juce::String (juce::CharPointer_UTF8 (s)); }

// Kasten-Positionen unten (gleiches Raster in allen Modi)
constexpr float kBoxY = 208.0f, kBoxH = 200.0f;
constexpr float kAx = 12.0f, kAw = 250.0f;     // linker Kasten
constexpr float kBx = 270.0f, kBw = 250.0f;    // mittlerer Kasten
constexpr float kCx = 528.0f, kCw = 140.0f;    // GERAEUSCHE (immer da)

juce::String lineDescription (int line)
{
    switch (line)
    {
        case rph::kGespraech:   return u8 ("Die Stimme ist zu h\xc3\xb6ren, leise Leitung im Hintergrund.");
        case rph::kAufgelegt:   return u8 ("Leitung tot, die Stimme ist stumm. Der Wechsel von hier weg klingt wie H\xc3\xb6rer abnehmen, der Wechsel hierher legt die Leitung auf (Knacken in der Leitung, passt zu jedem Telefon).");
        case rph::kWaehlton:    return u8 ("H\xc3\xb6rer ist abgenommen: Dauerton (425 Hz), es wurde noch nicht gew\xc3\xa4hlt.");
        case rph::kWaehlen:     return u8 ("Es wird eine Nummer gew\xc3\xa4hlt, bis du auf Freizeichen umschaltest.");
        case rph::kFreizeichen: return u8 ("Es klingelt beim Angerufenen: 1 s Ton, 4 s Pause.");
        case rph::kBesetzt:     return u8 ("Besetztzeichen \xe2\x80\x93 auch nach dem Auflegen der Gegenseite.");
        default: return {};
    }
}
}

RadiophoneEditor::Theme RadiophoneEditor::darkTheme()
{
    return { juce::Colour (0xff182326), juce::Colour (0xff233539), juce::Colour (0xfff5781a),
             juce::Colour (0xffe8ecec), juce::Colour (0xff8fa1a5), juce::Colour (0xff35494e), true };
}
RadiophoneEditor::Theme RadiophoneEditor::lightTheme()
{
    return { juce::Colour (0xffd5dbda), juce::Colour (0xffe8eae9), juce::Colour (0xfff5781a),
             juce::Colour (0xff233539), juce::Colour (0xff5d6e72), juce::Colour (0xffb5bfbe), false };
}

static juce::AttributedString buildHelp (juce::Colour text, juce::Colour accent, juce::Colour dim)
{
    juce::AttributedString s;
    s.setWordWrap (juce::AttributedString::byWord);
    s.setJustification (juce::Justification::topLeft);

    auto head = [&] (const char* t) { s.append (u8 (t) + "\n", juce::Font (juce::FontOptions (14.0f, juce::Font::bold)), accent); };
    auto body = [&] (const char* t) { s.append (u8 (t) + "\n\n", juce::Font (juce::FontOptions (13.0f)), text); };
    auto note = [&] (const char* t) { s.append (u8 (t) + "\n\n", juce::Font (juce::FontOptions (12.0f)), dim); };

    s.append (u8 ("RADIOPHONE \xe2\x80\x93 Hilfe") + u8 (" \xc2\xb7 v") + juce::String (PLUGIN_VERSION_TEXT) + "\n\n",
              juce::Font (juce::FontOptions (17.0f, juce::Font::bold)), accent);

    body ("Radiophone ist ein Insert-Effekt: Die Spur l\xc3\xa4uft komplett durch den Effekt (Funk, Radio oder Telefon). Der Klang ist fest abgestimmt, es gibt nichts zu mischen. Die Ger\xc3\xa4usche sind eingebaut.");

    head ("FUNK");
    body ("Ger\xc3\xa4t: Walkie-Talkie (schmal, rau, langes Kschhh), Autofunk (mittig, kurzes Kschhh) oder Tischfunk (voller, mittleres Kschhh).");
    body ("Sprechtaste \xe2\x80\x93 Automatik: Das Plugin \xc3\xb6\x66\x66net den Funk beim ersten Wort (das Tschk liegt kurz davor) und schlie\xc3\x9ft erst, wenn l\xc3\xa4nger als die eingestellte Pause Stille ist. Kurze Pausen zwischen W\xc3\xb6rtern oder S\xc3\xa4tzen l\xc3\xb6sen also nichts aus.");
    body ("Sprechtaste \xe2\x80\x93 Gedr\xc3\xbc\x63kt / Losgelassen: Volle Kontrolle per Automation. Der Wechsel auf Gedr\xc3\xbc\x63kt \xc3\xb6\x66\x66net mit Tschk, der Wechsel auf Losgelassen schlie\xc3\x9ft mit Kschhh. Solange losgelassen ist, ist der Empf\xc3\xa4nger stumm.");
    body ("Pause bis Tastenende: nur f\xc3\xbcr Automatik. So lange muss Stille sein, bis die Taste als losgelassen gilt.");
    body ("Abhacken (Gate): aus = sauber. Je weiter aufgedreht, desto eher bricht die Verbindung bei leisen Stellen hart weg, wie bei schlechtem Empfang.");

    head ("RADIO");
    body ("Fester Radioklang mit leisem Grundrauschen und gelegentlichem Knacken. Gut f\xc3\xbcr Musik und Durchsagen aus einem Radio.");
    body ("St\xc3\xb6rungen: Wie oft und wie stark der Empfang schlecht wird. Die Stimme sackt ab und geht ins Gruscheln, das Rauschen kommt hoch, dann ist sie wieder da. aus = keine St\xc3\xb6rungen, voll = h\xc3\xa4ufig und stark.");
    body ("Senderwechsel: Klick auf den Knopf, oder den Parameter \xe2\x80\x9eRadio: Senderwechsel\xe2\x80\x9c automatisieren. Jede \xc3\x84nderung dreht den Sender weiter: Das Signal ist etwa eine Sekunde weg, es pfeift und rauscht, dann rastet der Sender wieder ein. Es gibt neun verschiedene Senderwechsel-Ger\xc3\xa4usche, sie kommen zuf\xc3\xa4llig und nie zweimal dasselbe hintereinander.");
    body ("Radio an/aus: Schaltet das Radio aus, mit einem Klick. Rauschen, Knacken und Stimme sind dann sofort still, auch wenn die Spur weiterl\xc3\xa4uft. Wieder an mit einem zweiten Klick. Automatisierbar als \xe2\x80\x9eRadio: Ein\xe2\x80\x9c.");
    note ("Den Raum (Radio steht hinten rechts) macht LOCATIONS oder der Pan am Kanal.");

    head ("TELEFON");
    body ("Fester Telefonklang. Automatisiere \xe2\x80\x9eTelefon: Leitung\xe2\x80\x9c f\xc3\xbcr den Ablauf, z. B. Aufgelegt \xe2\x86\x92 W\xc3\xa4hlton \xe2\x86\x92 W\xc3\xa4hlen \xe2\x86\x92 Freizeichen \xe2\x86\x92 Gespr\xc3\xa4\x63h \xe2\x86\x92 Besetzt \xe2\x86\x92 Aufgelegt.");
    body ("Weg von Aufgelegt = H\xc3\xb6rer abnehmen (Klacken), hin zu Aufgelegt = Leitung wird aufgelegt (passt zu W\xc3\xa4hlscheibe, Tastentelefon und Handy). Die Stimme ist nur bei Gespr\xc3\xa4\x63h zu h\xc3\xb6ren.");
    body ("W\xc3\xa4hlart: W\xc3\xa4hlscheibe (Impulse, typisch f\xc3\xbcr die 80er) oder Tasten (T\xc3\xb6ne). Jedes W\xc3\xa4hlen ist eine neue, zuf\xc3\xa4llige Nummer mit leicht schwankenden Pausen. Du musst keine W\xc3\xa4hlt\xc3\xb6ne mehr von Hand variieren. Nach dem Laden beginnt der Zufall immer gleich, ein Export klingt also jedes Mal identisch.");

    head ("GER\xc3\x84USCHE");
    body ("Lautst\xc3\xa4rke aller eingebauten Ger\xc3\xa4usche: Squelch, Rauschen, Knacken, Senderwechsel und Telefont\xc3\xb6ne. Die Stimme bleibt davon unber\xc3\xbchrt.");

    head ("Allgemein");
    body ("Nur Funk hat rund 30 ms Vorlauf, damit das Tschk vor dem ersten Wort liegt. Radio und Telefon haben 0 ms Plugin-Latenz. Die DAW gleicht den Funk-Vorlauf aus. Den Modus vor der Wiedergabe einstellen.");
    body ("Doppelklick auf einen Drehregler setzt ihn zur\xc3\xbc\x63k. Look: Dunkel / Hell, das Fenster ist am rechten unteren Rand skalierbar (50\xe2\x80\x93\x33\x30\x30 %).");

    head ("Idee und Konzept");
    body ("Lars Eickst\xc3\xa4\x64t, Stromschlag Audio");
    note ("Gebaut mit JUCE. S\xc3\xa4ttigung nach Airwindows Density (Chris Johnson, MIT-Lizenz).");
    return s;
}

RadiophoneEditor::RadiophoneEditor (RadiophoneProcessor& p) : AudioProcessorEditor (&p), processor (p)
{
    addAndMakeVisible (content);
    setResizable (true, true);
    setResizeLimits (kBaseW / 2, kBaseH / 2, kBaseW * 3, kBaseH * 3);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) kBaseW / (double) kBaseH);

    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);
    theme = processor.uiTheme.load() == 1 ? lightTheme() : darkTheme();
    setLookAndFeel (&look);

    const auto modes = RadiophoneProcessor::modeNames();
    for (int i = 0; i < 3; ++i)
    {
        modeButtons[i].setButtonText (modes[i].toUpperCase());
        modeButtons[i].onClick = [this, i] { setMode (i); };
        content.addAndMakeVisible (modeButtons[i]);
    }
    modeButtons[0].setTooltip (u8 ("Funkger\xc3\xa4t mit Squelch beim \xc3\x96\x66\x66nen und Schlie\xc3\x9f\x65n"));
    modeButtons[1].setTooltip (u8 ("Radio mit Rauschen, Knacken und Senderwechsel"));
    modeButtons[2].setTooltip (u8 ("Telefon mit W\xc3\xa4hlton, W\xc3\xa4hlen, Freizeichen, Besetzt und H\xc3\xb6rerger\xc3\xa4uschen"));

    // Funk
    setupBox (deviceBox, RadiophoneProcessor::deviceNames(), "device", attDevice);
    setupBox (keyBox, RadiophoneProcessor::keyNames(), "key", attKey);
    setupKnob (gate, "gate", "Abhacken");
    setupKnob (pause, "pause", "Pause bis Ende");
    deviceBox.setTooltip (u8 ("Walkie-Talkie: schmal und rau. Autofunk: mittig. Tischfunk: voller."));
    keyBox.setTooltip (u8 ("Automatik: \xc3\xb6\x66\x66net beim ersten Wort, schlie\xc3\x9ft nach der Pause. Gedr\xc3\xbc\x63kt / Losgelassen: per Automation steuern."));
    gate.slider.setTooltip (u8 ("aus = sauber. Mehr = Verbindung bricht bei leisen Stellen hart weg."));
    pause.slider.setTooltip (u8 ("Nur Automatik: so lange Stille, bis die Taste als losgelassen gilt."));

    // Radio
    tuneButton.setButtonText ("SENDERWECHSEL");
    tuneButton.setTooltip (u8 ("Dreht den Sender weiter (ca. 1 s Pfeifen und Rauschen). Automatisierbar als \xe2\x80\x9eRadio: Senderwechsel\xe2\x80\x9c, jede \xc3\x84nderung l\xc3\xb6st einmal aus."));
    tuneButton.onClick = [this] { triggerTuning(); };
    content.addAndMakeVisible (tuneButton);
    powerButton.setTooltip (u8 ("Radio ausschalten (Klick, dann Stille) oder wieder einschalten. Automatisierbar als \xe2\x80\x9eRadio: Ein\xe2\x80\x9c."));
    powerButton.onClick = [this] { togglePower(); };
    content.addAndMakeVisible (powerButton);
    setupKnob (intf, "intf", "St\xc3\xb6rungen");
    intf.slider.setTooltip (u8 ("Wie oft und wie stark der Empfang schlecht wird: die Stimme sackt ab und geht ins Gruscheln. aus = nie."));
    radioInfo.setText (u8 ("Senderwechsel: Das Signal ist kurz weg, es pfeift und rauscht, dann rastet der Sender wieder ein.\n\n"
                           "St\xc3\xb6rungen: Die Stimme sackt ab und geht ins Gruscheln, wie bei schlechtem Empfang.\n\n"
                           "Radio: Aus schaltet mit einem Klick Rauschen und Stimme ab."), juce::dontSendNotification);
    radioInfo.setJustificationType (juce::Justification::topLeft);
    content.addAndMakeVisible (radioInfo);

    // Telefon
    setupBox (lineBox, RadiophoneProcessor::lineNames(), "line", attLine);
    setupBox (dialBox, RadiophoneProcessor::dialNames(), "dial", attDial);
    lineBox.setTooltip (u8 ("Zustand der Leitung. Automatisieren f\xc3\xbcr den Ablauf eines Anrufs."));
    dialBox.setTooltip (u8 ("Wie gew\xc3\xa4hlt wird, wenn die Leitung auf W\xc3\xa4hlen steht."));
    lineInfo.setJustificationType (juce::Justification::topLeft);
    dialInfo.setJustificationType (juce::Justification::topLeft);
    dialInfo.setText (u8 ("W\xc3\xa4hlscheibe: Impulse (Knacken), typisch f\xc3\xbcr alte Telefone und die 80er.\n"
                          "Tasten: T\xc3\xb6ne wie beim Tastentelefon."), juce::dontSendNotification);
    content.addAndMakeVisible (lineInfo);
    content.addAndMakeVisible (dialInfo);

    // alle
    setupKnob (sfx, "sfx", "Lautst\xc3\xa4rke");
    sfx.slider.setTooltip (u8 ("Lautst\xc3\xa4rke der eingebauten Ger\xc3\xa4usche (Squelch, Rauschen, Knacken, T\xc3\xb6ne). Die Stimme bleibt gleich."));

    themeButton.onClick = [this]
    {
        theme = theme.dark ? lightTheme() : darkTheme();
        processor.uiTheme.store (theme.dark ? 0 : 1);
        applyTheme();
    };
    content.addAndMakeVisible (themeButton);

    helpButton.setButtonText ("?");
    helpButton.setTooltip (u8 ("Hilfe: was machen die Regler?"));
    helpButton.onClick = [this] { helpScroll.setVisible (! helpScroll.isVisible()); };
    content.addAndMakeVisible (helpButton);

    helpScroll.setViewedComponent (&helpText, false);
    helpScroll.setScrollBarsShown (true, false);
    helpScroll.setVisible (false);
    content.addChildComponent (helpScroll);

    applyTheme();
    updateModeVisibility();

    const int savedW = processor.uiWidth.load();
    const int savedH = processor.uiHeight.load();
    setSize (juce::jlimit (kBaseW / 2, kBaseW * 3, savedW), juce::jlimit (kBaseH / 2, kBaseH * 3, savedH));
    sizeReady = true;
    startTimerHz (15);
}

RadiophoneEditor::~RadiophoneEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

int RadiophoneEditor::currentMode() const
{
    return juce::jlimit (0, 2, juce::roundToInt (processor.apvts.getRawParameterValue ("mode")->load()));
}

void RadiophoneEditor::setMode (int m)
{
    if (auto* prm = processor.apvts.getParameter ("mode"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->convertTo0to1 ((float) m));
        prm->endChangeGesture();
    }
    timerCallback();
}

void RadiophoneEditor::triggerTuning()
{
    if (auto* prm = processor.apvts.getParameter ("tune"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->getValue() > 0.5f ? 0.0f : 1.0f);
        prm->endChangeGesture();
    }
}

void RadiophoneEditor::timerCallback()
{
    const bool wantsDark = processor.uiTheme.load() != 1;
    if (theme.dark != wantsDark) { theme = wantsDark ? darkTheme() : lightTheme(); applyTheme(); }
    const int savedW = processor.uiWidth.load(), savedH = processor.uiHeight.load();
    if (getWidth() != savedW || getHeight() != savedH) setSize (savedW, savedH);
    const int m = currentMode();
    if (m != shownMode)
    {
        shownMode = m;
        updateModeVisibility();
        repaint();
    }
    const bool lamp = m == rph::kFunk ? processor.transmitting.load()
                    : m == rph::kRadio ? processor.tuning.load()
                    : juce::roundToInt (processor.apvts.getRawParameterValue ("line")->load()) == rph::kGespraech;
    const int line = juce::roundToInt (processor.apvts.getRawParameterValue ("line")->load());
    if (lamp != lampOn || line != lastLine)
    {
        lampOn = lamp;
        lastLine = line;
        lineInfo.setText (lineDescription (line), juce::dontSendNotification);
        repaint();
    }
    const bool radioOn = processor.apvts.getRawParameterValue ("power")->load() > 0.5f;
    const juce::String powerText = radioOn ? "RADIO: AN" : "RADIO: AUS";
    if (powerButton.getButtonText() != powerText) { powerButton.setButtonText (powerText); repaint(); }
    // Funk-Statuszeile zeigt Automatik/Taste, darum bei Aenderungen nachziehen
    const int key = juce::roundToInt (processor.apvts.getRawParameterValue ("key")->load());
    if (key != lastKeyShown) { lastKeyShown = key; repaint(); }
}

void RadiophoneEditor::togglePower()
{
    if (auto* prm = processor.apvts.getParameter ("power"))
    {
        prm->beginChangeGesture();
        prm->setValueNotifyingHost (prm->getValue() > 0.5f ? 0.0f : 1.0f);
        prm->endChangeGesture();
    }
}

void RadiophoneEditor::updateModeVisibility()
{
    const int m = currentMode();
    for (int i = 0; i < 3; ++i)
        modeButtons[i].setToggleState (i == m, juce::dontSendNotification);

    const bool funk = m == rph::kFunk, radio = m == rph::kRadio, tel = m == rph::kTelefon;
    for (juce::Component* c : { (juce::Component*) &deviceBox, (juce::Component*) &keyBox,
                                (juce::Component*) &gate.slider, (juce::Component*) &gate.label,
                                (juce::Component*) &pause.slider, (juce::Component*) &pause.label })
        c->setVisible (funk);
    tuneButton.setVisible (radio);
    powerButton.setVisible (radio);
    radioInfo.setVisible (radio);
    intf.slider.setVisible (radio);
    intf.label.setVisible (radio);
    for (juce::Component* c : { (juce::Component*) &lineBox, (juce::Component*) &dialBox,
                                (juce::Component*) &lineInfo, (juce::Component*) &dialInfo })
        c->setVisible (tel);
}

void RadiophoneEditor::applyTheme()
{
    look.setColour (juce::Slider::thumbColourId, theme.accent);
    look.setColour (juce::Slider::rotarySliderFillColourId, theme.accent);
    look.setColour (juce::Slider::rotarySliderOutlineColourId, theme.line);
    look.setColour (juce::Slider::textBoxTextColourId, theme.text);
    look.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    look.setColour (juce::ComboBox::backgroundColourId, theme.bg);
    look.setColour (juce::ComboBox::textColourId, theme.text);
    look.setColour (juce::ComboBox::arrowColourId, theme.text);
    look.setColour (juce::ComboBox::outlineColourId, theme.line);
    look.setColour (juce::PopupMenu::backgroundColourId, theme.panel);
    look.setColour (juce::PopupMenu::textColourId, theme.text);
    look.setColour (juce::PopupMenu::highlightedBackgroundColourId, theme.accent);
    look.setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::black);
    look.setColour (juce::Label::textColourId, theme.text);
    look.setColour (juce::TextButton::buttonColourId, theme.dark ? theme.panel : theme.bg);
    look.setColour (juce::TextButton::buttonOnColourId, theme.accent);
    look.setColour (juce::TextButton::textColourOffId, theme.text);
    look.setColour (juce::TextButton::textColourOnId, juce::Colours::black);

    for (auto* l : { &radioInfo, &lineInfo, &dialInfo })
    {
        l->setColour (juce::Label::textColourId, theme.dim);
        l->setFont (juce::FontOptions (13.0f));
    }
    themeButton.setButtonText (theme.dark ? "Look: Dunkel" : "Look: Hell");
    helpText.setAttributed (buildHelp (theme.text, theme.accent, theme.dim));
    helpScroll.fill = theme.panel;
    helpScroll.edge = theme.line;
    sendLookAndFeelChange();
    repaint();
}

void RadiophoneEditor::setupKnob (Knob& k, const juce::String& paramId, const char* text)
{
    k.slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    k.slider.setMouseDragSensitivity (180);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
    content.addAndMakeVisible (k.slider);
    k.label.setText (u8 (text), juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    content.addAndMakeVisible (k.label);
    k.attachment = std::make_unique<APVTS::SliderAttachment> (processor.apvts, paramId, k.slider);
}

void RadiophoneEditor::setupBox (juce::ComboBox& box, const juce::StringArray& items, const juce::String& paramId,
                                 std::unique_ptr<APVTS::ComboBoxAttachment>& att)
{
    box.addItemList (items, 1);
    content.addAndMakeVisible (box);
    att = std::make_unique<APVTS::ComboBoxAttachment> (processor.apvts, paramId, box);
}

void RadiophoneEditor::paint (juce::Graphics& g)
{
    g.fillAll (theme.bg);
    g.addTransform (juce::AffineTransform::scale (uiScale));
    const int W = kBaseW;
    const int m = currentMode();

    // Kopfleiste wie bei LOCATIONS
    g.setColour (theme.panel);
    g.fillRect (0, 0, W, 64);
    g.setColour (theme.line);
    g.fillRect (0, 63, W, 1);
    g.setColour (theme.accent);
    g.fillRect (0, 0, 5, 64);
    if (logo.isValid())
    {
        g.setColour (theme.text);
        g.drawImage (logo, juce::Rectangle<float> (14.0f, 4.0f, 56.0f, 56.0f), juce::RectanglePlacement::centred, true);
    }

    const juce::Font titleFont (juce::FontOptions (22.0f, juce::Font::bold));
    const juce::Font subFont (juce::FontOptions (11.5f, juce::Font::bold));
    auto textWidth = [] (const juce::Font& f, const juce::String& t)
    {
        juce::GlyphArrangement ga; ga.addLineOfText (f, t, 0.0f, 0.0f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    };
    const juce::String sub = "AUDIO DRAMA RADIO & PHONE SIMULATOR";
    const float subX = 80.0f + textWidth (titleFont, "RADIOPHONE") + 16.0f;
    g.setColour (theme.text);
    g.setFont (titleFont);
    g.drawText ("RADIOPHONE", 80, 9, 200, 28, juce::Justification::centredLeft);
    g.setColour (theme.accent);
    g.setFont (subFont);
    g.drawText (sub, (int) subX, 14, 340, 22, juce::Justification::centredLeft);
    g.setColour (theme.dim);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (u8 ("STROMSCHLAG AUDIO  \xc2\xb7  v" PLUGIN_VERSION_TEXT), 80, 36, 300, 16, juce::Justification::centredLeft);

    // Kleines Symbol: Walkie-Talkie mit Funkwellen (einfarbig, folgt dem Look)
    {
        juce::Path body, grille, ant, waves;
        body.addRoundedRectangle (0.0f, 9.0f, 12.0f, 20.0f, 2.0f);
        ant.startNewSubPath (3.0f, 9.0f);
        ant.lineTo (3.0f, 0.0f);
        for (float y : { 19.0f, 22.0f, 25.0f })
        {
            grille.startNewSubPath (3.0f, y);
            grille.lineTo (9.0f, y);
        }
        grille.addRectangle (3.0f, 12.0f, 6.0f, 4.0f);
        const float cx = 3.0f, cy = 2.0f, half = 0.75f;
        for (float r : { 4.5f, 8.0f, 11.5f })
            waves.addArc (cx - r, cy - r, 2.0f * r, 2.0f * r,
                          juce::MathConstants<float>::halfPi - half, juce::MathConstants<float>::halfPi + half, true);
        const float subEnd = subX + textWidth (subFont, sub);
        const float helpX = (float) (kBaseW - 170);
        const float mid = 0.5f * (subEnd + helpX);
        const auto place = juce::AffineTransform::scale (1.3f).translated (mid - 9.0f, 12.0f);
        for (auto* p : { &body, &grille, &ant, &waves }) p->applyTransform (place);
        const auto ink = theme.panel.interpolatedWith (theme.text, 0.30f);
        const auto fill = theme.panel.interpolatedWith (theme.text, 0.10f);
        g.setColour (fill);
        g.fillPath (body);
        g.setColour (ink);
        g.strokePath (body, juce::PathStrokeType (1.6f));
        g.strokePath (grille, juce::PathStrokeType (1.2f));
        g.strokePath (ant, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.strokePath (waves, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // Obere Karte: Modus + Status
    g.setColour (theme.panel);
    g.fillRoundedRectangle (12.0f, 76.0f, (float) W - 24.0f, 122.0f, 8.0f);
    g.setColour (theme.line);
    g.drawRoundedRectangle (12.5f, 76.5f, (float) W - 25.0f, 121.0f, 8.0f, 1.0f);
    g.setColour (theme.dim);
    g.setFont (juce::FontOptions (12.0f));
    g.drawText ("MODUS", 24, 80, 200, 18, juce::Justification::centredLeft);

    // Statuszeile mit Lampe
    juce::String status, detail;
    if (m == rph::kFunk)
    {
        const int key = juce::roundToInt (processor.apvts.getRawParameterValue ("key")->load());
        status = lampOn ? "SENDET" : "STILL";
        detail = key == rph::kKeyAuto ? u8 ("Sprechtaste: Automatik \xe2\x80\x93 \xc3\xb6\x66\x66net beim ersten Wort, schlie\xc3\x9ft nach der Pause")
               : key == rph::kKeyDown ? u8 ("Sprechtaste: gedr\xc3\xbc\x63kt (per Automation)")
                                      : u8 ("Sprechtaste: losgelassen \xe2\x80\x93 Empf\xc3\xa4nger stumm");
    }
    else if (m == rph::kRadio)
    {
        const bool radioOn = processor.apvts.getRawParameterValue ("power")->load() > 0.5f;
        status = ! radioOn ? "AUS" : lampOn ? "SENDERWECHSEL" : "EMPFANG";
        detail = radioOn ? u8 ("Grundrauschen und gelegentliches Knacken laufen leise mit")
                         : u8 ("Radio aus \xe2\x80\x93 Rauschen und Stimme sind stumm");
    }
    else
    {
        status = RadiophoneProcessor::lineNames()[juce::jlimit (0, 5, lastLine < 0 ? 0 : lastLine)].toUpperCase()
                     .replace (juce::String::fromUTF8 ("\xc3\xa4"), juce::String::fromUTF8 ("\xc3\x84"))
                     .replace (juce::String::fromUTF8 ("\xc3\xb6"), juce::String::fromUTF8 ("\xc3\x96"))
                     .replace (juce::String::fromUTF8 ("\xc3\xbc"), juce::String::fromUTF8 ("\xc3\x9c"));   // toUpperCase laesst Umlaute klein
        detail = lampOn ? u8 ("Stimme ist zu h\xc3\xb6ren") : u8 ("Stimme ist stumm (nur bei Gespr\xc3\xa4\x63h zu h\xc3\xb6ren)");
    }
    const juce::Rectangle<float> lamp (26.0f, 162.0f, 16.0f, 16.0f);
    g.setColour (lampOn ? theme.accent : theme.line);
    g.fillEllipse (lamp);
    if (lampOn)
    {
        g.setColour (theme.accent.withAlpha (0.25f));
        g.fillEllipse (lamp.expanded (5.0f));
    }
    g.setColour (lampOn ? theme.accent : theme.dim);
    g.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    g.drawText (status, 52, 160, 170, 20, juce::Justification::centredLeft);
    g.setColour (theme.dim);
    g.setFont (juce::FontOptions (12.5f));
    g.drawText (detail, 200, 160, W - 224, 20, juce::Justification::centredLeft);

    // Untere Kaesten je nach Modus
    struct Box { float x, w; juce::String title, note; };
    std::vector<Box> boxes;
    if (m == rph::kFunk)
        boxes = { { kAx, kAw, u8 ("GER\xc3\x84T"), "" }, { kBx, kBw, "SPRECHTASTE", "" } };
    else if (m == rph::kRadio)
        boxes = { { kAx, kBx + kBw - kAx, "RADIO", "" } };
    else
        boxes = { { kAx, kAw, "LEITUNG", "automatisierbar" }, { kBx, kBw, u8 ("W\xc3\x84HLEN"), "" } };
    boxes.push_back ({ kCx, kCw, u8 ("GER\xc3\x84USCHE"), "" });

    for (const auto& b : boxes)
    {
        g.setColour (theme.panel);
        g.fillRoundedRectangle (b.x, kBoxY, b.w, kBoxH, 8.0f);
        g.setColour (theme.line);
        g.drawRoundedRectangle (b.x + 0.5f, kBoxY + 0.5f, b.w - 1.0f, kBoxH - 1.0f, 8.0f, 1.0f);
        g.setColour (theme.accent);
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (b.title, (int) b.x + 12, (int) kBoxY + 6, (int) b.w - 24, 16, juce::Justification::centredLeft);
        g.setColour (theme.dim);
        g.setFont (juce::FontOptions (10.5f));
        g.drawText (b.note, (int) b.x + 12, (int) kBoxY + 6, (int) b.w - 24, 16, juce::Justification::centredRight);
    }
}

void RadiophoneEditor::resized()
{
    if (sizeReady)
    {
        processor.uiWidth.store (getWidth());
        processor.uiHeight.store (getHeight());
    }
    uiScale = std::min ((float) getWidth() / kBaseW, (float) getHeight() / kBaseH);
    content.setTransform (juce::AffineTransform::scale (uiScale));
    content.setBounds (0, 0, kBaseW, kBaseH);
    repaint();

    const int w = kBaseW;
    themeButton.setBounds (w - 170, 18, 112, 28);
    helpButton.setBounds (w - 48, 18, 32, 28);
    helpScroll.setBounds (12, 76, w - 24, 332);
    helpText.setWidth (helpScroll.getMaximumVisibleWidth());

    // Modus-Knoepfe
    const int bx = 24, bw = (w - 48 - 20) / 3;
    for (int i = 0; i < 3; ++i)
        modeButtons[i].setBounds (bx + i * (bw + 10), 102, bw, 40);

    auto placeKnob = [] (Knob& k, int x, int y, int cw, int h)
    {
        k.label.setBounds (x, y, cw, 18);
        k.slider.setBounds (x + 10, y + 18, cw - 20, h);
    };
    const int top = (int) kBoxY + 28;   // erste Zeile in den Kaesten

    // Funk
    deviceBox.setBounds ((int) kAx + 12, top, (int) kAw - 24, 28);
    placeKnob (gate, (int) kAx, top + 38, (int) kAw, 112);
    keyBox.setBounds ((int) kBx + 12, top, (int) kBw - 24, 28);
    placeKnob (pause, (int) kBx, top + 38, (int) kBw, 112);

    // Radio
    tuneButton.setBounds ((int) kAx + 12, top + 4, 160, 44);
    powerButton.setBounds ((int) kAx + 12, top + 56, 160, 44);
    placeKnob (intf, (int) kAx + 190, top, 130, 112);
    radioInfo.setBounds ((int) kAx + 332, top, (int) (kBx + kBw - kAx) - 344, 160);

    // Telefon
    lineBox.setBounds ((int) kAx + 12, top, (int) kAw - 24, 28);
    lineInfo.setBounds ((int) kAx + 8, top + 36, (int) kAw - 16, 120);
    dialBox.setBounds ((int) kBx + 12, top, (int) kBw - 24, 28);
    dialInfo.setBounds ((int) kBx + 8, top + 36, (int) kBw - 16, 120);

    // Geraeusche
    placeKnob (sfx, (int) kCx, top + 10, (int) kCw, 130);

    helpScroll.toFront (false);
}
