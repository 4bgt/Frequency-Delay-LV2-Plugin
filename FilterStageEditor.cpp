#include "FilterStageEditor.h"

FilterStageEditor::FilterStageEditor(juce::AudioProcessorValueTreeState& vts) : apvts(vts)
{
    auto setupKnob = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);
    };

    setupKnob(cutoffSlider, cutoffLabel, "Cutoff");
    cutoffAttach = std::make_unique<SliderAttachment>(apvts, "CUTOFF", cutoffSlider);

    setupKnob(widthSlider, widthLabel, "Width/Reso");
    widthAttach = std::make_unique<SliderAttachment>(apvts, "WIDTH", widthSlider);

    typeLabel.setText("Filter Type", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(typeLabel);
    filterTypeBox.addItemList({"High-Pass", "Low-Pass", "Band-Pass"}, 1);
    addAndMakeVisible(filterTypeBox);
    filterTypeAttach = std::make_unique<ComboBoxAttachment>(apvts, "FILTERTYPE", filterTypeBox);

    invertLabel.setText("Invert", juce::dontSendNotification);
    invertLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(invertLabel);
    addAndMakeVisible(invertButton);
    invertAttach = std::make_unique<ButtonAttachment>(apvts, "INVERT", invertButton);

    setupKnob(lfoDepthSlider, lfoDepthLabel, "LFO Depth");
    lfoDepthAttach = std::make_unique<SliderAttachment>(apvts, "LFODEPTH", lfoDepthSlider);

    setupKnob(lfoRateSlider, lfoRateLabel, "LFO Rate");
    lfoRateAttach = std::make_unique<SliderAttachment>(apvts, "LFORATE", lfoRateSlider);

    lfoSyncLabel.setText("LFO Sync", juce::dontSendNotification);
    lfoSyncLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoSyncLabel);
    addAndMakeVisible(lfoSyncButton);
    lfoSyncAttach = std::make_unique<ButtonAttachment>(apvts, "LFOSYNC", lfoSyncButton);

    lfoSyncRateLabel.setText("LFO Rate", juce::dontSendNotification);
    lfoSyncRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoSyncRateLabel);
    
    juce::StringArray syncOptions = {
        "1/2", "1/2 Dotted", "1/2 Triplet",
        "1/4", "1/4 Dotted", "1/4 Triplet",
        "1/8", "1/8 Dotted", "1/8 Triplet",
        "1/16", "1/16 Dotted", "1/16 Triplet",
        "1/32", "1/32 Dotted", "1/32 Triplet"
    };
    lfoSyncRateBox.addItemList(syncOptions, 1);
    addAndMakeVisible(lfoSyncRateBox);
    lfoSyncRateAttach = std::make_unique<ComboBoxAttachment>(apvts, "LFOSYNCRATE", lfoSyncRateBox);

    filterTypeBox.onChange = [this] {
        bool isBandpass = (filterTypeBox.getSelectedId() == 3);
        invertButton.setEnabled(isBandpass); 
    };
    filterTypeBox.onChange(); 

    lfoSyncButton.onClick = [this] {
        bool isSynced = lfoSyncButton.getToggleState();
        lfoRateSlider.setEnabled(!isSynced);
        lfoSyncRateBox.setEnabled(isSynced);
    };
    lfoSyncButton.onClick(); 
}

FilterStageEditor::~FilterStageEditor() {}

void FilterStageEditor::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black.withAlpha(0.1f));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
}

void FilterStageEditor::resized()
{
    int knobSize = 80;
    int labelHeight = 20;
    int spacing = 20;
    
    // ROW 1
    int currentX = 10;
    int currentY = 10;

    cutoffLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    cutoffSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    widthLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    widthSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    typeLabel.setBounds(currentX, currentY, 120, labelHeight);
    filterTypeBox.setBounds(currentX, currentY + labelHeight + 25, 120, 30); 
    currentX += 120 + spacing; 
    
    invertLabel.setBounds(currentX, currentY + 15, 60, labelHeight);
    invertButton.setBounds(currentX + 20, currentY + labelHeight + 20, 40, 30);

    // ROW 2
    currentX = 10;
    currentY = 130;

    lfoDepthLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    lfoDepthSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    lfoRateLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    lfoRateSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    lfoSyncLabel.setBounds(currentX, currentY, 100, labelHeight);
    lfoSyncButton.setBounds(currentX + 30, currentY + labelHeight, 40, 30);
    
    // Widened box to comfortably fit "1/16 Dotted"
    lfoSyncRateLabel.setBounds(currentX + 90, currentY, 110, labelHeight);
    lfoSyncRateBox.setBounds(currentX + 90, currentY + labelHeight, 110, 30);
}