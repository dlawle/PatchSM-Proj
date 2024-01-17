#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "Sequencer.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;


DaisyPatchSM 	hw;
Sequencer		seq;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	//seq.Process();
	for (size_t i = 0; i < size; i++)
	{
		OUT_L[i] = 0.f;
		OUT_R[i] = 0.f;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	// init daisy stuff
	float sample_rate = hw.AudioSampleRate();
	seq.Init(sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) 
	{
		seq.Process();
	}
}


