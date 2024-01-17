#include "Grain.h"

using namespace graindelay;

const size_t Grain::kGrainMinDuration = 4000;   // In samples
const float Grain::kPanMaxWidth = 1.0f;
const size_t Grain::kInterpolationTail = 8;
const float Grain::kSizeVariationAmount = 0.05f;

float Grain::Process(const float in)
{
    buffer_[writeIndex_] = in;

    if (!env_.IsRunning())
    {
        audible_ = (rand() * kRandFrac) >= grainDensity_ ? 0.0f : 1.0f;
        updateGrainSize();
        UpdatePan();
        Trigger();
    }

    const float out = readHermite(readPosition_);
    const float level = env_.Process() * amp_ * audible_;

    // Apply feedback, with high-pass filtering to prevent build-ups at very
    // low frequencies (causing large DC swings).
    feedbackSvf_.Process(out);

    // TODO Try out with formula from https://github.com/pichenettes/eurorack/blob/master/clouds/dsp/granular_processor.cc
    buffer_[writeIndex_] += feedbackSvf_.High() * level * feedback_;

    if (writeIndex_ < kInterpolationTail)
        buffer_[writeIndex_ + bufferSize_] = buffer_[writeIndex_];
    writeIndex_ += (++writeIndex_) % bufferSize_;

    updateReadPosition();

    return out * level;
}

void Grain::SetSpeed(const float speed)
{
    speed_ = speed;
    float sizeRatio = 1.0f;
    if (speed < 0.0f || speed > 1.0f)
        sizeRatio = 1.0f / fabs(speed - 1.0f);
    grainMaxDuration_ = static_cast<size_t>(sizeRatio * bufferSize_);
}

inline float Grain::read(size_t position)
{
    return buffer_[position % bufferSize_];
}

// Linear interpolation
inline float Grain::read(float position)
{
    const int32_t t = static_cast<int32_t>(position);
    const float f = position - static_cast<float>(t);

    const float a = buffer_[t % bufferSize_];
    const float b = buffer_[(t+1) % bufferSize_];

    return a + (b-a) * f;
}

// Hermite interpolation (from here https://github.com/pichenettes/stmlib/blob/master/dsp/dsp.h)
inline float Grain::readHermite(float position)
{
    const int32_t t = static_cast<int32_t>(position);
    const float f = position - static_cast<float>(t);

    const float xm1 = buffer_[(t - 1) % bufferSize_]; // this line causes issue as a negative index doesn't wrap to the end of the buffer
    const float x0 = buffer_[(t) % bufferSize_];
    const float x1 = buffer_[(t + 1) % bufferSize_];
    const float x2 = buffer_[(t + 2) % bufferSize_];

    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;

    return (((a * f) - b_neg) * f + c) * f + x0;
}

void Grain::updateGrainSize()
{
    const size_t scaledDialedDuration = static_cast<size_t>(kGrainMinDuration + nextDuration_ * (bufferSize_ - kGrainMinDuration));
    const size_t variation = static_cast<size_t>((rand() * kRandFrac * 2 - 1) * scaledDialedDuration * kSizeVariationAmount);

    grainSize_ = std::min(std::max(scaledDialedDuration + variation, kGrainMinDuration), grainMaxDuration_);

    env_.SetTime(ADENV_SEG_ATTACK, grainSize_ * 0.5f / sampleRate_);
    env_.SetTime(ADENV_SEG_DECAY, grainSize_ * 0.5f / sampleRate_);
}
