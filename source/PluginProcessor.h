#pragma once

#include <JuceHeader.h>
#include "DelayLine.h"
#include "LFO.h"
#include "Distortion.h"
#include "OnePoleFilter.h"

struct TapParams
{
    juce::AudioParameterFloat* timeMs{nullptr};
    juce::AudioParameterFloat* level{nullptr};
    juce::AudioParameterFloat* feedback{nullptr};
    juce::AudioParameterFloat* filter{nullptr};
    juce::AudioParameterChoice* noteDiv{nullptr};
    juce::AudioParameterBool* sync{nullptr};
    juce::AudioParameterFloat* bpm{nullptr};
    float currentTimeSec = 0.1f;
    float currentBpm = 120.0f;
};

class VSTPluginAudioProcessor : public juce::AudioProcessor
{
public:
    VSTPluginAudioProcessor();
    ~VSTPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void tapTempo(int tapIndex);

    juce::AudioParameterFloat* dryWetParam{nullptr};
    TapParams tap[3];
    juce::AudioParameterFloat* modRateParam{nullptr};
    juce::AudioParameterFloat* modDepthParam{nullptr};
    juce::AudioParameterFloat* distDriveParam{nullptr};
    juce::AudioParameterFloat* distBlendParam{nullptr};

    static constexpr const char* noteDivChoices[] = { "1/4", "1/4T", "1/8", "1/8D", "1/8T", "1/16", "1/16D", "1/32" };

private:
    DelayLine delayLineL;
    DelayLine delayLineR;
    LFO lfo;
    Distortion distortion;
    OnePoleFilter filterL[3];
    OnePoleFilter filterR[3];
    double currentSampleRate = 44100.0;
    juce::uint32 lastTapTime[3] = {};
    float tapHistory[3][5] = {};
    int tapCount[3] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VSTPluginAudioProcessor)
};
