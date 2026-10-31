#pragma once
#include <JuceHeader.h>

class MonitorUtilityAudioProcessor : public juce::AudioProcessor
{
public:
    MonitorUtilityAudioProcessor();
    ~MonitorUtilityAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Monitor Utility"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    Filter bassL, bassR, midHpL, midHpR, midLpL, midLpR, highL, highR;

    double currentSampleRate = 44100.0;
    float lastBass = -1.0f, lastMidLow = -1.0f, lastMidHigh = -1.0f, lastHigh = -1.0f;

    void updateFilters(float bassHz, float midLowHz, float midHighHz, float highHz);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MonitorUtilityAudioProcessor)
};
