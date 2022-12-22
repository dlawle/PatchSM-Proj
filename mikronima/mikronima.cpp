#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
StringVoice str;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    // set CV inputs and Coarse knob
    float coarse_knob = hw.GetAdcValue(CV_1);
    float coarse      = fmap(coarse_knob, 36.f, 96.f);

    float voct_cv = hw.GetAdcValue(CV_5);
    float voct    = fmap(voct_cv, 0.f, 60.f);

    float midi_nn = fclamp(coarse + voct, 0.f, 127.f);
    float freq    = mtof(midi_nn);

	// set dampening and sustain (testing)
	float damp = fmap(hw.GetAdcValue(CV_2), 0.0f, .99f);
	float sustain = fmap(hw.GetAdcValue(CV_3), 0.0f, .99f);


	hw.ProcessAllControls();
    for(size_t i = 0; i < size; i++)
    {
        bool t = hw.gate_in_1.Trig();
        if(t)
        {
            str.SetFreq(freq);
			str.SetDamping(damp);
            str.SetSustain(sustain);
        }

		float sig = hw.GetAdcValue(CV_6);
        str.SetStructure(sig);
        str.SetBrightness(.1f + sig * .2f);

        out[0][i] = out[1][i] = str.Process(t);
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);

	float sample_rate = hw.AudioSampleRate();
	str.Init(sample_rate);
    str.SetAccent(1.f);
	
	while(1) {}
}
