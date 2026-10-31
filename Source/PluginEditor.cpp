#include "PluginEditor.h"

namespace
{
    void styleKnob(juce::Slider& s)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
        s.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff3280fc));
        s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff343943));
        s.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd1d5db));
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffb9c0cc));
        s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff111318));
        s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff30343c));
    }
    void styleButton(juce::ToggleButton& b)
    {
        b.setClickingTogglesState(true);
        b.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe5e7eb));
        b.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff3280fc));
        b.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff454b56));
    }
}

MonitorUtilityAudioProcessorEditor::MonitorUtilityAudioProcessorEditor(MonitorUtilityAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(800, 600);
    addKeyListener(this);
    setWantsKeyboardFocus(true);

    header.setText("MONITOR UTILITY", juce::dontSendNotification);
    header.setJustificationType(juce::Justification::centred);
    header.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    header.setFont(juce::Font(13.0f, juce::Font::bold));
    addAndMakeVisible(header);

    auto setupTitle = [this](juce::Label& l, const juce::String& t) {
        l.setText(t, juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        l.setColour(juce::Label::textColourId, juce::Colour(0xff3280fc));
        l.setFont(juce::Font(13.0f, juce::Font::bold));
        addAndMakeVisible(l);
    };
    setupTitle(bassTitle, "1. BASS");
    setupTitle(midsTitle, "2. MIDS");
    setupTitle(highsTitle, "3. HIGHS");
    setupTitle(monoTitle, "4. MID (MONO)");

    for (auto* s : { &bass, &midLow, &midHigh, &highs }) { styleKnob(*s); addAndMakeVisible(*s); }
    for (auto* b : { &soloBass, &soloMids, &soloHighs, &monitorMono }) { styleButton(*b); addAndMakeVisible(*b); }

    bass.setName("Cutoff"); midLow.setName("Low Cut"); midHigh.setName("High Cut"); highs.setName("Cutoff");
    bassAttachment = std::make_unique<SliderAttachment>(p.parameters, "BassCutoff", bass);
    midLowAttachment = std::make_unique<SliderAttachment>(p.parameters, "MidLowCutoff", midLow);
    midHighAttachment = std::make_unique<SliderAttachment>(p.parameters, "MidHighCutoff", midHigh);
    highsAttachment = std::make_unique<SliderAttachment>(p.parameters, "HighCutoff", highs);
    bassButtonAttachment = std::make_unique<ButtonAttachment>(p.parameters, "SoloBass", soloBass);
    midsButtonAttachment = std::make_unique<ButtonAttachment>(p.parameters, "SoloMids", soloMids);
    highsButtonAttachment = std::make_unique<ButtonAttachment>(p.parameters, "SoloHighs", soloHighs);
    monoButtonAttachment = std::make_unique<ButtonAttachment>(p.parameters, "MonitorMid", monitorMono);
}

void MonitorUtilityAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff090b0f));
    auto outer = getLocalBounds().reduced(18);
    g.setColour(juce::Colour(0xff14171d));
    g.fillRoundedRectangle(outer.toFloat(), 14.0f);
    g.setColour(juce::Colour(0xff303641));
    g.drawRoundedRectangle(outer.toFloat(), 14.0f, 1.0f);

    auto area = outer.reduced(14);
    area.removeFromTop(34);
    const int gap = 10;
    const int w = (area.getWidth() - 3 * gap) / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto panel = juce::Rectangle<int>(area.getX() + i * (w + gap), area.getY(), w, area.getHeight());
        g.setColour(juce::Colour(0xff1b1f27));
        g.fillRoundedRectangle(panel.toFloat(), 10.0f);
        g.setColour(juce::Colour(0xff343a45));
        g.drawRoundedRectangle(panel.toFloat(), 10.0f, 1.0f);
    }
    g.setColour(juce::Colour(0xff343a45));
    g.drawLine(static_cast<float>(outer.getX() + 14), static_cast<float>(outer.getY() + 39),
               static_cast<float>(outer.getRight() - 14), static_cast<float>(outer.getY() + 39), 1.0f);
}

void MonitorUtilityAudioProcessorEditor::resized()
{
    auto outer = getLocalBounds().reduced(32);
    header.setBounds(outer.removeFromTop(34));
    outer.removeFromTop(24);
    const int gap = 10;
    const int w = (outer.getWidth() - 3 * gap) / 4;

    auto placePanel = [&](int index, juce::Label& title, std::initializer_list<juce::Component*> controls) {
        auto panel = juce::Rectangle<int>(outer.getX() + index * (w + gap), outer.getY(), w, outer.getHeight()).reduced(10);
        title.setBounds(panel.removeFromTop(30));
        panel.removeFromTop(12);
        for (auto* c : controls)
        {
            if (c == &midLow || c == &midHigh)
            {
                const int half = (panel.getWidth() - 4) / 2;
                c->setBounds(panel.removeFromTop(170).withWidth(half));
                // The second mids knob is placed beside the first below.
            }
            else
            {
                c->setBounds(panel.removeFromTop(190).withSizeKeepingCentre(110, 170));
            }
            panel.removeFromTop(8);
        }
    };

    // Explicit bounds keep the four modules balanced and readable.
    auto panel = [&](int i) { return juce::Rectangle<int>(outer.getX() + i * (w + gap), outer.getY(), w, outer.getHeight()).reduced(10); };
    auto p0 = panel(0); bassTitle.setBounds(p0.removeFromTop(30)); bass.setBounds(p0.removeFromTop(220).withSizeKeepingCentre(112, 190)); p0.removeFromTop(8); soloBass.setBounds(p0.removeFromTop(42));
    auto p1 = panel(1); midsTitle.setBounds(p1.removeFromTop(30));
    auto knobs = p1.removeFromTop(220); const int half = (knobs.getWidth() - 4) / 2;
    midLow.setBounds(knobs.removeFromLeft(half)); knobs.removeFromLeft(4); midHigh.setBounds(knobs);
    p1.removeFromTop(8); soloMids.setBounds(p1.removeFromTop(42));
    auto p2 = panel(2); highsTitle.setBounds(p2.removeFromTop(30)); highs.setBounds(p2.removeFromTop(220).withSizeKeepingCentre(112, 190)); p2.removeFromTop(8); soloHighs.setBounds(p2.removeFromTop(42));
    auto p3 = panel(3); monoTitle.setBounds(p3.removeFromTop(30));
    auto indicator = p3.removeFromTop(220);
    monitorMono.setBounds(p3.removeFromTop(42));
    // The toggle button is the mono status/action indicator in this native build.
}

bool MonitorUtilityAudioProcessorEditor::keyPressed(const juce::KeyPress& key, juce::Component*)
{
    const auto c = juce::CharacterFunctions::toLowerCase(key.getTextCharacter());
    if (c == '1') soloBass.setToggleState(!soloBass.getToggleState(), juce::sendNotification);
    else if (c == '2') soloMids.setToggleState(!soloMids.getToggleState(), juce::sendNotification);
    else if (c == '3') soloHighs.setToggleState(!soloHighs.getToggleState(), juce::sendNotification);
    else if (c == '4') monitorMono.setToggleState(!monitorMono.getToggleState(), juce::sendNotification);
    else return false;
    return true;
}
