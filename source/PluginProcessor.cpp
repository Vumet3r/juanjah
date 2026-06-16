#include "PluginProcessor.h"
#include "PluginEditor.h"

static float noteDivToBeat(int index)
{
    switch (index)
    {
        case 0: return 1.0f / 4.0f;
        case 1: return 1.0f / 6.0f;
        case 2: return 1.0f / 8.0f;
        case 3: return 3.0f / 16.0f;
        case 4: return 1.0f / 12.0f;
        case 5: return 1.0f / 16.0f;
        case 6: return 3.0f / 32.0f;
        case 7: return 1.0f / 32.0f;
        default: return 1.0f / 4.0f;
    }
}

VSTPluginAudioProcessor::VSTPluginAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo())
                                     .withOutput("Output", juce::AudioChannelSet::stereo()))
{
    auto addParam = [&](juce::RangedAudioParameter* p) { addParameter(p); };
    auto floatRange = juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f);
    auto timeRange  = juce::NormalisableRange<float>(10.0f, 2000.0f, 0.1f, 0.4f);
    auto fbRange    = juce::NormalisableRange<float>(0.0f, 0.95f, 0.001f);
    auto freqRange  = juce::NormalisableRange<float>(0.0f, 10.0f, 0.01f);
    auto bpmRange   = juce::NormalisableRange<float>(40.0f, 200.0f, 0.1f);
    auto driveRange = juce::NormalisableRange<float>(1.0f, 15.0f, 0.01f, 0.4f);

    dryWetParam = new juce::AudioParameterFloat("dryWet", "Mix", floatRange, 0.3f);
    addParam(dryWetParam);

    for (int i = 0; i < 3; ++i)
    {
        auto p = juce::String(i + 1);
        tap[i].bpm      = new juce::AudioParameterFloat("bpm" + p, "BPM" + p, bpmRange, 120.0f);
        tap[i].timeMs   = new juce::AudioParameterFloat("t" + p + "Ms", "Tap" + p + " ms", timeRange, 100.0f + i * 150.0f);
        tap[i].level    = new juce::AudioParameterFloat("t" + p + "Lv", "Tap" + p + " Level", floatRange, 0.5f - i * 0.15f);
        tap[i].feedback = new juce::AudioParameterFloat("t" + p + "Fb", "Tap" + p + " Fb", fbRange, 0.2f);
        tap[i].filter   = new juce::AudioParameterFloat("t" + p + "Ft", "Tap" + p + " Filter", floatRange, 0.7f);
        tap[i].noteDiv  = new juce::AudioParameterChoice("t" + p + "Nd", "Tap" + p + " Note", juce::StringArray(noteDivChoices, 8), i == 0 ? 2 : i + 2);
        tap[i].sync     = new juce::AudioParameterBool("t" + p + "Sy", "Tap" + p + " Sync", true);
        addParam(tap[i].bpm);
        addParam(tap[i].timeMs);
        addParam(tap[i].level);
        addParam(tap[i].feedback);
        addParam(tap[i].filter);
        addParam(tap[i].noteDiv);
        addParam(tap[i].sync);
    }

    modRateParam   = new juce::AudioParameterFloat("modRate",   "Rate",   freqRange, 0.5f);
    modDepthParam  = new juce::AudioParameterFloat("modDepth",  "Depth",  floatRange, 0.15f);
    distDriveParam = new juce::AudioParameterFloat("distDrv",   "Drive",  driveRange, 3.0f);
    distBlendParam = new juce::AudioParameterFloat("distBlnd",  "Blend",  floatRange, 0.0f);
    addParam(modRateParam);
    addParam(modDepthParam);
    addParam(distDriveParam);
    addParam(distBlendParam);
}

VSTPluginAudioProcessor::~VSTPluginAudioProcessor() = default;

void VSTPluginAudioProcessor::tapTempo(int idx)
{
    if (idx < 0 || idx > 2) return;
    auto now = juce::Time::getMillisecondCounter();
    if (lastTapTime[idx] == 0)
    {
        lastTapTime[idx] = now;
        tapCount[idx] = 0;
        return;
    }
    auto delta = static_cast<float>(now - lastTapTime[idx]);
    lastTapTime[idx] = now;
    if (delta < 100.0f || delta > 3000.0f)
    {
        tapCount[idx] = 0;
        return;
    }
    tapHistory[idx][tapCount[idx] % 5] = delta;
    tapCount[idx]++;
    if (tapCount[idx] >= 2)
    {
        auto n = juce::jmin(tapCount[idx], 5);
        float avg = 0.0f;
        for (int i = 0; i < n; ++i)
            avg += tapHistory[idx][i];
        avg /= static_cast<float>(n);
        *tap[idx].bpm = juce::jlimit(40.0f, 200.0f, 60000.0f / avg);
    }
}

void VSTPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    auto maxDelaySamples = static_cast<int>(sampleRate * 2.5);
    delayLineL.prepare(sampleRate, maxDelaySamples);
    delayLineR.prepare(sampleRate, maxDelaySamples);
    lfo.prepare(sampleRate);
    for (int i = 0; i < 3; ++i)
    {
        filterL[i].reset();
        filterR[i].reset();
    }
}

void VSTPluginAudioProcessor::releaseResources()
{
    delayLineL.reset();
    delayLineR.reset();
    lfo.reset();
    for (int i = 0; i < 3; ++i)
    {
        filterL[i].reset();
        filterR[i].reset();
    }
}

void VSTPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    auto dryWet = dryWetParam->get();
    auto modRate = modRateParam->get();
    auto modDepth = modDepthParam->get();
    auto distDrive = distDriveParam->get();
    auto distBlend = distBlendParam->get();

    float tapTimeSec[3], tapLevel[3], tapFb[3], tapCutoff[3];
    for (int i = 0; i < 3; ++i)
    {
        bool synced = tap[i].sync->get();
        auto bpm = tap[i].bpm->get();
        if (synced)
        {
            auto beatMs = 60000.0f / bpm;
            auto noteBeats = noteDivToBeat(tap[i].noteDiv->getIndex());
            tapTimeSec[i] = (beatMs / 1000.0f) * noteBeats * 4.0f;
            tap[i].currentTimeSec = tapTimeSec[i];
            tap[i].currentBpm = bpm;
        }
        else
        {
            tapTimeSec[i] = tap[i].timeMs->get() / 1000.0f;
            tap[i].currentTimeSec = tapTimeSec[i];
        }
        tapLevel[i] = tap[i].level->get();
        tapFb[i]    = tap[i].feedback->get();
        auto f = tap[i].filter->get();
        tapCutoff[i] = 200.0f * std::pow(40.0f, f);
        filterL[i].setCutoff(tapCutoff[i], static_cast<float>(currentSampleRate));
        filterR[i].setCutoff(tapCutoff[i], static_cast<float>(currentSampleRate));
    }

    lfo.setFrequency(modRate);

    auto* channelDataL = buffer.getWritePointer(0);
    auto* channelDataR = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : channelDataL;

    for (int s = 0; s < numSamples; ++s)
    {
        auto inputL = channelDataL[s];
        auto inputR = channelDataR[s];
        auto mod = modDepth > 0.0f ? 1.0f + lfo.process() * modDepth : 1.0f;

        float wetL = 0.0f, wetR = 0.0f;
        float fbL = 0.0f, fbR = 0.0f;

        for (int t = 0; t < 3; ++t)
        {
            auto delaySamples = static_cast<float>(tapTimeSec[t] * currentSampleRate * mod);
            if (delaySamples < 1.0f) delaySamples = 1.0f;
            auto tapOutL = delayLineL.getSample(delaySamples) * tapLevel[t];
            auto tapOutR = delayLineR.getSample(delaySamples) * tapLevel[t];

            tapOutL = filterL[t].process(tapOutL);
            tapOutR = filterR[t].process(tapOutR);

            wetL += tapOutL;
            wetR += tapOutR;
            fbL += tapOutL * tapFb[t];
            fbR += tapOutR * tapFb[t];
        }

        delayLineL.pushSample(inputL + fbL);
        if (numChannels > 1)
            delayLineR.pushSample(inputR + fbR);
        else
            delayLineR.pushSample(inputL + fbL);

        auto distL = distortion.process(wetL, distDrive);
        auto distR = distortion.process(wetR, distDrive);
        auto delayedL = wetL * (1.0f - distBlend) + distL * distBlend;
        auto delayedR = wetR * (1.0f - distBlend) + distR * distBlend;

        channelDataL[s] = inputL * (1.0f - dryWet) + delayedL * dryWet;
        channelDataR[s] = inputR * (1.0f - dryWet) + delayedR * dryWet;
    }
}

juce::AudioProcessorEditor* VSTPluginAudioProcessor::createEditor()
{
    return new VSTPluginAudioProcessorEditor(*this);
}

bool VSTPluginAudioProcessor::hasEditor() const { return true; }
const juce::String VSTPluginAudioProcessor::getName() const { return "Juanjah"; }
bool VSTPluginAudioProcessor::acceptsMidi() const { return false; }
bool VSTPluginAudioProcessor::producesMidi() const { return false; }
bool VSTPluginAudioProcessor::isMidiEffect() const { return false; }
double VSTPluginAudioProcessor::getTailLengthSeconds() const { return 2.0; }

int VSTPluginAudioProcessor::getNumPrograms() { return 1; }
int VSTPluginAudioProcessor::getCurrentProgram() { return 0; }
void VSTPluginAudioProcessor::setCurrentProgram(int) {}
const juce::String VSTPluginAudioProcessor::getProgramName(int) { return {}; }
void VSTPluginAudioProcessor::changeProgramName(int, const juce::String&) {}

void VSTPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    for (auto* p : getParameters())
    {
        if (auto* fp = dynamic_cast<juce::AudioParameterFloat*>(p))
        { auto val = fp->get(); destData.append(&val, sizeof(val)); }
        else if (auto* bp = dynamic_cast<juce::AudioParameterBool*>(p))
        { auto val = bp->get() ? 1.0f : 0.0f; destData.append(&val, sizeof(val)); }
        else if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p))
        { auto val = static_cast<float>(cp->getIndex()); destData.append(&val, sizeof(val)); }
    }
}

void VSTPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto* bytes = static_cast<const float*>(data);
    auto count = sizeInBytes / static_cast<int>(sizeof(float));
    int idx = 0;
    for (auto* p : getParameters())
    {
        if (idx >= count) break;
        if (auto* fp = dynamic_cast<juce::AudioParameterFloat*>(p))
            *fp = bytes[idx++];
        else if (auto* bp = dynamic_cast<juce::AudioParameterBool*>(p))
            *bp = bytes[idx++] > 0.5f;
        else if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(p))
        {
            auto ci = juce::jlimit(0, cp->choices.size() - 1, juce::roundToInt(bytes[idx++]));
            *cp = static_cast<float>(ci) / static_cast<float>(cp->choices.size() - 1);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VSTPluginAudioProcessor();
}
