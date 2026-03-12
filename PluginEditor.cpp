#include "PluginProcessor.h"
#include "PluginEditor.h"

FrequencyDelayAudioProcessorEditor::FrequencyDelayAudioProcessorEditor (FrequencyDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), filterEditor(p.apvts) 
{
    addAndMakeVisible(filterEditor);

    auto setupKnob = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        addAndMakeVisible(slider);
    };

    setupKnob(timeSlider, timeLabel, "Time");
    timeAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TIME", timeSlider);

    setupKnob(feedbackSlider, feedbackLabel, "Feedback");
    feedbackAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEEDBACK", feedbackSlider);

    setupKnob(mixSlider, mixLabel, "Mix");
    mixAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MIX", mixSlider);

    setupKnob(panRateSlider, panRateLabel, "Pan Rate");
    panRateAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PANRATE", panRateSlider);

    setupKnob(panDepthSlider, panDepthLabel, "Pan Depth");
    panDepthAttach = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PANDEPTH", panDepthSlider);

    juce::StringArray syncOptions = {
        "1/2", "1/2 Dotted", "1/2 Triplet",
        "1/4", "1/4 Dotted", "1/4 Triplet",
        "1/8", "1/8 Dotted", "1/8 Triplet",
        "1/16", "1/16 Dotted", "1/16 Triplet",
        "1/32", "1/32 Dotted", "1/32 Triplet"
    };

    // Delay Sync UI
    syncLabel.setText("Delay Sync", juce::dontSendNotification);
    syncLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(syncLabel);
    addAndMakeVisible(syncButton);
    syncAttach = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "SYNC", syncButton);

    syncRateLabel.setText("Delay Rate", juce::dontSendNotification);
    syncRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(syncRateLabel);
    syncRateBox.addItemList(syncOptions, 1);
    addAndMakeVisible(syncRateBox);
    syncRateAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "SYNCRATE", syncRateBox);

    syncButton.onClick = [this] {
        bool isSynced = syncButton.getToggleState();
        timeSlider.setEnabled(!isSynced);
        syncRateBox.setEnabled(isSynced);
    };
    syncButton.onClick();

    // Pan Sync UI
    panSyncLabel.setText("Pan Sync", juce::dontSendNotification);
    panSyncLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(panSyncLabel);
    addAndMakeVisible(panSyncButton);
    panSyncAttach = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "PANSYNC", panSyncButton);

    panSyncRateLabel.setText("Pan Rate", juce::dontSendNotification);
    panSyncRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(panSyncRateLabel);
    panSyncRateBox.addItemList(syncOptions, 1);
    addAndMakeVisible(panSyncRateBox);
    panSyncRateAttach = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "PANSYNCRATE", panSyncRateBox);

    panSyncButton.onClick = [this] {
        bool isSynced = panSyncButton.getToggleState();
        panRateSlider.setEnabled(!isSynced);
        panSyncRateBox.setEnabled(isSynced);
    };
    panSyncButton.onClick();

    setSize (520, 520);
}

FrequencyDelayAudioProcessorEditor::~FrequencyDelayAudioProcessorEditor() {}

void FrequencyDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void FrequencyDelayAudioProcessorEditor::resized()
{
    filterEditor.setBounds(10, 10, 500, 240);

    int knobSize = 80;
    int labelHeight = 20;
    int spacing = 20;
    
    // DELAY ROW 1
    int currentX = 20;
    int currentY = 260; 

    timeLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    timeSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    feedbackLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    feedbackSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    mixLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    mixSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    syncLabel.setBounds(currentX, currentY, 100, labelHeight);
    syncButton.setBounds(currentX + 30, currentY + labelHeight, 40, 30);
    
    syncRateLabel.setBounds(currentX + 90, currentY, 110, labelHeight);
    syncRateBox.setBounds(currentX + 90, currentY + labelHeight, 110, 30);

    // DELAY ROW 2 (Panning Controls)
    currentX = 20;
    currentY = 380;

    panDepthLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    panDepthSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    panRateLabel.setBounds(currentX, currentY, knobSize, labelHeight);
    panRateSlider.setBounds(currentX, currentY + labelHeight, knobSize, knobSize);
    currentX += knobSize + spacing;

    panSyncLabel.setBounds(currentX, currentY, 100, labelHeight);
    panSyncButton.setBounds(currentX + 30, currentY + labelHeight, 40, 30);
    
    panSyncRateLabel.setBounds(currentX + 90, currentY, 110, labelHeight);
    panSyncRateBox.setBounds(currentX + 90, currentY + labelHeight, 110, 30);
}