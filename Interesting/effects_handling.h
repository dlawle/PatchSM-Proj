// reverb implementation from https://github.com/erwincoumans/DaisyCloudSeed

#pragma once
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "rings/dsp/fx/reverb.h"
#include "clouds/dsp/granular_processor.h"
#include "clouds/dsp/frame.h"

using namespace rings;
using namespace clouds;

class EffectsHandling
{
public:
    void UpdateReverbParams(float amount, float diffusion, float time, float input_gain)
    {
        clouds_reverb.set_amount(amount);
        clouds_reverb.set_diffusion(diffusion);
        clouds_reverb.set_time(time);// 0.5f + (0.49f * patch_position));
        clouds_reverb.set_input_gain(input_gain);
        clouds_reverb.set_lp(0.3f);// : 0.6f);
    }

    void Init(float sample_rate)
    {
        clouds_reverb.Init(reverb_buffer);
        enabled_ = false;
    }

    void Process(float* left, float* right, size_t size)
    {
        if(enabled_ == true) { clouds_reverb.Process(left, right, size); }
    }

    void FlipEffects() { enabled_ = !enabled_; }

    bool returnState() { return enabled_; }


private:
    rings::Reverb clouds_reverb;
    uint16_t reverb_buffer[65536];
    bool enabled_;
};
