#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 	hw;
Metro			clock;
Oscillator		osc[4];
AdEnv			env[4];
int  stepNumber;

void ProcessKnobs();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	ProcessKnobs();
	for (size_t i = 0; i < size; i++)
	{
		float sig,env_out[4];
		if(clock.Process())
		{
			stepNumber++;
			stepNumber %= 4;
			env[stepNumber].Trigger();
		}

		for (size_t i = 0; i < 4; i++)
		{
			env_out[i] = env[i].Process();
			osc[i].SetAmp(env_out[i]);
			sig =+ osc[i].Process();
		}

		OUT_L[i] = sig;
		OUT_R[i] = sig;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	// init devices and audio
	float sample_rate = hw.AudioSampleRate();
	InitBed();
	clock.Init(3, sample_rate);
	stepNumber = 0;

	for (size_t i = 0; i < 4; i++)
	{
		//oscillators 
		osc[i].Init(sample_rate);
		osc[i].SetWaveform(Oscillator::WAVE_POLYBLEP_SAW);

		// envelopes
		env[i].Init(sample_rate);
		env[i].SetTime(ADENV_SEG_ATTACK, 0.15);
		env[i].SetTime(ADENV_SEG_DECAY, 0.35);
		env[i].SetMin(0.0);
		env[i].SetMax(0.25);
		env[i].SetCurve(0); // linear
	}
	

	hw.StartAudio(AudioCallback);
	while(1) {}
}

void ProcessKnobs()
{
	float f1 = hw.GetAdcValue(CV_1);
	float f2 = hw.GetAdcValue(CV_2);
	float f3 = hw.GetAdcValue(CV_3);
	float f4 = hw.GetAdcValue(CV_4);

	osc[0].SetFreq(f1);
	osc[1].SetFreq(f2);
	osc[2].SetFreq(f3);
	osc[3].SetFreq(f4);

}