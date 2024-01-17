#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "grain.h"


using namespace daisysp;
using namespace daisy;
using namespace patch_sm;

DaisyPatchSM            hw;
GrainTest DSY_SDRAM_BSS grain;
float DSY_SDRAM_BSS grain_buffer[48000];

void AudioCallback(AudioHandle::InputBuffer  in, AudioHandle::OutputBuffer out, size_t size)
{  
    b1.Debounce();
    for(size_t i = 0; i < size; i++)
    {
        float output;
        float inputs = IN_L[i]+IN_R[i];
        grain.Process(inputs);

        OUT_L[i] = 0.f;
        OUT_R[i] = 0.f;
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

    float sample_rate = hw.AudioSampleRate();
    InitBed();
    grain.Init(sample_rate, grain_buffer, 48000);

    hw.StartAudio(AudioCallback);
    while(1)
    {
        
    }
}