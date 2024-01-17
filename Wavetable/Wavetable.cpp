/**
 * 
 *  Oscillator made with custom waveforms. The base oscillator class heavily leans on DaisySP's oscillator class.
 *  The Envelope setup was also adapted from Daisy's EnvelopeExample.
 *  Full disclosure: I'm 1000% pretending to know math here. 
 * 
*/

#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "FilterButterworth24db.h"
#include "baby_osc.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 			hw;
CFilterButterworth24db 	filter;
BabyOscillator 			wv;
AdEnv					env;

bool env_state;
void ControlUpdate();
void DisplayTimeWaveform(float signal);
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	ControlUpdate();
	b1.Debounce();
	for (size_t i = 0; i < size; i++)
	{
		float env_out, wave, output;
		if(b1.Pressed()||hw.gate_in_1.State()) { env.Trigger(); }

		env_out = env.Process();
		wv.SetAmp(env_out);
		wave = wv.Process();
		output = filter.Run(wave);

		OUT_L[i] = output;
		OUT_R[i] = output;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	float sample_rate = hw.AudioSampleRate();
	InitBed();

	// set up sound / mod sources
	wv.Init(sample_rate);
	filter.SetSampleRate(sample_rate);
    env.Init(sample_rate);
    env.SetCurve(0); // linear

	hw.StartAudio(AudioCallback);
	while(1) {}
}
void ControlUpdate()
{
	hw.ProcessAllControls();
	gate_in_led1.Write(b1.Pressed());

	// ripped from simple oscillator example
    float coarse_knob = hw.GetAdcValue(CV_1);
    float coarse      = fmap(coarse_knob, 36.f, 96.f);

	float voct_cv = hw.GetAdcValue(CV_5);
    float voct    = fmap(voct_cv, 0.f, 60.f);

    float midi_nn = fclamp(coarse + voct, 0.f, 127.f);
    float freq    = mtof(midi_nn);
	wv.SetFreq(freq);

	// filter settings
	float cutoff = fmap(hw.GetAdcValue(CV_2), 0.f, 22050.f);
	float q = fmap(hw.GetAdcValue(CV_3), 0.f, 1.f);
	filter.Set(cutoff, q);

	// wavemod 
	float max_n = fmap(hw.GetAdcValue(CV_4)+hw.GetAdcValue(CV_6), 0.f, 10.f);
	wv.SetMod(max_n);

	float attack = fmap(hw.GetAdcValue(ADC_9), 0.f, 1.f);
	float decay = fmap(hw.GetAdcValue(ADC_10), 0.f, 1.f);
	env.SetTime(ADENV_SEG_ATTACK, attack);
	env.SetTime(ADENV_SEG_DECAY, decay);

}



void DisplayTimeWaveform(float signal, size_t pos)
{
    // Clear the display
    pb.display.Fill(false);

        float amplitude = signal;
        if (amplitude != 0.0) // Skip zero values
        {
            int y = static_cast<int>((amplitude + 1.0) * 0.5 * (pb.display.Height() - 1));

            // Draw a vertical line at the current position
            pb.display.DrawPixel(pos, pb.display.Height() - y, true);
        }

    // Update the display
    pb.display.Update();
}