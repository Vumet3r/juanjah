#pragma once

#include <JuceHeader.h>

class LFO
{
public:
    void prepare(double sr)
    {
        sampleRate = sr;
        phase = 0.0f;
    }

    void setFrequency(float freqHz)
    {
        increment = freqHz / static_cast<float>(sampleRate);
    }

    float process()
    {
        auto value = std::sin(2.0f * juce::MathConstants<float>::pi * phase);
        phase += increment;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    void reset()
    {
        phase = 0.0f;
    }

private:
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float increment = 0.0f;
};
