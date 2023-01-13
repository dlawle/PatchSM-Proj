#include "daisy_patch_sm.h"
#include "daisysp.h"
//#include "samplebuffer.h"
#include "reffubelpmas.h"
#include <array>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM    hw;
Switch          toggle;
Switch          button;
ReverbSc        rv;

//sample setup 
#define NUM_BUFFERS 8
// 1 second sample buffers at 48kHz
#define BUFFER_SIZE (48000 * 1)
std::array<SampleBuffer<BUFFER_SIZE>, NUM_BUFFERS> DSY_SDRAM_BSS buffers;
int s = 0;

void getSamples(){
    hw.ProcessAllControls();

	button.Debounce();
    toggle.Debounce(); 

    float time_knob = hw.GetAdcValue(CV_1);
    float time      = fmap(time_knob, 0.3f, 0.99f);

    float damp_knob = hw.GetAdcValue(CV_2);
    float damp      = fmap(damp_knob, 1000.f, 19000.f, Mapping::LOG);

    rv.SetFeedback(time);
    rv.SetLpFreq(damp);
    
    // randomizer
	int randSamp = rand() % NUM_BUFFERS;

    // playState will choose between gate states or otherwise
    //bool playState = toggle.Pressed();

    for(size_t i = 0; i < buffers.size(); i++)
    {
        if(hw.gate_in_2.Trig()) {
            buffers[randSamp].Play();
        }
    }
    switch(s) {
        case 0:
            if(button.RisingEdge() && !buffers[0].IsRecording()) {
                buffers[0].Record();
            } else if(button.FallingEdge() && buffers[0].IsRecording()) {
                buffers[0].Record(false);
                s++;
            }
            break;;
        case 1:
            if(button.RisingEdge() && !buffers[1].IsRecording()) {
                buffers[1].Record();
            } else if(button.FallingEdge() && buffers[1].IsRecording()) {
                buffers[1].Record(false);
                s++;
            }
            break;;
        case 2:
            if(button.RisingEdge() && !buffers[2].IsRecording()) {
                buffers[2].Record();
            } else if(button.FallingEdge() && buffers[2].IsRecording()) {
                buffers[2].Record(false);
                s++;
            }
            break;;
        case 3:
            if(button.RisingEdge() && !buffers[3].IsRecording()) {
                buffers[3].Record();
            } else if(button.FallingEdge() && buffers[3].IsRecording()) {
                buffers[3].Record(false);
                s++;
            }
            break;;
        case 4:
            if(button.RisingEdge() && !buffers[4].IsRecording()) {
                buffers[4].Record();
            } else if(button.FallingEdge() && buffers[4].IsRecording()) {
                buffers[4].Record(false);
                s++;
            }
            break;;
        case 5:
            if(button.RisingEdge() && !buffers[5].IsRecording()) {
                buffers[5].Record();
            } else if(button.FallingEdge() && buffers[5].IsRecording()) {
                buffers[5].Record(false);
                s++;
            }
            break;;
        case 6:
            if(button.RisingEdge() && !buffers[6].IsRecording()) {
                buffers[6].Record();
            } else if(button.FallingEdge() && buffers[6].IsRecording()) {
                buffers[6].Record(false);
                s++;
            }
            break;;
        case 7:
            if(button.RisingEdge() && !buffers[7].IsRecording()) {
                buffers[7].Record();
            } else if(button.FallingEdge() && buffers[7].IsRecording()) {
                buffers[7].Record(false);
                s++;
            }
            break;;
    }

}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{   
    float wet;
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

            OUT_L[i] += sample;
            OUT_R[i] += sample;
        }

        // Feed stereo input through to output, as well as process reverb POST buffers
        rv.Process(OUT_L[i],  OUT_R[i], &wet, &wet);
        OUT_L[i] += IN_L[i] + wet;
        OUT_R[i] += IN_R[i] + wet;
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
    rv.Init(hw.AudioSampleRate());

    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAudio(AudioCallback);

	while(1) {
	}
}
