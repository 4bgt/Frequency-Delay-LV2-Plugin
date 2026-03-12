#include "FilterStage.h"

void FilterStage::prepare(const juce::dsp::ProcessSpec& spec)
{
    juce::dsp::ProcessSpec monoSpec = spec;
    monoSpec.numChannels = 1;

    for (auto& filter : filters) {
        filter.prepare(monoSpec);
        filter.reset();
    }
    
    lfo.prepare(spec);
    lfo.initialise([](float x) { return std::sin(x); });
}

void FilterStage::tickLFO(float frequencyHz)
{
    lfo.setFrequency(frequencyHz);
    currentLfoVal = lfo.processSample(0.0f);
}

float FilterStage::processSample(int channel, float input, float cutoff, float width, int filterType, bool isInverted, float lfoDepth)
{
    float w = std::pow(2.0f, width);
    float effectiveQ = std::sqrt(w) / (w - 1.0f);

    auto type = juce::dsp::StateVariableTPTFilterType::highpass;
    if (filterType == 1) type = juce::dsp::StateVariableTPTFilterType::lowpass;
    if (filterType == 2) type = juce::dsp::StateVariableTPTFilterType::bandpass;

    float maxOctaves = 4.0f;
    float modulatedCutoff = cutoff * std::pow(2.0f, currentLfoVal * lfoDepth * maxOctaves);
    modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);

    filters[channel].setType(type);
    filters[channel].setCutoffFrequency(modulatedCutoff);
    filters[channel].setResonance(effectiveQ);

    float filtered = filters[channel].processSample(0, input);
    
    if (filterType == 2 && isInverted) {
        filtered = input - filtered;
    }

    return filtered;
}

void FilterStage::addFilterParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"CUTOFF", 1}, "Cutoff", juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 1000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"WIDTH", 1}, "Width/Reso", juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.3f), 0.707f));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"FILTERTYPE", 1}, "Type", juce::StringArray{"High-Pass", "Low-Pass", "Band-Pass"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"INVERT", 1}, "Invert (Notch)", false));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"LFODEPTH", 1}, "LFO Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"LFORATE", 1}, "LFO Rate", juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.5f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"LFOSYNC", 1}, "LFO Sync", false));
    
    // The Master 15 Sync Options
    juce::StringArray syncOptions = {
        "1/2", "1/2 Dotted", "1/2 Triplet",
        "1/4", "1/4 Dotted", "1/4 Triplet",
        "1/8", "1/8 Dotted", "1/8 Triplet",
        "1/16", "1/16 Dotted", "1/16 Triplet",
        "1/32", "1/32 Dotted", "1/32 Triplet"
    };
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"LFOSYNCRATE", 1}, "LFO Sync Rate", syncOptions, 3)); // 3 = 1/4 Note Default
}