#include "daisysp.h"
#include "baby_osc.h"
#include "Utility/dsp.h"

using namespace daisysp;
static inline float Polyblep(float phase_inc, float t);

constexpr float TWO_PI_RECIP = 1.0f / TWOPI_F;

float BabyOscillator::Process()
{

        float out;
        /**
         * basic test, leaving phase out of the calculation of the harmonics
        */
        float x = phase_;

        /**-----------------------------------------------------------------------
        *  Weird dancing sin thing 
        *-----------------------------------------------------------------------**/
        float wave = sin(mod_ * x) * (4 * abs(x - floor(x + 3/4) + 1/4) - 1);
        out = soft_saturate(wave, 0.5);
        //out = 2*(sin(sin(mod_*PI_F*x)+mod_))/PI_F; // this needs tested but may result in some weird waveforms
        
        phase_ += phase_inc_;
        if(phase_ > TWOPI_F)
        {
            phase_ -= TWOPI_F;
            eoc_ = true;
        }
        else
        {
            eoc_ = false;
        }
        eor_ = (phase_ - phase_inc_ < PI_F && phase_ >= PI_F);

        return out * amp_;

}

float BabyOscillator::CalcPhaseInc(float f)
{
    return (TWOPI_F * f) * sr_recip_;
}

static float Polyblep(float phase_inc, float t)
{
    float dt = phase_inc * TWO_PI_RECIP;
    if(t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    else if(t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    else
    {
        return 0.0f;
    }
}
