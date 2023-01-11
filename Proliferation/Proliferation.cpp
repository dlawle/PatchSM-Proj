#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "samplebuffer.h"
#include <array>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
Switch toggle;
Switch button;

#define NUM_BUFFERS 8
// 5 second sample buffers at 48kHz
#define BUFFER_SIZE (48000 * 5)
std::array<SampleBuffer<BUFFER_SIZE>, NUM_BUFFERS> DSY_SDRAM_BSS buffers;

void getSamples(){
	hw.ProcessAllControls();
	button.Debounce();

	int randSamp = rand() % 8;
	float s = 0;

    for(size_t i = 0; i < buffers.size(); i++)
    {
        if(hw.gate_in_1.Trig()) {
            buffers[randSamp].Play();
        } else if(hw.gate_in_1.Trig()) {
            //buffers[i].Play(false);
        }

		if(hw.gate_in_2.Trig()) {
            buffers[randSamp].Play();
        } else if(hw.gate_in_2.Trig()) {
            //buffers[i].Play(false);
        }
    }

	if(button.RisingEdge() && !buffers[s].IsRecording()) {
		buffers[s].Record();
	} else if(button.FallingEdge() && buffers[s].IsRecording()) {
		buffers[s].Record(false);
		s++;
	}

	if (s >= 8) {
		s = 0;
	}

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	getSamples();

	for (size_t i = 0; i < size; i++)
	{
        // Zero samples prior to summing
        OUT_L[i] = 0.f;
        OUT_R[i] = 0.f;
        for(auto &buffer : buffers)
        {
            // Record in mono
            float sample = buffer.Process((0.5 * IN_L[i] + 0.5 * IN_R[i]));
            // Sum all playback channels
            OUT_L[i] += sample;
            OUT_R[i] += sample;
        }
        // Feed stereo input through to output
        OUT_L[i] += IN_L[i];
        OUT_R[i] += IN_R[i];
	}
}

void InitSamplers(){
    for(auto &buffer : buffers)
    {
        buffer.Init();
    }
}


int main(void)
{
	hw.Init();

	toggle.Init(hw.B8);
	button.Init(hw.B7);
	InitSamplers();

	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);

	while(1) {
	}
}
