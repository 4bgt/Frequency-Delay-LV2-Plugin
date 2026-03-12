#pragma once
#include <JuceHeader.h>
#include <array>

class FilterStage 
{
public:
    FilterStage() = default;

    void prepare(const juce::dsp::ProcessSpec& spec);
    void tickLFO(float frequencyHz);
    float processSample(int channel, float input, float cutoff, float width, int filterType, bool isInverted, float lfoDepth);

    static void addFilterParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

private:
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> filters;
    juce::dsp::Oscillator<float> lfo;
    float currentLfoVal = 0.0f;
};