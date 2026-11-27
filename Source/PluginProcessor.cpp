#include "PluginProcessor.h"
#include "PluginEditor.h"

CCStudioAudioProcessor::CCStudioAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "CCSTUDIO", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CCStudioAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    p.push_back (std::make_unique<juce::AudioParameterChoice> ("MODE", "Reverb Type",
        juce::StringArray { "Dream", "Vintage", "Dark Hall", "Crystal", "Ethereal" }, 0));
    p.push_back (std::make_unique<APF> ("MIX", "Mix", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 28.0f));
    p.push_back (std::make_unique<APF> ("DECAY", "Decay", juce::NormalisableRange<float> (0.3f, 18.0f, 0.01f, 0.4f), 5.5f));
    p.push_back (std::make_unique<APF> ("PREDELAY", "Pre-Delay", juce::NormalisableRange<float> (0.0f, 250.0f, 0.1f), 35.0f));
    p.push_back (std::make_unique<APF> ("SIZE", "Size", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 65.0f));
    p.push_back (std::make_unique<APF> ("WIDTH", "Width", juce::NormalisableRange<float> (0.0f, 150.0f, 0.1f), 115.0f));
    p.push_back (std::make_unique<APF> ("DAMP", "Damping", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 48.0f));
    p.push_back (std::make_unique<APF> ("LOWCUT", "Low Cut", juce::NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.35f), 140.0f));
    p.push_back (std::make_unique<APF> ("HIGHCUT", "High Cut", juce::NormalisableRange<float> (2000.0f, 20000.0f, 1.0f, 0.35f), 12500.0f));
    p.push_back (std::make_unique<APF> ("MOD", "Modulation", juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 22.0f));
    p.push_back (std::make_unique<APF> ("OUTPUT", "Output", juce::NormalisableRange<float> (-18.0f, 6.0f, 0.1f), 0.0f));
    return { p.begin(), p.end() };
}

void CCStudioAudioProcessor::prepareToPlay (double sr, int)
{
    currentSampleRate = sr;
    predelayBuffer.setSize (2, (int) std::ceil (sr * 0.27) + 2);
    predelayBuffer.clear();
    predelayWrite = 0;
    reverb.reset();
    lowCutState[0] = lowCutState[1] = highCutState[0] = highCutState[1] = 0.0f;
}

void CCStudioAudioProcessor::releaseResources() { reverb.reset(); }

bool CCStudioAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();

    // Logic Pro offers different insert menus for mono and stereo tracks.
    // Advertise both layouts, while keeping input and output channel counts matched.
    if (input != output)
        return false;

    return input == juce::AudioChannelSet::mono()
        || input == juce::AudioChannelSet::stereo();
}

void CCStudioAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numChannels < 1 || numChannels > 2) return;

    const int mode = (int) parameters.getRawParameterValue ("MODE")->load();
    const float mix = parameters.getRawParameterValue ("MIX")->load() * 0.01f;
    const float decay = parameters.getRawParameterValue ("DECAY")->load();
    const float size = parameters.getRawParameterValue ("SIZE")->load() * 0.01f;
    const float width = parameters.getRawParameterValue ("WIDTH")->load() * 0.01f;
    const float damp = parameters.getRawParameterValue ("DAMP")->load() * 0.01f;
    const float preMs = parameters.getRawParameterValue ("PREDELAY")->load();
    const float mod = parameters.getRawParameterValue ("MOD")->load() * 0.01f;
    const float outGain = juce::Decibels::decibelsToGain (parameters.getRawParameterValue ("OUTPUT")->load());
    const float lowHz = parameters.getRawParameterValue ("LOWCUT")->load();
    const float highHz = parameters.getRawParameterValue ("HIGHCUT")->load();

    const float modeDecay[] = { 1.0f, 0.82f, 1.28f, 0.72f, 1.55f };
    const float modeDamp[]  = { 0.46f, 0.68f, 0.82f, 0.24f, 0.38f };
    const float modeSize[]  = { 0.76f, 0.62f, 1.0f, 0.55f, 1.0f };
    const float modeWidth[] = { 1.2f, 1.0f, 1.35f, 1.15f, 1.45f };

    juce::AudioBuffer<float> wet (numChannels, n);
    wet.clear();
    const int delaySamples = juce::jlimit (0, predelayBuffer.getNumSamples() - 2,
        (int) std::round (preMs * 0.001 * currentSampleRate));

    for (int ch = 0; ch < numChannels; ++ch)
    {
        const float* in = buffer.getReadPointer (ch);
        float* delayed = wet.getWritePointer (ch);
        for (int i = 0; i < n; ++i)
        {
            const int read = (predelayWrite + i - delaySamples + predelayBuffer.getNumSamples()) % predelayBuffer.getNumSamples();
            delayed[i] = delaySamples == 0 ? in[i] : predelayBuffer.getReadPointer (ch)[read];
            predelayBuffer.setSample (ch, (predelayWrite + i) % predelayBuffer.getNumSamples(), in[i]);
        }
    }
    predelayWrite = (predelayWrite + n) % predelayBuffer.getNumSamples();

    juce::Reverb::Parameters rp;
    rp.roomSize = juce::jlimit (0.05f, 0.98f, (0.12f + 0.58f * (decay / 18.0f) + 0.30f * size) * modeSize[mode] * modeDecay[mode]);
    rp.damping = juce::jlimit (0.05f, 0.98f, damp * 0.72f + modeDamp[mode] * 0.28f);
    rp.wetLevel = 1.0f;
    rp.dryLevel = 0.0f;
    rp.width = juce::jlimit (0.0f, 1.0f, width * modeWidth[mode] / 1.5f);
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);

    // The five voicings shape decay, room size and damping independently.
    if (numChannels == 2)
        reverb.processStereo (wet.getWritePointer (0), wet.getWritePointer (1), n);
    else
        reverb.processMono (wet.getWritePointer (0), n);

    const float hpA = std::exp (-juce::MathConstants<float>::twoPi * lowHz / (float) currentSampleRate);
    const float lpA = std::exp (-juce::MathConstants<float>::twoPi * highHz / (float) currentSampleRate);
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* w = wet.getWritePointer (ch);
        float* dry = buffer.getWritePointer (ch);
        float lowState = lowCutState[ch], highState = highCutState[ch];
        for (int i = 0; i < n; ++i)
        {
            const float x = w[i];
            lowState += (1.0f - hpA) * (x - lowState);
            const float highPassed = x - lowState;
            highState += (1.0f - lpA) * (highPassed - highState);
            // A tiny, decorrelated modulation component adds movement without chorus-like pitch wobble.
            const float movement = 1.0f + 0.012f * mod * std::sin (juce::MathConstants<float>::twoPi
                * (float) (i + predelayWrite) / (float) currentSampleRate * (0.18f + 0.04f * mode));
            const float wetSample = highState * movement;
            dry[i] = (dry[i] * (1.0f - mix) + wetSample * mix) * outGain;
        }
        lowCutState[ch] = lowState;
        highCutState[ch] = highState;
    }
}

juce::AudioProcessorEditor* CCStudioAudioProcessor::createEditor() { return new CCStudioAudioProcessorEditor (*this); }

void CCStudioAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void CCStudioAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CCStudioAudioProcessor(); }
