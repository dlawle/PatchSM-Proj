#pragma once

#include "daisy.h"
#include "per/gpio.h"
#include "daisy_patch_sm.h"

namespace daisy
{
namespace patch_sm
{
    // simple stupid led set up
    GPIO    gen_led1;
    GPIO    gen_led2;
    GPIO    cv_led1;
    GPIO    cv_led2;
    GPIO    gate_in_led1;
    GPIO    gate_in_led2;
    GPIO    gate_out_led1;
    GPIO    gate_out_led2;


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
    // ^^^ {DSY_GPIOB, 4},  /**< D1  - SPI2 CS */

    // gate leds
    dsy_gpio_pin oldPinD3 = DaisyPatchSM::D3;
    Pin newPinD3 = Pin(static_cast<GPIOPort>(oldPinD3.port), oldPinD3.pin);
    dsy_gpio_pin oldPinD6 = DaisyPatchSM::D6;
    Pin newPinD6 = Pin(static_cast<GPIOPort>(oldPinD6.port), oldPinD6.pin);
    dsy_gpio_pin oldPinD4 = DaisyPatchSM::D4;
    Pin newPinD4 = Pin(static_cast<GPIOPort>(oldPinD4.port), oldPinD4.pin);
    dsy_gpio_pin oldPinD5 = DaisyPatchSM::D5;
    Pin newPinD5 = Pin(static_cast<GPIOPort>(oldPinD5.port), oldPinD5.pin);

    // remap
     
    // map buttons
    Switch      b1,b2;

    void InitBed(){
        // leds
        gen_led1.Init(newPinD7, GPIO::Mode::OUTPUT);
        gen_led2.Init(newPinA9, GPIO::Mode::OUTPUT);
        cv_led1.Init(newPinD2, GPIO::Mode::OUTPUT);
        cv_led2.Init(newPinD1, GPIO::Mode::OUTPUT);
        gate_in_led1.Init(newPinD3, GPIO::Mode::OUTPUT);
        gate_in_led2.Init(newPinD4, GPIO::Mode::OUTPUT);
        gate_out_led1.Init(newPinD6, GPIO::Mode::OUTPUT);
        gate_out_led2.Init(newPinD5, GPIO::Mode::OUTPUT);

        // buttons
        b1.Init(DaisyPatchSM::A8);
        b2.Init(DaisyPatchSM::D10);
    }

    void ledOff(){
        gen_led1.Write(false);
        gen_led2.Write(false);
        cv_led1.Write(false);
        cv_led2.Write(false);
    }

    void StartupCheese(){
		gate_in_led1.Toggle();
		System::Delay(100);
		gate_in_led2.Toggle();
		System::Delay(100);
		gen_led1.Toggle();
		System::Delay(100);
		gen_led2.Toggle();
		System::Delay(100);
		cv_led1.Toggle();
		System::Delay(100);
		cv_led2.Toggle();
		System::Delay(100);
		gate_out_led1.Toggle();
		System::Delay(100);
		gate_out_led2.Toggle();
		System::Delay(100);
        ledOff();
    }

    void gateTest()
    {
        gate_in_led1.Toggle();
        System::Delay(1000);
        gate_in_led2.Toggle();
        System::Delay(1000);
        gate_out_led1.Toggle();
        System::Delay(1000);
        gate_out_led2.Toggle();
        System::Delay(1000);
    }

} // namespace patch_sm

} // namespace daisy