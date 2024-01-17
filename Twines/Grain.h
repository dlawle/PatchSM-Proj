#pragma once
#ifndef GRAIN_H
#define GRAIN_H
#include <stdint.h>
#include "daisysp.h"
#ifdef __cplusplus

using namespace daisysp;

namespace graindelay
{
class Grain
{
    public:
        Grain() {}
        ~Grain() {}

        void Init(const float sampleRate, float* buffer, const size_t bufferSize)
        {
            sampleRate_ = sampleRate;
            buffer_ = buffer;
            bufferSize_ = bufferSize - kInterpolationTail;
            for (size_t i = 0; i < bufferSize; i++)
                buffer_[i] = 0;
            writeIndex_ = 0;
            readPosition_ = 0;

            feedbackSvf_.Init(sampleRate);
            env_.Init(sampleRate);

            audible_ = 0.0f;
            speed_ = 1.0f;
            amp_ = 0.0f;
            grainDensity_ = 0.0f;
            feedback_ = 0.0f;
        }

        void SetSpeed(const float speed);

        void SetAmp(const float amp)
        {
            amp_ = fclamp(amp, 0.0f, 1.0f);
        }

        inline void SetFeedback(const float feedback)
        {
            feedback_ = fclamp(feedback, 0.0f, 1.0f);
            feedbackSvf_.SetFreq(20.0f + 100.0f * feedback_ * feedback_);   // Formula from https://github.com/pichenettes/eurorack/blob/master/clouds/dsp/granular_processor.cc
        }

        inline void Trigger()
        {
            readPosition_ = speed_ > sampleRate_ ? ((writeIndex_ - grainSize_) + bufferSize_) % bufferSize_ : writeIndex_;
            env_.Trigger();
        }

        inline void SetDuration(const float length)
        {
            nextDuration_ = length;
        }

        inline void SetGrainDensity(const float grainDensity)
        {
            grainDensity_ = fclamp(grainDensity, 0.0f, 1.0f);
        }

        inline float GetPan() {return pan_;}

        float Process(const float in);

    private:
        float* buffer_;

        size_t  bufferSize_,
                writeIndex_,
                grainMaxDuration_,
                grainSize_;

        float   amp_,
                pan_,
                feedback_,
                nextDuration_,
                grainDensity_,
                sampleRate_,
                speed_,
                readPosition_,
                audible_;

        AdEnv env_;
        Svf feedbackSvf_;

        static const size_t kGrainMinDuration;
        static const float kPanMaxWidth;
        static const size_t kInterpolationTail;
        static const float kSizeVariationAmount;

        inline void updateReadPosition()
        {
            // TODO (Later optimization) readPosition could be split in two size_t vars
            // one to track the integral value of the other one to track the fractional value * 65536 (similar to what
            // is in clouds code)
            readPosition_ += speed_;
            if (readPosition_ < 0) readPosition_ += bufferSize_;
            else if (readPosition_ > bufferSize_) readPosition_ -= bufferSize_;
        }

        inline void UpdatePan()
        {
            pan_ = fmap(rand() * kRandFrac, -1 * kPanMaxWidth, kPanMaxWidth);
        }

        float read(size_t position);
        float read(float position);
        float readHermite(float position);
        void updateGrainSize();
};
}

#endif
#endif
