#include "PluginProcessor.h"
#include "PluginEditor.h"

FrequencyDelayAudioProcessor::FrequencyDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{}

FrequencyDelayAudioProcessor::~FrequencyDelayAudioProcessor() {}
const juce::String FrequencyDelayAudioProcessor::getName() const { return JucePlugin_Name; }
bool FrequencyDelayAudioProcessor::acceptsMidi() const { return false; }
bool FrequencyDelayAudioProcessor::producesMidi() const { return false; }
bool FrequencyDelayAudioProcessor::isMidiEffect() const { return false; }
double FrequencyDelayAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int FrequencyDelayAudioProcessor::getNumPrograms() { return 1; }
int FrequencyDelayAudioProcessor::getCurrentProgram() { return 0; }
void FrequencyDelayAudioProcessor::setCurrentProgram (int index) {}
const juce::String FrequencyDelayAudioProcessor::getProgramName (int index) { return "Default"; }
void FrequencyDelayAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void FrequencyDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filterStage.prepare(spec);

    delayLine.prepare (spec);
    delayLine.reset();

    panLfo.prepare(spec);
    panLfo.initialise([](float x) { return std::sin(x); });

    smoothedDelayTime.reset(sampleRate, 0.1);
    smoothedFeedback.reset(sampleRate, 0.02);
    smoothedMix.reset(sampleRate, 0.02);
    smoothedCutoff.reset(sampleRate, 0.02);
    smoothedWidth.reset(sampleRate, 0.02); 
}

void FrequencyDelayAudioProcessor::releaseResources() {}

