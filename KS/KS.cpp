#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

String 		 string;
DaisyPatchSM hw;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	float nn, trig; 

    // set CV inputs and Coarse knob
    float coarse_knob = hw.GetAdcValue(CV_1);
    float coarse      = fmap(coarse_knob, 36.f, 96.f);

    float voct_cv = hw.GetAdcValue(CV_5);
    float voct    = fmap(voct_cv, 0.f, 60.f);

    float midi_nn = fclamp(coarse + voct, 0.f, 127.f);
    float freq    = mtof(midi_nn);

    

    for(size_t i = 0; i < size; i += 2)
    {
        bool trig = hw.gate_in_1.Trig();
        if(trig)
        {
            string.SetFreq(freq);
        }
        // Output
        OUT_L[i] = string.Process(trig);
        OUT_R[i] = string.Process(trig);
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    // Set up Pluck algo
    string.Init(hw.AudioSampleRate());

	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	while(1) {}
}
