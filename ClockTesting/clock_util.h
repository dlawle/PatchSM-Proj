#pragma once
#ifndef CLOCK_UTIL_H
#define CLOCK_UTIL_H

#include "daisysp.h"
#include "daisy.h"

using namespace daisy;
using namespace daisysp;

extern daisy::patch_sm::DaisyPatchSM hw;

class ClockUtils
{
public:


// provide a pulse (aka clock signal) to calculate the time between 2 pulses

/* calculate frequency from the given period samples. 
*   the formula for calculating the frequency from a given period is:
*   F = 1 / T 
*   where F is frequency and T is period  
*/

void Start()
{
    if(hw.gate_in_1.Trig())
    {
        clockTiming();
    }
}

float calculateFrequency()
{
    frequency_ = 1 / (static_cast<float>(period_) / 1e6);
    return frequency_ / 2;
}

uint32_t returnPeriod()
{
    return period_;
}

private:
float frequency_;
uint32_t previous_clock_;
uint32_t period_;


// private functions 
uint32_t us_diff(uint32_t now, uint32_t last)
{
	// credit nick @  https://www.infrasonicaudio.com/
    if (now < last)
    {
        // handle wraparound
        now += UINT32_MAX / (System::GetTickFreq() / 1e6);
    }
    return now - last;
}

void clockTiming() 
{  
    // credit awonak @ https://github.com/awonak    
    period_ = us_diff(System::GetUs(), previous_clock_);
    previous_clock_ = System::GetUs();
}
};


#endif //CLOCK_UTIL_H