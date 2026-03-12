#pragma once
#include <JuceHeader.h>
#include "../../SharedDSP/FilterStage.h" 

class FrequencyDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    FrequencyDelayAudioProcessor();
    ~FrequencyDelayAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts; 

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    FilterStage filterStage;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayLine { 1536000 };
    
    // The new panning LFO
    juce::dsp::Oscillator<float> panLfo;

    // Smoothed values to prevent zipper noise
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDelayTime;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedFeedback;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedMix;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedWidth;

    // Safe fallback for DAW stop bug
    float lastValidBpm = 120.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyDelayAudioProcessor)
};