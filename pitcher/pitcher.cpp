#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
PitchShifter ps;
Oscillator   osc;

#define LEFT (i)
#define RIGHT (i + 1)

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    hw.ProcessAllControls();

    float shifted, unshifted;
	
    for(size_t i = 0; i < size; i += 2)
    {
        float readK   = hw.GetAdcValue(CV_1);
        float transpo = fclamp(readK,0,12);
        
        // set transposition 1 octave up (12 semitones)
        ps.SetTransposition(transpo);
        unshifted  = osc.Process();
        shifted    = ps.Process(unshifted);
        OUT_L[i]  = shifted;
        OUT_R[i] = unshifted;
    }
}

int main(void)
{
    // initialize seed hardware and daisysp modules
    float sample_rate;
    hw.Init();
    hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();

    ps.Init(sample_rate);

    //setup oscillator
    osc.Init(sample_rate);
    osc.SetFreq(440.f);
    osc.SetWaveform(Oscillator::WAVE_TRI);

    // start callback
    hw.StartAudio(AudioCallback);

    while(1) {
    }
}
