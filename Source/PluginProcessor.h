#pragma once
#include <JuceHeader.h>

class CCStudioAudioProcessor : public juce::AudioProcessor
{
public:
    CCStudioAudioProcessor();
    ~CCStudioAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 18.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::dsp::Reverb reverb;
    juce::AudioBuffer<float> predelayBuffer;
    int predelayWrite = 0;
    double currentSampleRate = 44100.0;
    float lowCutState[2] { 0.0f, 0.0f };
    float highCutState[2] { 0.0f, 0.0f };
    juce::Random random;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CCStudioAudioProcessor)
};
