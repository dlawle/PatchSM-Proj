#include "daisy.h"
#include "daisysp.h"
#include "daisy_patch_sm.h"
#include "daisy_bed.h"
#include "warps/dsp/modulator.h"
#include "warps/settings.h"


using namespace daisy;
using namespace daisysp;
using namespace patch_sm;


DaisyPatchSM hw;
warps::Modulator warp;
//void UpdateParameters() 
//{
//	p->modulation_algorithm = fmap(hw.GetAdcValue(CV_1), 0.f, 1.0f);
//	p->modulation_parameter = fmap(hw.GetAdcValue(CV_2), 0.f, 1.0f);
//	p->carrier_shape = fmap(hw.GetAdcValue(CV_3), 0.f, 1.0f);
//	p->note = fmap(hw.GetAdcValue(CV_4), 0.f, 1.0f);
//	p->channel_drive[0] = fmap(hw.GetAdcValue(ADC_9), 0.f, 1.0f);
//	p->channel_drive[1] = fmap(hw.GetAdcValue(ADC_10), 0.f, 1.0f);
//}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
	    warps::ShortFrame input[size];
	    warps::ShortFrame output[size];

		OUT_L[i] = IN_L[i] + IN_R[i];
		OUT_R[i] = IN_L[i] + IN_R[i];
	}


}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	InitBed();
	float sample_rate = hw.AudioSampleRate();
	warp.Init(48000.f);
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);
	while(1) {}
}
