#pragma once 
#ifndef PATCH_BAY_BSP_H
#define PATCH_BAY_BSP_H

#include "daisy.h"
#include "daisy_patch_sm.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace patch_sm;

namespace daisy {

class Patch_Bay
{
public:
    SdmmcHandler                        sd;
    SdmmcHandler::Config                sd_cfg;
    Switch                              shift;
    Encoder                             enc;
    OledDisplay<SSD130xI2c128x64Driver> display;
    Led                                 cv_ind, gate_1, gate_2;


    /**
     * Note: LEDs are not set up as they're directly tied to 
     * Gate out (x2) and CV out 1
     * 
     * Pinout below is only reference to pins that are 
     * changed/deviate from the noraml pinout 
     * 
     *       [[ Pinout ]]
     * 
     * Control Voltage: 
     * CV_1 (Pot)   = C5
     * CV_2 (Pot)   = C4
     * CV_3 (Pot)   = C3 
     * CV_4 (Pot)   = C2 
     * CV_5 (Input) = C6
     * CV_6 (Input) = C7
     * CV_7 (Input) = C8
     * CV_8 (Input) = C9
     * ADC_9 (POT)  = A2
     * ADC_10 (POT) = A3
     * ADC_11 (POT) = D9
     * ADC_12 (POT) = D8
     * 
     * SD Card:
     * SD_SCK       = D6
     * SD_CMD       = D7
     * SD_D0        = D5
     * SD_D1        = D4
     * SD_D2        = D3
     * SD_D3        = D2
     * 
     * Endcoder:
     * Switch       = A8
     * Enc +        = D1
     * Enc -        = A9
     * 
     * Button: 
     * BUTTON       = D10
     * 
     * LEDs:
     * LED_IND      = C10
     * GATE_1_IND   = B5
     * GATE_2_IND   = B6
    */  
    void Init();

private:
    void InitDisplay();
    void InitEncoder();
    void InitButtons();
    void InitLeds();
    void InitSD();

}; // end class
} // namespace daisy

#endif //PATCH_BAY_BSP_H