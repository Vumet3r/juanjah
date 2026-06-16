#pragma once

#include <cmath>

class Distortion
{
public:
    float process(float input, float drive) const
    {
        return std::tanh(input * drive);
    }
};
