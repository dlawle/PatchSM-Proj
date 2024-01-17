#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
Oscillator 	 osc[2];

#define NUM_WAVEFORMS 8
int waveform[2];
float cv_freq[2];
float env[2];

uint8_t Waveforms[NUM_WAVEFORMS] = {
    Oscillator::WAVE_SIN,
    Oscillator::WAVE_TRI,
    Oscillator::WAVE_SAW,
    Oscillator::WAVE_RAMP,
    Oscillator::WAVE_SQUARE,
    Oscillator::WAVE_POLYBLEP_TRI,
    Oscillator::WAVE_POLYBLEP_SAW,
    Oscillator::WAVE_POLYBLEP_SQUARE
};

void ButtonHandler();
void FreqHandler();
void KnobHandler();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	ButtonHandler();
	FreqHandler();
	KnobHandler();
	for (size_t i = 0; i < size; i++)
	{
		float sig[2];

		sig[0] = osc[0].Process();
		env[0] = fmap(hw.GetAdcValue(CV_6), 0.0f, 1.0f);
		sig[1] = osc[1].Process();
		env[1] = fmap(hw.GetAdcValue(CV_7), 0.0f, 1.0f);

		OUT_L[i] = sig[0] * env[0];
		OUT_R[i] = sig[1] * env[1];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	InitBed();
	for (size_t i = 0; i < 2; i++)
	{
		waveform[i] = 0;
		osc[i].Init(sample_rate);
		osc[i].SetFreq(440.f);
	}
	
	hw.StartAudio(AudioCallback);
	while(1) {}
}


void ButtonHandler()
{
	b1.Debounce();
	b2.Debounce();

	bool wave_one = b1.FallingEdge();
	bool wave_two = b2.FallingEdge();

	if(wave_one)
	{
		waveform[0] = waveform[0]+1;
		waveform[0] = DSY_CLAMP(waveform[0], 0, NUM_WAVEFORMS);
		if(waveform[0] >= NUM_WAVEFORMS) {waveform[0] = 0; };
	}

	if(wave_two)
	{
		waveform[1] = waveform[1]+1;
		waveform[1] = DSY_CLAMP(waveform[1], 0, NUM_WAVEFORMS);
		if(waveform[1] >= NUM_WAVEFORMS) {waveform[1] = 0; };
	}
	osc[0].SetWaveform(Waveforms[waveform[0]]);
	osc[1].SetWaveform(Waveforms[waveform[1]]);
} 

void FreqHandler()
{
	// here's where we will handle CV ins 
	float freq[2], coarse[2], midi_conv[2];

	coarse[0] = fmap(hw.GetAdcValue(CV_1), 36.f, 96.f);
	cv_freq[0] = fmap(hw.GetAdcValue(CV_5), 0.f, 60.f);
	midi_conv[0] = fclamp(coarse[0] + cv_freq[0], 0.f, 127.f);
	freq[0] = mtof(midi_conv[0]);
	
	coarse[1] = fmap(hw.GetAdcValue(CV_3), 36.f, 96.f);
	cv_freq[1] = fmap(hw.GetAdcValue(CV_8), 0.f, 60.f);
	midi_conv[1] = fclamp(coarse[1] + cv_freq[1], 0.f, 127.f);
	freq[1] = mtof(midi_conv[1]);

	for (size_t i = 0; i < 2; i++)
	{
		osc[i].SetFreq(freq[i]);
	}	

	hw.WriteCvOut(CV_OUT_1, freq[0]);
	hw.WriteCvOut(CV_OUT_2, freq[1]);
}

void KnobHandler()
{
	float pw[2];
	pw[0] = fmap(hw.GetAdcValue(CV_2), 0.f, 1.f);
	osc[0].SetPw(pw[0]);
	pw[1] = fmap(hw.GetAdcValue(CV_4), 0.f, 1.f);
	osc[1].SetPw(pw[1]);
}