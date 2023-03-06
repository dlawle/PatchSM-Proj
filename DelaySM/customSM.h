#pragma once

#include "daisy.h"
#include "daisysp.h"
#include "per/gpio.h"

namespace daisy
{
namespace patch_sm
{

    // simple stupid led set up

    GPIO    gen_led1;
    GPIO    gen_led2;
    GPIO    cv_led1;
    GPIO    cv_led2;
    GPIO    gate_in_led;
    GPIO    gate_out_led;

    // generic leds
    dsy_gpio_pin oldPinD7 = DaisyPatchSM::D7;
    Pin newPinD7 = Pin(static_cast<GPIOPort>(oldPinD7.port), oldPinD7.pin);
    dsy_gpio_pin oldPinA9 = DaisyPatchSM::A9;
    Pin newPinA9 = Pin(static_cast<GPIOPort>(oldPinA9.port), oldPinA9.pin);

    // cv leds
    dsy_gpio_pin oldPinD2 = DaisyPatchSM::D2;
    Pin newPinD2 = Pin(static_cast<GPIOPort>(oldPinD2.port), oldPinD2.pin);
    dsy_gpio_pin oldPinD1 = DaisyPatchSM::D1;
    Pin newPinD1 = Pin(static_cast<GPIOPort>(oldPinD1.port), oldPinD1.pin);

    // gate leds
    dsy_gpio_pin oldPinD3 = DaisyPatchSM::D3;
    Pin newPinD3 = Pin(static_cast<GPIOPort>(oldPinD3.port), oldPinD3.pin);
    dsy_gpio_pin oldPinD6 = DaisyPatchSM::D6;
    Pin newPinD6 = Pin(static_cast<GPIOPort>(oldPinD6.port), oldPinD6.pin);  

    void InitLEDs(){
        gen_led1.Init(newPinD7, GPIO::Mode::OUTPUT);
        gen_led2.Init(newPinD7, GPIO::Mode::OUTPUT);
        cv_led1.Init(newPinD2, GPIO::Mode::OUTPUT);
        cv_led2.Init(newPinD1, GPIO::Mode::OUTPUT);
        gate_in_led.Init(newPinD3, GPIO::Mode::OUTPUT);
        gate_out_led.Init(newPinD6, GPIO::Mode::OUTPUT);
    }

} // namespace patch_sm

} // namespace daisy