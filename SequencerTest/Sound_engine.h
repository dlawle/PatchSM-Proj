#pragma once
#ifndef SOUND_ENGINE_H_
#define SOUND_ENGINE_H_

#include "daisy.h"
#include "daisysp.h"
#include "Sequencer.h"

Sequencer int_seq;

class Sound_engine
{
public:
    void Init(float sample_rate)
    {
        plstep1.Init(sample_rate);
        plstep2.Init(sample_rate);
        plstep3.Init(sample_rate);
        plstep4.Init(sample_rate); 
    }



private:
    PolyPluck<4> plstep1; 
    PolyPluck<4> plstep2; 
    PolyPluck<4> plstep3; 
    PolyPluck<4> plstep4; 
};


#endif //SOUND_ENGINE_H_