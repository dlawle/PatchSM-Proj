#include "control.h"
using namespace bed; 

Control::Control(DaisyPatchSM& patch, Engine& engine) : hw(patch), engine(engine) {}

Control::~Control() {}

void Control::ProcessControls()
{
    pot1 = hw.GetAdcValue(CV_1);
    pot2 = hw.GetAdcValue(CV_2);
    pot3 = hw.GetAdcValue(CV_3);
    pot4 = hw.GetAdcValue(CV_4);
    pot5 = hw.GetAdcValue(ADC_9);
    pot6 = hw.GetAdcValue(ADC_10);
    pot7 = hw.GetAdcValue(ADC_11);
    pot8 = hw.GetAdcValue(ADC_12);

    engine.SetParam(pot1, pot2, pot3, pot4, pot5, pot6, pot7, pot8);
}

// for use with b2. This callback sets the current "effect suite"
void Control::ButtonCallback()
{
    b2.Debounce();
    button_cb = b2.RisingEdge();
    if (button_cb)
    {
        currentEffect = static_cast<EffectType>((currentEffect + 1) % 4);
    }

    switch (currentEffect)
    {
		case Effect1:
			// Background Mode
			engine.UpdateProcesor(1);
            ledOff();
            gen_led1.Toggle();
			break;

		case Effect2:
			// Apply Effect 2
            ledOff();
            gen_led2.Toggle();
			engine.UpdateProcesor(2);
			break;

		case Effect3:
			// Apply Effect 3
            ledOff();
            cv_led1.Toggle();
			engine.UpdateProcesor(3);
			break;

		case Effect4:
			// Loop Mode
            ledOff();
            cv_led2.Toggle();
			engine.UpdateProcesor(4);
			break;
        
        default:
            ledOff();
            gen_led1.Toggle();
            engine.UpdateProcesor(1);
            break;
    }
}

EffectType Control::GetType()
{
    return currentEffect;
}


// to be used with b1. this button will send a callback signal to the engine in order to update the Process() function.
void Control::ButtonCallback2()
{
    b1.Debounce();
    button_cb2 = b1.Pressed();
    engine.GetActivate(button_cb2);
    gate_in_led1.Write(button_cb2);
}

void Control::Process()
{
    ProcessControls();
	ButtonCallback();
	ButtonCallback2();
}