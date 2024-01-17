#pragma once
#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include "daisy.h"
#include "daisysp.h"
#include "daisy_bed.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
extern daisy::patch_sm::DaisyPatchSM hw;

// pointers for making writing to LEDs a little easier lol
typedef void (*StepIndicator)(bool a);
void led1(bool a) {gen_led1.Write(a);}
void led2(bool a) {gen_led2.Write(a);}
void led3(bool a) {cv_led1.Write(a);}
void led4(bool a) {cv_led2.Write(a);}

StepIndicator leds[] = {  led1, led2, led3, led4 };

class Sequencer
{

public:
    
    void Init(float sample_rate)
    {
        samp_rate = sample_rate;
        clock.Init(0.5, samp_rate);
        
        reverse_ = false;
        forward_ = true;
        InitBed();
    }

    void ManageClock()
    {
        tic = clock.Process();
        if(tic)
        {
            ledOff();
            dsy_gpio_write(&hw.gate_out_1, true);
            step = (step+1) % 4;
            leds[step](true);
            gate_out_led1.Toggle();
            hw.WriteCvOut(CV_OUT_1, freq[step]);
        }
        dsy_gpio_write(&hw.gate_out_1, false);
    };

    void ManageUI()
    {
        b1.Debounce();
        b2.Debounce();

        mod = (b2.Pressed() * 0b10) | (b1.Pressed() * 0b01);
        gate_in_led1.Write(mod == 1 || mod == 3);
        gate_in_led2.Write(mod == 2 || mod == 3);

    }

    void SetParameters()
    {
        
        freq[0] = fmap(hw.GetAdcValue(CV_1), 0.f, 127.f);
        freq[1] = fmap(hw.GetAdcValue(CV_2), 0.f, 127.f);
        freq[2] = fmap(hw.GetAdcValue(CV_3), 0.f, 127.f);
        freq[3] = fmap(hw.GetAdcValue(CV_4), 0.f, 127.f);
        param2[0] = hw.GetAdcValue(ADC_9);
        param2[1] = hw.GetAdcValue(ADC_10);
        param2[2] = hw.GetAdcValue(ADC_11);
        param2[3] = hw.GetAdcValue(ADC_12);

        for (size_t i = 0; i < 4; i++)
        {
            freq[i] = fmap(mtof(freq[i]), 0.f, 5.f);
        }
        
    }

    void Process()
    {
        ManageClock();
        ManageUI();
        SetParameters();
    }

private:
    Metro clock;
    uint8_t tic;
    float freq[4];
    float param2[4];
    float samp_rate;
    float sig;
    float env_out;
    int step;
    int mod;

    // direction flags
    bool reverse_, forward_;
};


#endif //SEQUENCER_H_
