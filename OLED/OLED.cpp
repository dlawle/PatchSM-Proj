#include <stdio.h>
#include <string.h>
#include "daisy_patch_sm.h"
#include "dev/oled_ssd130x.h"
#include <hid/encoder.h>

using namespace daisy;
using namespace patch_sm;
using MyOledDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;

DaisyPatchSM     hw;
MyOledDisplay display;
Encoder myEnc;

int main(void)
{
    uint8_t message_idx;
    hw.Init();

    /* Init Encoder (left pin, right pin, click pin)*/
	myEnc.Init(DaisyPatchSM::D8,DaisyPatchSM::A8,DaisyPatchSM::B7); /* pins are currently placeholder values*/

    /** Configure the Display */
    MyOledDisplay::Config disp_cfg;

    disp_cfg.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_2;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.sclk = hw.GetPin(DaisyPatchSM::PinBank::D, 10);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.mosi = hw.GetPin(DaisyPatchSM::PinBank::D, 9);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.nss  = hw.GetPin(DaisyPatchSM::PinBank::D, 1);
    disp_cfg.driver_config.transport_config.pin_config.dc    = hw.GetPin(DaisyPatchSM::PinBank::A, 2);
    disp_cfg.driver_config.transport_config.pin_config.reset = hw.GetPin(DaisyPatchSM::PinBank::A, 3);
    /** And Initialize */
    display.Init(disp_cfg);

    message_idx = 0;
    char strbuff[128];
    while(1)
    {
        System::Delay(500);
        switch(message_idx)
        {
            case 0: sprintf(strbuff, "Testing. . ."); break;
            case 1: sprintf(strbuff, "Daisy. . ."); break;
            case 2: sprintf(strbuff, "1. . ."); break;
            case 3: sprintf(strbuff, "2. . ."); break;
            case 4: sprintf(strbuff, "3. . ."); break;
            default: break;
        }
        message_idx = (message_idx + 1) % 5;
        display.Fill(true);
        display.SetCursor(0, 0);
        display.WriteString(strbuff, Font_11x18, false);
        display.Update();

        /** Debounce the switch */
        myEnc.Debounce();

        /** Get the current button state */
        bool state = myEnc.Pressed();

        /** Set the onboard led to the current state */
        hw.SetLed(state);
    }
}
