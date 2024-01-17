#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "engine.h"
#include "control.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;
using namespace bed;

DaisyPatchSM hw;
Engine engine;
Control control(hw, engine);

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig_in;
	hw.ProcessAllControls();
	control.Process();
	
	for (size_t i = 0; i < size; i++)
	{
		float output;
		sig_in = (IN_L[i] + IN_R[i]) * 0.5f;

		engine.Feed();
		output = engine.Process(sig_in);

		OUT_L[i] = output;
		OUT_R[i] = sig_in;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	
	float sample_rate = hw.AudioSampleRate();
	engine.Init(sample_rate);
	control.Init();
	
	hw.StartAudio(AudioCallback);
	while(1) {}
}

