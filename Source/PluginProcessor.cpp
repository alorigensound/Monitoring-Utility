#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float butterworthQ = 0.70710678f;
    constexpr float minDb = -1.0f;
    constexpr float maxDb = 1.0f;

    juce::AudioProcessorValueTreeState::ParameterLayout makeLayout()
    {
        using Float = juce::AudioParameterFloat;
        using Bool = juce::AudioParameterBool;
        juce::AudioProcessorValueTreeState::ParameterLayout p;

        p.add(std::make_unique<Float>("BassCutoff", "Bass Cutoff",
            juce::NormalisableRange<float>(20.0f, 1000.0f, 1.0f, 0.35f), 250.0f, "Hz"));
        p.add(std::make_unique<Float>("MidLowCutoff", "Mids Low Cut",
            juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.35f), 270.0f, "Hz"));
        p.add(std::make_unique<Float>("MidHighCutoff", "Mids High Cut",
            juce::NormalisableRange<float>(500.0f, 16000.0f, 1.0f, 0.35f), 3800.0f, "Hz"));
        p.add(std::make_unique<Float>("HighCutoff", "High Cutoff",
            juce::NormalisableRange<float>(500.0f, 16000.0f, 1.0f, 0.35f), 3500.0f, "Hz"));

        p.add(std::make_unique<Bool>("SoloBass", "Solo Bass", false));
        p.add(std::make_unique<Bool>("SoloMids", "Solo Mids", false));
        p.add(std::make_unique<Bool>("SoloHighs", "Solo Highs", false));
        p.add(std::make_unique<Bool>("MonitorMid", "Monitor Mid Mono", false));
        return p;
    }
}

MonitorUtilityAudioProcessor::MonitorUtilityAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MonitorUtilityAudioProcessor::createParameterLayout()
{
    return makeLayout();
}

bool MonitorUtilityAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void MonitorUtilityAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    const juce::dsp::ProcessSpec spec { sr, 1u, 1u };
    bassL.prepare(spec); bassR.prepare(spec);
    midHpL.prepare(spec); midHpR.prepare(spec);
    midLpL.prepare(spec); midLpR.prepare(spec);
    highL.prepare(spec); highR.prepare(spec);

    bassL.reset(); bassR.reset(); midHpL.reset(); midHpR.reset();
    midLpL.reset(); midLpR.reset(); highL.reset(); highR.reset();

    lastBass = lastMidLow = lastMidHigh = lastHigh = -1.0f;
    updateFilters(250.0f, 270.0f, 3800.0f, 3500.0f);
}

void MonitorUtilityAudioProcessor::updateFilters(float bassHz, float midLowHz, float midHighHz, float highHz)
{
    const auto nyquistLimit = static_cast<float>(currentSampleRate * 0.45);
    bassHz = juce::jlimit(20.0f, nyquistLimit, bassHz);
    midLowHz = juce::jlimit(20.0f, nyquistLimit, midLowHz);
    midHighHz = juce::jlimit(30.0f, nyquistLimit, juce::jmax(midHighHz, midLowHz + 5.0f));
    highHz = juce::jlimit(20.0f, nyquistLimit, highHz);

    auto lpBass = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, bassHz, butterworthQ);
    auto hpMid = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, midLowHz, butterworthQ);
    auto lpMid = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, midHighHz, butterworthQ);
    auto hpHigh = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, highHz, butterworthQ);

    *bassL.coefficients = *lpBass; *bassR.coefficients = *lpBass;
    *midHpL.coefficients = *hpMid; *midHpR.coefficients = *hpMid;
    *midLpL.coefficients = *lpMid; *midLpR.coefficients = *lpMid;
    *highL.coefficients = *hpHigh; *highR.coefficients = *hpHigh;

    lastBass = bassHz; lastMidLow = midLowHz; lastMidHigh = midHighHz; lastHigh = highHz;
}

void MonitorUtilityAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = getTotalNumInputChannels();
    const int samples = buffer.getNumSamples();

    for (int ch = getTotalNumOutputChannels(); ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, samples);

    const auto bassFreq = parameters.getRawParameterValue("BassCutoff")->load();
    const auto midLowFreq = parameters.getRawParameterValue("MidLowCutoff")->load();
    const auto midHighFreq = parameters.getRawParameterValue("MidHighCutoff")->load();
    const auto highFreq = parameters.getRawParameterValue("HighCutoff")->load();

    if (bassFreq != lastBass || midLowFreq != lastMidLow || midHighFreq != lastMidHigh || highFreq != lastHigh)
        updateFilters(bassFreq, midLowFreq, midHighFreq, highFreq);

    const bool soloBass = parameters.getRawParameterValue("SoloBass")->load() > 0.5f;
    const bool soloMids = parameters.getRawParameterValue("SoloMids")->load() > 0.5f;
    const bool soloHighs = parameters.getRawParameterValue("SoloHighs")->load() > 0.5f;
    const bool mono = parameters.getRawParameterValue("MonitorMid")->load() > 0.5f;
    const bool anySolo = soloBass || soloMids || soloHighs;

    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < samples; ++i)
    {
        const float dryL = left[i];
        const float dryR = right != nullptr ? right[i] : dryL;
        const float inL = mono ? 0.5f * (dryL + dryR) : dryL;
        const float inR = mono ? 0.5f * (dryL + dryR) : dryR;

        // Keep all filter states advancing even when a band is not selected.
        const float bassOutL = bassL.processSample(0, inL);
        const float bassOutR = bassR.processSample(0, inR);
        const float midsOutL = midLpL.processSample(0, midHpL.processSample(0, inL));
        const float midsOutR = midLpR.processSample(0, midHpR.processSample(0, inR));
        const float highsOutL = highL.processSample(0, inL);
        const float highsOutR = highR.processSample(0, inR);

        float outL = inL, outR = inR;
        if (anySolo)
        {
            outL = outR = 0.0f;
            if (soloBass) { outL += bassOutL; outR += bassOutR; }
            if (soloMids) { outL += midsOutL; outR += midsOutR; }
            if (soloHighs) { outL += highsOutL; outR += highsOutR; }
        }

        left[i] = juce::jlimit(-1.0f, 1.0f, outL);
        if (right != nullptr)
            right[i] = juce::jlimit(-1.0f, 1.0f, outR);
    }
}

void MonitorUtilityAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void MonitorUtilityAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonitorUtilityAudioProcessor();
}
