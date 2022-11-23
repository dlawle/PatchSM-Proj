#pragma once


#include "daisy_patch_sm.h"
#include "daisy.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace patch_sm;

//DaisyPatchSM     hw;

/** To make it easier to refer to specific buttons in the 
 *  code, we use this enum to give each button ID a specific name.
 */
enum ButtonIds
{
    bttnEncoder = 0,
    // We don't have any more buttons on the Patch, but if there were more, you'd add them here
    NUM_BUTTONS
};

/** To make it easier to refer to specific encoders in the 
 *  code, we use this enum to give each encoder ID a specific name.
 */
enum EncoderIds
{
    encoderMain = 0,
    // We don't have any more encoders on the Patch, but if there were more, you'd add them here
    NUM_ENCODERS
};

/** To make it easier to refer to specific canvases in the 
 *  code, we use this enum to give each canvas ID a specific name.
 */
enum CanvasIds
{
    // This canvas is for the OLED display
    canvasOledDisplay = 0,
    NUM_CANVASES
};

/** This is the type of display we use on the patch. This is provided here for better readability. */
//using OledDisplayType =  OledDisplay<SSD130x4WireSpi128x64Driver>;
using OledDisplayType = OledDisplay<SSD130x4WireSpi128x64Driver>;
//MyOledDisplay display;

// These will be called from the UI system. @see InitUi() in UiSystemDemo.cpp
void FlushCanvas(const daisy::UiCanvasDescriptor& canvasDescriptor)
{
    if(canvasDescriptor.id_ == canvasOledDisplay)
    {
        OledDisplayType& display
            = *((OledDisplayType*)(canvasDescriptor.handle_));
        display.Update();
    }
}
void ClearCanvas(const daisy::UiCanvasDescriptor& canvasDescriptor)
{
    if(canvasDescriptor.id_ == canvasOledDisplay)
    {
        OledDisplayType& display
            = *((OledDisplayType*)(canvasDescriptor.handle_));
        display.Fill(false);
    }
}