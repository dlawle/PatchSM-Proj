#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "clock_util.h"


using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
ClockUtils	 util;
float freq;
bool tic;
uint32_t period;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		OUT_L[i] = IN_L[i];
		OUT_R[i] = IN_R[i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	InitBed();
	hw.StartLog(); // for testing 

	hw.StartAudio(AudioCallback);
	while(1) 
	{ 
		util.Start();
		hw.PrintLine("Value: %f", util.calculateFrequency());
	}
}