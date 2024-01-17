#include "daisy_patch_sm.h"
#include "daisy.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "processor.h"
#include "effects_handling.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#define MAX_SIZE static_cast<size_t>(48000)
DaisyPatchSM hw;
InterestingProcessor<MAX_SIZE> DSY_SDRAM_BSS proc;
EffectsHandling effects;
CpuLoadMeter loadMeter;
int mod;

void ModManager();
void UpdateParameters()
{
	float speed1 = fmap(hw.GetAdcValue(CV_1), 0.1f, 1.0f);
	float speed2 = fmap(hw.GetAdcValue(CV_2), 0.1f, 1.0f);
	float speed3 = fmap(hw.GetAdcValue(CV_3), 0.1f, 1.0f);
	float speed4 = fmap(hw.GetAdcValue(CV_4), 0.1f, 1.0f);
	proc.SetSpeed(speed1, speed2, speed3, speed4);

	float amount, diffusion, time, gain;
	amount = fmap(hw.GetAdcValue(ADC_9), 0.1f, 1.0f);
	diffusion = fmap(hw.GetAdcValue(ADC_10), 0.1f, 1.0f);
	time = fmap(hw.GetAdcValue(ADC_11), 0.1f, 1.0f);// 0.5f + (0.49f * patch_position));
    
	if(proc.Freeze()) 
	{
		float window = fmap(hw.GetAdcValue(ADC_12), 1.f, 14000.0f);
		proc.SetWindow(window);
	} else { gain = fmap(hw.GetAdcValue(ADC_12), 0.1f, 1.0f);}

	effects.UpdateReverbParams(amount,diffusion,time,gain);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	UpdateParameters();
	for (size_t i = 0; i < size; i++)
	{
		float output = 0.f;
		float inputs = IN_L[i]+IN_R[i];

		proc.Process(inputs);
		for (size_t i = 0; i < 4; i++)
		{
			output += proc.PlaybackSample(i);
			if(proc.Freeze())
			{
				proc.FreezePlayback(i);
			}
		}
		effects.Process(&output, &output, 1);

		OUT_L[i] = output;
		OUT_R[i] = output;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(48); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();
	
	InitBed();
	proc.Init(sample_rate);
	effects.Init(sample_rate);
	
	hw.StartAudio(AudioCallback);
	while(1) 
	{
		ModManager();
		// used to handle UI for samples
		gen_led1.Write(proc.GetState(0));
		gen_led2.Write(proc.GetState(1));
		cv_led1.Write(proc.GetState(2));
		cv_led2.Write(proc.GetState(3));

		// handle grain generation
		if(hw.gate_in_1.Trig())
		{
			proc.Generate();
		}
	}
}

void ModManager()
{
	b1.Debounce();
	b2.Debounce();

	// handling setting the mod param
	mod = (b2.Pressed() * 0b10) | (b1.Pressed() * 0b01);

	gate_out_led1.Write(mod & 0b01);
	gate_out_led2.Write(mod & 0b10);

	// if mod 2, and b1 pressed, toggle forward/reverse
	if(mod == 2 && b1.RisingEdge()) {proc.FlipDirection(); }

	// if mod 1 and b2 pressed, toggle reverb
	if(mod == 1 && b2.RisingEdge()) { effects.FlipEffects(); }

	if(hw.gate_in_2.State()) { proc.SetFreeze(); }

	// eject button
	if(b2.TimeHeldMs() >= 3000) {proc.SetFreeze(); }

	// update leds to reflect modes
	gate_in_led1.Write(proc.returnState());
	gate_in_led2.Write(effects.returnState());
}

