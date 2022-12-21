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
	float nn, trig, sig_out; 

	// set up v/oct tracking
	float coarse_tune = 24.0f + hw.GetAdcValue(CV_4) * 60.0f;
    float cv_voct = hw.GetAdcValue(CV_5);
    float voct    = fmap(cv_voct, 0.f, 60.f);

    /** Convert from MIDI note number to frequency */
    nn = fclamp(coarse_tune + voct, 0.f, 127.f);
    nn = static_cast<int32_t>(nn); // Quantize to semitones

    // Set brightness, dampening and nonlinearity knobs respectively 
    float bright_knob 			= hw.GetAdcValue(CV_1);
    float bright      			= fmap(bright_knob, 0.3f, 0.99f);
	float damp_knob	  			= hw.GetAdcValue(CV_2);
	float dampening   		  	= fmap(damp_knob, 0.3f, 0.99f);
	float nonlinear_knob	  	= hw.GetAdcValue(CV_3);
	float nonlinear   			= fmap(nonlinear_knob, -0.99f, 0.99f);

	string.SetBrightness(bright); 
	string.SetDamping(dampening);
	string.SetNonLinearity(nonlinear);

    for(size_t i = 0; i < size; i += 2)
    {
        trig = 0.0f;
        if(hw.gate_in_1.Trig())
        {
            string.SetFreq(nn);
            trig = 1.0f;
        }
        sig_out = string.Process(trig);
        // Output
        OUT_L[i] = sig_out;
        OUT_R[i] = sig_out;
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
