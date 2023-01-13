#include "daisysp.h"
#include "daisy_patch_sm.h"
#include <array>
#include <stddef.h>
#include <unistd.h>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM    hw;
Switch          toggle;
Switch          button;
ReverbSc        rv;
Svf             svf;

/** Sample buffer with record and playback.

By: Alex Virgona
*/
template <size_t kBufferSize>
class SampleBuffer
{
  public:
    SampleBuffer() {}
    ~SampleBuffer() {}

    void Init()
    {
        Clear();
        recording_ = false;
        write_ptr_ = 0;
        playing_   = false;
        read_ptr_  = 0;
    }

    void Clear()
    {
        for(size_t i = 0; i < kBufferSize; i++)
        {
            buffer_[i] = 0.f;
        }
    }

    void Record(bool start = true)
    {
        if(start)
        {
            Clear();
        }
        recording_ = start;
        write_ptr_ = 0;
    }

    void Play(bool start = true)
    {
        playing_  = start;
        read_ptr_ = 0;
    }

    // testing reverse play 
    void PlayBackward(bool start = true)
    {
        playing_  = start;
        read_ptr_back_ = kBufferSize;
    }


    float Process(const float sample)
    {
        Write(sample);
        return Read();
    }

    // testing reverse play
    float ProcessBack(const float sample)
    {
        Write(sample);
        return ReadBack();
    }

    void Write(const float sample)
    {
        if(recording_)
        {
            buffer_[write_ptr_] = sample;
            write_ptr_++;
            if(write_ptr_ >= kBufferSize)
            {
                recording_ = false;
                write_ptr_ = 0;
            }
        }
    }

    float Read()
    {
        if(playing_)
        {
            float inc = hw.GetAdcValue(CV_3); 
            float wait = fmap(inc,0.0f,0.99f);

            float out = buffer_[read_ptr_];
            
            for(size_t i = 0.f; i <= wait; i++) {
            
            read_ptr_++;
            
            }


            if(read_ptr_ >= kBufferSize)
            {
                playing_  = false;
                read_ptr_ = 0;
            }
            return out;
        }
        else
        {
            return 0.f;
        }
    }

    // testing reverse play 
    float ReadBack()
    {
        if(playing_)
        {
            float outback = buffer_[read_ptr_back_];
            read_ptr_back_--;
            if(read_ptr_back_ <= 0)
            {
                playing_  = false;
                read_ptr_back_ = kBufferSize;
            }
            return outback;
        }
        else
        {
            return 0.f;
        }
    }

    bool IsPlaying() const { return playing_; }
    bool IsRecording() const { return recording_; }

  private:
    size_t write_ptr_;
    size_t read_ptr_;
    size_t read_ptr_back_;
    bool   playing_;
    bool   recording_;
    float  buffer_[kBufferSize];
};

//sample setup 
#define NUM_BUFFERS 8
// 5 second sample buffers at 48kHz
#define BUFFER_SIZE (48000 * 5)
std::array<SampleBuffer<BUFFER_SIZE>, NUM_BUFFERS> DSY_SDRAM_BSS buffers;
int s = 0;

void getSamples(){
	hw.ProcessAllControls();
	button.Debounce();
    toggle.Debounce(); 
    
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
	getSamples();

    /** Update Params with the four knobs */
    float time_knob     = hw.GetAdcValue(CV_1);
    float time          = fmap(time_knob, 0.3f, 0.99f);

    float damp_knob     = hw.GetAdcValue(CV_2);
    float damp          = fmap(damp_knob, 1000.f, 19000.f, Mapping::LOG);

    float send_level     = hw.GetAdcValue(CV_4);
    
    // moving param sets to inside the buffers - may need to move this? also order? Should probably take in the filter before reverb? 
    rv.SetFeedback(time);
    rv.SetLpFreq(damp);

    svf.SetFreq(24000);
    svf.SetRes(0.5);
    svf.SetDrive(0.8);

	for (size_t i = 0; i < size; i++)
	{
        // Zero samples prior to summing
        OUT_L[i] = 0.f;
        OUT_R[i] = 0.f;
        for(auto &buffer : buffers)
        {
            // Record in mono
            float sample = buffer.Process((0.5 * IN_L[i] + 0.5 * IN_R[i]));

            float send =  sample * send_level;

            svf.Process(sample);

            float wet;

            rv.Process(send, send, &wet, &wet);
            OUT_L[i] += svf.Low() + wet;
            OUT_R[i] += svf.Notch() + wet;
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
    float SR = hw.AudioSampleRate();

    toggle.Init(hw.B8);
    button.Init(hw.B7);
    InitSamplers();
    rv.Init(SR);
    svf.Init(SR);

    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    hw.StartAudio(AudioCallback);

	while(1) {
	}
}
