#pragma once

#include <JuceHeader.h>
#include <vector>

class DelayLine
{
public:
    void prepare(double sampleRate, int maxDelaySamples)
    {
        buffer.assign(maxDelaySamples + 1, 0.0f);
        writeIndex = 0;
        this->sampleRate = sampleRate;
        this->maxDelaySamples = maxDelaySamples;
    }

    void pushSample(float sample)
    {
        buffer[writeIndex] = sample;
        writeIndex = (writeIndex + 1) % static_cast<int>(buffer.size());
    }

    float getSample(float delaySamples) const
    {
        auto readIndex = static_cast<float>(writeIndex) - delaySamples;
        if (readIndex < 0.0f)
            readIndex += static_cast<float>(buffer.size());

        auto intIndex = static_cast<int>(readIndex);
        auto frac = readIndex - static_cast<float>(intIndex);
        auto nextIndex = (intIndex + 1) % static_cast<int>(buffer.size());

        auto s0 = buffer[intIndex];
        auto s1 = buffer[nextIndex];

        return s0 + frac * (s1 - s0);
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

private:
    std::vector<float> buffer;
    int writeIndex = 0;
    double sampleRate = 44100.0;
    int maxDelaySamples = 0;
};
