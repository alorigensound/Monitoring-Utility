#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class MonitorUtilityAudioProcessorEditor : public juce::AudioProcessorEditor,
                                           private juce::KeyListener
{
public:
    explicit MonitorUtilityAudioProcessorEditor(MonitorUtilityAudioProcessor&);
    ~MonitorUtilityAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    bool keyPressed(const juce::KeyPress&, juce::Component*) override;

    MonitorUtilityAudioProcessor& audioProcessor;
    juce::Slider bass, midLow, midHigh, highs;
    juce::ToggleButton soloBass { "SOLO BASS" }, soloMids { "SOLO MIDS" },
                       soloHighs { "SOLO HIGHS" }, monitorMono { "MONITOR MID" };
    juce::Label bassTitle, midsTitle, highsTitle, monoTitle, header;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> bassAttachment, midLowAttachment, midHighAttachment, highsAttachment;
    std::unique_ptr<ButtonAttachment> bassButtonAttachment, midsButtonAttachment, highsButtonAttachment, monoButtonAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MonitorUtilityAudioProcessorEditor)
};
