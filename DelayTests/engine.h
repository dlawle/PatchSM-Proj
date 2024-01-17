#pragma once
#ifndef Engine_H_
#define Engine_H_

#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "fx/delay.h"
#include "fx/Grain.h"
#include <cmath>

using namespace daisysp;
#define MAX_DELAY static_cast<size_t>(48000)

namespace daisy
{
    class Engine
    {
    public:
        Engine();
        ~Engine();

        void Init(float sample_rate);
        void SetParam(float pot1, float pot2, float pot3, float pot4, float pot5, float pot6, float pot7, float pot8);
        void Write(float in);
        void Feed();
        float Process(float in);
        void UpdateProcesor(float type);
        float GetProcessor();
        void GetActivate(bool signal);

    private:
        DelayLineReverse<float, MAX_DELAY> del;
        DelayLineReverse<float, MAX_DELAY / 2> del_second;
        int effect_suite;
        bool activate;
        float signal;
        float samp_rate;

        // pot vars
        float delay_time1;
        float delay_time2;
        float feedbackamt;
        float feedbackamt2;
        float feedback;
        float feedback2;
        float reverb_time;
        float reverb_filter;
        float g_amp_amt;
        float g_dur_amt;
        float g_fb_amt;
        float g_dens_amt;
        float g_speed_amt;
        float smooth_time1, smooth_time2;
        // outputs 
        float del_out;
        float second_phase_out;
        float output;
    };

} // namespace daisy

#endif // Engine_H_
