#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "patch_bay.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
Patch_Bay pb;
uint32_t screen_update_last_, screen_update_period_;

void DisplayControls(bool invert)
{
    enum CVread
    {
        ADC_9,
        ADC_10,
        ADC_11,
        ADC_12,
        CV_LAST
    };
    
    bool on, off;
    on  = invert ? false : true;
    off = invert ? true : false;
    if(hw.system.GetNow() - screen_update_last_ > screen_update_period_)
    {
        // Graph Knobs
        size_t barwidth, barspacing;
        size_t curx, cury;
        screen_update_last_ = hw.system.GetNow();
        barwidth            = 15;
        barspacing          = 20;
        pb.display.Fill(off);
        // Bars for all four knobs.
        for(size_t i = 0; i < CVread::CV_LAST; i++)
        {
            float  v;
            size_t dest;
            curx = (barspacing * i + 1) + (barwidth * i);
            cury = pb.display.Height();
            v    = hw.GetAdcValue(CVread(i));
            dest = (v * pb.display.Height());
            for(size_t j = dest; j > 0; j--)
            {
                for(size_t k = 0; k < barwidth; k++)
                {
                    pb.display.DrawPixel(curx + k, cury - j, on);
                }
            }
        }
        pb.display.Update();
    }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    hw.ProcessAllControls();
    for (size_t i = 0; i < size; i++)
    {
		float output = 0.f;
        OUT_L[i] = output;
        OUT_R[i] = output;
    }
}


int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	pb.Init();
	hw.StartAudio(AudioCallback);
	while(1) 
	{
        DisplayControls(false);
    }
}