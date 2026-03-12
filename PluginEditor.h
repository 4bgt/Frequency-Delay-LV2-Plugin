#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "../../SharedDSP/FilterStageEditor.h"

class FrequencyDelayAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    FrequencyDelayAudioProcessorEditor (FrequencyDelayAudioProcessor&);
    ~FrequencyDelayAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    FrequencyDelayAudioProcessor& audioProcessor;
    FilterStageEditor filterEditor;

    // Delay & Pan Controls
    juce::Slider mixSlider, timeSlider, feedbackSlider, panRateSlider, panDepthSlider;
    
    // ADDED the pan sync box and button here
    juce::ComboBox syncRateBox, panSyncRateBox;
    juce::ToggleButton syncButton, panSyncButton;
    juce::Label mixLabel, syncLabel, syncRateLabel, timeLabel, feedbackLabel, panRateLabel, panDepthLabel, panSyncLabel, panSyncRateLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // ADDED the pan sync attachments here
    std::unique_ptr<SliderAttachment> mixAttach, timeAttach, feedbackAttach, panRateAttach, panDepthAttach;
    std::unique_ptr<ComboBoxAttachment> syncRateAttach, panSyncRateAttach;
    std::unique_ptr<ButtonAttachment> syncAttach, panSyncAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyDelayAudioProcessorEditor)
};