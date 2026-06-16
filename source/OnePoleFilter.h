#pragma once

#include <JuceHeader.h>
#include <cmath>

class OnePoleFilter
{
public:
    void setCutoff(float freq, float sampleRate)
    {
        if (freq < 20.0f) freq = 20.0f;
        auto maxF = sampleRate * 0.45f;
        if (freq > maxF) freq = maxF;
        auto rc = 1.0f / (2.0f * juce::MathConstants<float>::pi * freq);
        auto dt = 1.0f / static_cast<float>(sampleRate);
        alpha = dt / (rc + dt);
    }

    void reset() { y1 = 0.0f; }

    float process(float x)
    {
        y1 += alpha * (x - y1);
        return y1;
    }

private:
    float y1 = 0.0f;
    float alpha = 0.5f;
};
