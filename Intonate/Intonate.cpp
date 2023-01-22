#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 	hw;
PitchShifter	ps;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float shifted, unshifted;
	hw.ProcessAllControls();
	
	float range = hw.GetAdcValue(CV_1);
	float shift = fmap(range, .0f, 12.0f);

	ps.SetTransposition(shift);

	float del = hw.GetAdcValue(CV_2);
	float dsize = fmap(del,0.f,1000.f);

	ps.SetDelSize(dsize);

	float factor = hw.GetAdcValue(CV_3);
	float fun = fmap(factor,0.f,1000.f);

	ps.SetFun(fun);

	for (size_t i = 0; i < size; i++)
	{
		unshifted  = IN_L[i] + IN_R[i];
        shifted    = ps.Process(unshifted);
		OUT_L[i] = IN_L[i];
		OUT_R[i] = shifted;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	ps.Init(sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