bool FrequencyDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void FrequencyDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float targetCutoff = apvts.getRawParameterValue("CUTOFF")->load();
    int filterType = (int)apvts.getRawParameterValue("FILTERTYPE")->load();
    bool isInverted = apvts.getRawParameterValue("INVERT")->load() > 0.5f; 
    float targetMix = apvts.getRawParameterValue("MIX")->load();
    float targetFeedback = apvts.getRawParameterValue("FEEDBACK")->load();
    float targetWidth = apvts.getRawParameterValue("WIDTH")->load(); 
    
    float lfoRate = apvts.getRawParameterValue("LFORATE")->load();
    float lfoDepth = apvts.getRawParameterValue("LFODEPTH")->load();
    bool lfoSync = apvts.getRawParameterValue("LFOSYNC")->load() > 0.5f;
    int lfoSyncRateIndex = (int)apvts.getRawParameterValue("LFOSYNCRATE")->load();

    bool isSynced = apvts.getRawParameterValue("SYNC")->load() > 0.5f;
    int syncRateIndex = (int)apvts.getRawParameterValue("SYNCRATE")->load();
    float timeMs = apvts.getRawParameterValue("TIME")->load();

    float panRate = apvts.getRawParameterValue("PANRATE")->load();
    float panDepth = apvts.getRawParameterValue("PANDEPTH")->load();
    bool panSync = apvts.getRawParameterValue("PANSYNC")->load() > 0.5f;
    int panSyncRateIndex = (int)apvts.getRawParameterValue("PANSYNCRATE")->load();

    float targetDelayMs = timeMs;
    float currentPanFreqHz = panRate;
    float currentLfoFreqHz = lfoRate;

    if (isSynced || lfoSync || panSync) {
        if (auto* playHead = getPlayHead()) {
            if (auto pos = playHead->getPosition()) {
                if (pos->getBpm().hasValue() && *pos->getBpm() > 10.0f) {
                    lastValidBpm = *pos->getBpm(); 
                }
            }
        }

        float quarterNoteMs = 60000.0f / lastValidBpm;
        
        // Multipliers for: 1/2, 1/4, 1/8, 1/16, 1/32 (including Dotted and Triplet variants)
        const float timeMultipliers[] = {
            2.0f, 3.0f, 1.333333f,
            1.0f, 1.5f, 0.666667f,
            0.5f, 0.75f, 0.333333f,
            0.25f, 0.375f, 0.166667f,
            0.125f, 0.1875f, 0.083333f
        };
        
        // Inverse math because frequency is cycles-per-beat, while time is length-of-beat
        const float freqMultipliers[] = {
            0.5f, 0.333333f, 0.75f,
            1.0f, 0.666667f, 1.5f,
            2.0f, 1.333333f, 3.0f,
            4.0f, 2.666667f, 6.0f,
            8.0f, 5.333333f, 12.0f
        };

        if (isSynced) {
            targetDelayMs = quarterNoteMs * timeMultipliers[syncRateIndex];
        }

        if (lfoSync) {
            currentLfoFreqHz = (lastValidBpm / 60.0f) * freqMultipliers[lfoSyncRateIndex];
        }

        if (panSync) {
            currentPanFreqHz = (lastValidBpm / 60.0f) * freqMultipliers[panSyncRateIndex];
        }
    }

    panLfo.setFrequency(currentPanFreqHz);

    smoothedDelayTime.setTargetValue((targetDelayMs / 1000.0f) * getSampleRate());
    smoothedCutoff.setTargetValue(targetCutoff);
    smoothedMix.setTargetValue(targetMix);
    smoothedFeedback.setTargetValue(targetFeedback);
    smoothedWidth.setTargetValue(targetWidth); 

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        
        float currentCutoff = smoothedCutoff.getNextValue();
        float currentWidth = smoothedWidth.getNextValue(); 
        float currentMix = smoothedMix.getNextValue();
        float currentFeedback = smoothedFeedback.getNextValue();
        float currentDelayTimeSamples = smoothedDelayTime.getNextValue();

        delayLine.setDelay(currentDelayTimeSamples);
        filterStage.tickLFO(currentLfoFreqHz);

        float currentPan = panLfo.processSample(0.0f) * panDepth;
        float mappedPan = (currentPan + 1.0f) * 0.5f; 
        float leftPanGain = std::cos(mappedPan * juce::MathConstants<float>::halfPi);
        float rightPanGain = std::sin(mappedPan * juce::MathConstants<float>::halfPi);

        for (int ch = 0; ch < totalNumInputChannels; ++ch) {
            
            float input = buffer.getReadPointer(ch)[sample];
            float filtered = filterStage.processSample(ch, input, currentCutoff, currentWidth, filterType, isInverted, lfoDepth);
            float delayed = delayLine.popSample(ch);
            
            if (std::abs(delayed) < 1.0e-10f) delayed = 0.0f;
            
            delayLine.pushSample(ch, filtered + std::tanh(delayed * currentFeedback));

            float panGain = (ch == 0) ? leftPanGain : rightPanGain;
            float pannedDelayed = delayed * panGain;

            buffer.getWritePointer(ch)[sample] = (input * (1.0f - currentMix)) + (pannedDelayed * currentMix);
        }
    }
}

bool FrequencyDelayAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* FrequencyDelayAudioProcessor::createEditor() { return new FrequencyDelayAudioProcessorEditor (*this); }
void FrequencyDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void FrequencyDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessorValueTreeState::ParameterLayout FrequencyDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    FilterStage::addFilterParameters(layout);

    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"MIX", 1}, "Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    
    juce::NormalisableRange<float> tRange(1.0f, 8000.0f, 1.0f);
    tRange.setSkewForCentre(50.0f);
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"TIME", 1}, "Time", tRange, 500.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"FEEDBACK", 1}, "Feedback", juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.5f));
    
    // PAN PARAMS
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"PANRATE", 1}, "Pan Rate", juce::NormalisableRange<float>(0.1f, 20.0f, 0.01f, 0.5f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"PANDEPTH", 1}, "Pan Depth", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
    
    juce::StringArray syncOptions = {
        "1/2", "1/2 Dotted", "1/2 Triplet",
        "1/4", "1/4 Dotted", "1/4 Triplet",
        "1/8", "1/8 Dotted", "1/8 Triplet",
        "1/16", "1/16 Dotted", "1/16 Triplet",
        "1/32", "1/32 Dotted", "1/32 Triplet"
    };

    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"SYNC", 1}, "Delay Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"SYNCRATE", 1}, "Delay Sync Rate", syncOptions, 3)); 

    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"PANSYNC", 1}, "Pan Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"PANSYNCRATE", 1}, "Pan Sync Rate", syncOptions, 3));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new FrequencyDelayAudioProcessor(); }