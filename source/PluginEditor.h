#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CustomKnob : public juce::Component
{
public:
    CustomKnob(juce::RangedAudioParameter& param, const juce::String& label,
               juce::Colour accent);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void updateValue();

private:
    juce::RangedAudioParameter& param;
    juce::String label;
    juce::Colour accent;
    float normalised = 0.0f;
    float dragStartY = 0;
    float dragStartNorm = 0;
    bool isDragging = false;

    float getNormalised() const;
    void setNormalised(float v);
    juce::String getDisplayText() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomKnob)
};

class SyncButton : public juce::Button
{
public:
    SyncButton(juce::AudioParameterBool& param);
    void paintButton(juce::Graphics&, bool, bool) override;
    void clicked() override;

private:
    juce::AudioParameterBool& param;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncButton)
};

class NoteSelector : public juce::ComboBox
{
public:
    NoteSelector(juce::AudioParameterChoice& param);
    ~NoteSelector() override;

private:
    juce::AudioParameterChoice& param;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteSelector)
};

class TapTempoButton : public juce::Component
{
public:
    TapTempoButton(int tapIndex, VSTPluginAudioProcessor& proc);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void updateBpm();

private:
    int index;
    VSTPluginAudioProcessor& processor;
    float flashAlpha = 0.0f;
    juce::uint32 lastFlash = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapTempoButton)
};

class VSTPluginAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VSTPluginAudioProcessorEditor(VSTPluginAudioProcessor&);
    ~VSTPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    VSTPluginAudioProcessor& processorRef;
    juce::Image background;

    struct TapUI {
        std::unique_ptr<TapTempoButton> tapBtn;
        std::unique_ptr<SyncButton> syncBtn;
        std::unique_ptr<NoteSelector> noteCombo;
        std::unique_ptr<CustomKnob> timeKnob;
        std::unique_ptr<CustomKnob> fbKnob;
        std::unique_ptr<CustomKnob> filterKnob;
    };
    TapUI tapUI[3];

    std::unique_ptr<CustomKnob> dryWetKnob;
    std::unique_ptr<CustomKnob> modRateKnob;
    std::unique_ptr<CustomKnob> modDepthKnob;
    std::unique_ptr<CustomKnob> distDriveKnob;
    std::unique_ptr<CustomKnob> distBlendKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VSTPluginAudioProcessorEditor)
};
