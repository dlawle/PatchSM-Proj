#pragma once
#ifndef DRUM_H
#define DRUM_H

#include "daisy.h"
#include "daisysp.h"

using namespace daisysp;

class drumSet
{
public:
void Init(float sample_rate)
{
    sample_rate_ = sample_rate;
    kick_.Init(sample_rate_);
    kick_.SetFreq(50.f);
    kick_.SetDecay(.4f);
    snare_.Init(sample_rate_);
    hat_.Init(sample_rate);
    amplitude_ = 0.0f;
}

void updateAmp(int amp) 
{
    amplitude_ = amp;
}

void TriggerKick(bool trig)
{
    if(trig)
    {
        kick_.Trig();
    }
}

void TriggerSnare(bool trig)
{
    if(trig)
    {
        snare_.Trig();
    }
}

void TriggerHat2(bool trig)
{
    if(trig)
    {
        hat_.SetDecay(.2f);
        hat_.SetAccent(.8f);
        hat_.Trig();
    }
}

void TriggerHat(bool trig)
{
    if(trig)
    {
        hat_.SetDecay(1.f);
        hat_.SetAccent(1.f);
        hat_.Trig();
    }
}

float Process()
{
    out_ = (amplitude_ * kick_.Process()) + (amplitude_ * snare_.Process()) + (amplitude_ * hat_.Process());
    return out_;
}

private:
    SyntheticBassDrum kick_;
    SyntheticSnareDrum snare_; 
    HiHat<>            hat_;         
    float sample_rate_;
    float out_;
    float amplitude_ = 0.f;
};



#endif