#pragma once 

#include "daisy.h"
#include "daisysp.h"
#include "Utility/dsp.h"

using namespace daisy;
using namespace daisysp;

class GrainTest
{
public:

void Init(float samplerate, float* buffer, size_t bufferSize)
{
    sample_rate_ = samplerate;
    buffer_ = buffer;
    buffer_size_ = bufferSize;
    write_index_ = 0;
    read_position_ = 0;
    for (size_t i = 0; i < bufferSize; i++)
    {
        buffer_[i] = 0;
    }
}

void Process(const float in)
{
    buffer_[write_index_] = in;
    write_index_ = (++write_index_) % buffer_size_;
}

// Hermite interpolation (from here https://github.com/pichenettes/stmlib/blob/master/dsp/dsp.h)
inline float readHermite(float position)
{
    const int32_t t = static_cast<int32_t>(position);
    const float f = position - static_cast<float>(t);

    const float xm1 = buffer_[(t - 1) % buffer_size_]; // this line causes issue as a negative index doesn't wrap to the end of the buffer
    const float x0 = buffer_[(t) % buffer_size_];
    const float x1 = buffer_[(t + 1) % buffer_size_];
    const float x2 = buffer_[(t + 2) % buffer_size_];

    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;

    return (((a * f) - b_neg) * f + c) * f + x0;
}

inline void updateReadPosition()
{
    // TODO (Later optimization) readPosition could be split in two size_t vars
    // one to track the integral value of the other one to track the fractional value * 65536 (similar to what
    // is in clouds code)
    read_position_ += speed_;
    if (read_position_ < 0) read_position_ += buffer_size_;
    else if (read_position_ > buffer_size_) read_position_ -= buffer_size_;
}

size_t GetIndex()
{
    return write_index_;
}

private:
    float* buffer_;

    size_t  buffer_size_, 
            write_index_;
    
    float   sample_rate_,
            speed_,
            read_position_;

};
