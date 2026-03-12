#pragma once
#include <JuceHeader.h>

class FilterStageEditor : public juce::Component
{
public:
    FilterStageEditor(juce::AudioProcessorValueTreeState& vts);
    ~FilterStageEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;

    juce::Slider cutoffSlider, widthSlider, lfoDepthSlider, lfoRateSlider;
    juce::ComboBox filterTypeBox, lfoSyncRateBox;
    juce::ToggleButton invertButton, lfoSyncButton;
    juce::Label cutoffLabel, widthLabel, typeLabel, invertLabel, lfoDepthLabel, lfoRateLabel, lfoSyncLabel, lfoSyncRateLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> cutoffAttach, widthAttach, lfoDepthAttach, lfoRateAttach;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttach, lfoSyncRateAttach;
    std::unique_ptr<ButtonAttachment> invertAttach, lfoSyncAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterStageEditor)
};