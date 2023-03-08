#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

DaisyPatchSM		hw;
PolyPluck<32>		pp;
ReverbSc			rv;
MoogLadder			ladder;
Metro				tick;

// MIDI note numbers for a major triad
const float kArpeggio[3] = {48.0f, 52.0f, 55.0f};
uint8_t     arp_idx;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();

	float trig, freq, sig_out;
	trig = 0.f; 

	float speed = fmap(hw.adc.GetFloat(ADC_12), 1.f, 40.f);
	tick.SetFreq(speed);

	float decay = fmap(hw.adc.GetFloat(CV_4), 0.f, 1.f);
	pp.SetDecay(decay);
	
	float feedback = fmap(hw.adc.GetFloat(CV_3), 0.f, 1.f);
	rv.SetFeedback(feedback);

	float lpf = fmap(hw.adc.GetFloat(CV_2), 1000.f, 32000.f, Mapping::LOG);
	rv.SetLpFreq(lpf);

	float mlpf = fmap(hw.adc.GetFloat(CV_1), 1000.f, 32000.f, Mapping::LOG);
	ladder.SetFreq(mlpf);

	for (size_t i = 0; i < size; i++)
	{
		if(tick.Process()){
			arp_idx = (arp_idx + 1) % 3; // advance the kArpeggio, wrapping at the end.
			trig = 1.f;
		}
		freq    =  kArpeggio[arp_idx];

		sig_out = pp.Process(trig, freq);
		sig_out = ladder.Process(sig_out);

		rv.Process(sig_out, sig_out, &OUT_L[i], &OUT_R[i]);

		out[0][i] = OUT_L[i] + (sig_out * .5f);
		out[1][i] = OUT_R[i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();
	
	// pluck init 
	pp.Init(sample_rate);

	// reverb init
	rv.Init(sample_rate);

	// filter 
    ladder.Init(sample_rate);
    ladder.SetRes(0.4);

	// metro init 
	tick.Init(5.f, sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
