#pragma once
#include "PluginProcessor.h"

class CCStudioAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit CCStudioAudioProcessorEditor (CCStudioAudioProcessor&);
    ~CCStudioAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct GlassLookAndFeel : juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    } look;
    CCStudioAudioProcessor& processor;
    juce::Image background;
    juce::ComboBox mode;
    juce::Label title, subtitle;
    juce::Slider mix, decay, predelay, size, width, damping, lowcut, highcut, modulation, output;
    juce::Label labels[10];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachments[10];
    juce::Slider* sliders[10] { &mix, &decay, &predelay, &size, &width, &damping, &lowcut, &highcut, &modulation, &output };
    const char* parameterIds[10] { "MIX", "DECAY", "PREDELAY", "SIZE", "WIDTH", "DAMP", "LOWCUT", "HIGHCUT", "MOD", "OUTPUT" };
    const char* parameterNames[10] { "MIX", "DECAY", "PRE-DELAY", "SIZE", "WIDTH", "DAMPING", "LOW CUT", "HIGH CUT", "MOD", "OUTPUT" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CCStudioAudioProcessorEditor)
};
