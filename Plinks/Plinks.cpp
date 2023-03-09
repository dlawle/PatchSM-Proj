#include "daisy_bed.h"
#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "arp_notes.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
using namespace arps;

DaisyPatchSM		hw;
PolyPluck<32>		pp;
ReverbSc 			rv;
MoogLadder			ladder;
AdEnv      			env;

uint8_t     arp_idx;
uint8_t 	scale_idx;

// select scale, and pass midi note as nn 
float scaleSelect(float nn){
	float freq;
	b1.Debounce();
	if(b1.FallingEdge()){
		scale_idx = (scale_idx + 1) % 4; // advance scale +1, else wrap back to 1? 
	}

	switch (scale_idx)
	{
	case 1:
		freq = nn + noMajPen[arp_idx];
		ledOff();
		gen_led2.Toggle();
		break;
		
	case 2:
		freq = nn + noMinPen[arp_idx];
		ledOff();
		cv_led2.Toggle();
		break;
	case 3:
		freq = nn + noMajTri[arp_idx];
		ledOff();
		cv_led1.Toggle();
		break;
	case 4:
		freq = nn + noMinTri[arp_idx];
		ledOff();
		gen_led1.Toggle();
		break;
	default:
		freq = nn + noMajPen[arp_idx];
		ledOff();
		gen_led2.Toggle();
		break;
	}
	return freq;
	}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	float nn, trig, sig_out;
	trig = 0.f; 

	float decay = fmap(hw.GetAdcValue(CV_4), 0.f, 1.f);
	pp.SetDecay(decay);
	
	float feedback = fmap(hw.GetAdcValue(CV_3), 0.f, 1.f);
	rv.SetFeedback(feedback);

	float lpf = fmap(hw.GetAdcValue(CV_2), 1000.f, 32000.f, Mapping::LOG);
	rv.SetLpFreq(lpf);

	float mlpf = fmap(hw.GetAdcValue(CV_1), 1000.f, 32000.f, Mapping::LOG);
	ladder.SetFreq(mlpf);

	float attack = fmap(hw.GetAdcValue(ADC_12), 0.1f, 5.0f);
	float release = fmap(hw.GetAdcValue(ADC_11), 0.1f, 5.0f);
	env.SetTime(ADENV_SEG_ATTACK, attack);
	env.SetTime(ADENV_SEG_DECAY, release);

	// Set MIDI Note for new Pluck notes.
    nn = 24.0f + hw.GetAdcValue(CV_8) * 60.0f;
    nn = static_cast<int32_t>(nn); // Quantize to semitones

	for (size_t i = 0; i < size; i++)
	{
		if(hw.gate_in_1.Trig()){
			arp_idx = (arp_idx + 1) % 5; // advance the kArpeggio, wrapping at the end.
			trig = 1.f;
		}

		if(hw.gate_in_2.Trig()){
			env.Trigger();
		}

		float new_freq = scaleSelect(nn);

		sig_out = pp.Process(trig, new_freq);
		sig_out = ladder.Process(sig_out);
		sig_out *= env.Process();

		rv.Process(sig_out, sig_out, &OUT_L[i], &OUT_R[i]);

		out[0][i] = OUT_L[i] + (sig_out * .5f);
		out[1][i] = OUT_R[i];
		hw.WriteCvOut(CV_OUT_1, env.Process());
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	// init board
	InitBed();
	
	// pluck init 
	pp.Init(sample_rate);

	// reverb init
	rv.Init(sample_rate);

	// filter 
    ladder.Init(sample_rate);
    ladder.SetRes(0.4);

	// envelope
	env.Init(sample_rate);
	env.SetMin(0.0);
    env.SetMax(0.25);

	hw.StartAudio(AudioCallback);
	while(1) {
		bool state = hw.gate_in_1.State();
		gate_in_led.Write(state);
	}
}
