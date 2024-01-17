// Using tips learned from radicalTeapot at https://github.com/RadicalTeapot/DaisyGrains/blob/main/Grain.h
// this class will mostly be used as a "circular buffer" to run a constaint capture loop on the input 

#pragma once 
#include "daisy.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

template <size_t BufferSize>
class InterestingProcessor {
public:

void Init(float samplerate)
{
    sample_rate_ = samplerate;
    buffer_size_ = BufferSize;
    write_index_ = 0;
    read_position_ = 0;
    speed_ = 1;
    freeze_ = false;
    for (size_t i = 0; i < BufferSize; i++) 
    {
        buffer_[i] = 0;
        playback_buffer_[0][i] = 0;
        playback_buffer_[1][i] = 0;
        playback_buffer_[2][i] = 0;
        playback_buffer_[3][i] = 0;
    }

    for (size_t i = 0; i < 4; i++)
    {
        playback_position_[i] = 0;
        playback_ready_[i] = false;
        playback_speed_[i] = 1;
        env_[i].Init(sample_rate_);
        env_[i].SetTime(ADENV_SEG_ATTACK, 0.15);
        env_[i].SetTime(ADENV_SEG_DECAY, 0.35);
        env_[i].SetMin(0.0);
        env_[i].SetMax(0.25);
        env_[i].SetCurve(0); // linear
    }
    selected_buffer_ = 0;
    dc_.Init(sample_rate_);
    reverse_ = false;
}

void Process(const float in)
{
    buffer_[write_index_] = in;
    ++write_index_;
    write_index_ %= buffer_size_;
}

float PlaybackSample(size_t index)
{
    float output = 0.0f;
    if (playback_ready_[index])
    {
        env_[index].Trigger();
        float envelope_factor = env_[index].Process();
        output += readHermite(playback_position_[index], index) * envelope_factor;
        playback_position_[index] += playback_speed_[index] * floor(static_cast<float>(buffer_size_) / sample_rate_);
    }
    if (playback_position_[index] >= buffer_size_)
    {
        ResetBuffer(index);
    }
    dc_.Process(output);
    return output;
}

void CaptureForward()
{
    playback_ready_[selected_buffer_] = false;
    ClearBuffer(playback_buffer_[selected_buffer_], buffer_size_);
    memcpy(playback_buffer_[selected_buffer_], buffer_, buffer_size_ * sizeof(float)); 
    read_position_ = 0;
    playback_ready_[selected_buffer_] = true;
    ++selected_buffer_;
    if(selected_buffer_ >= 4) {selected_buffer_ = 0;}
}


void CaptureBackward()
{
    playback_ready_[selected_buffer_] = false;
    ClearBuffer(playback_buffer_[selected_buffer_], buffer_size_);
    size_t reverse_copy_index = buffer_size_ - 1;
    for (size_t i = 0; i < buffer_size_; ++i)
    {
        playback_buffer_[selected_buffer_][i] = buffer_[reverse_copy_index];
        --reverse_copy_index;
    }
    read_position_ = buffer_size_ - 1;
    playback_ready_[selected_buffer_] = true;
    ++selected_buffer_;
    if (selected_buffer_ >= 4) { selected_buffer_ = 0; }
}

void Generate()
{
    if(reverse_) { CaptureBackward(); }
    else { CaptureForward(); }
}

void ClearBuffer(float* buffer, size_t bufferSize)
{
    for (size_t i = 0; i < bufferSize; i++) {
        buffer[i] = 0.0f;
    }
}

void ResetBuffer(int bufferIndex)
{
    ClearBuffer(playback_buffer_[bufferIndex], buffer_size_);
    playback_ready_[bufferIndex] = false;
    playback_position_[bufferIndex] = 0;
}


bool GetState(int buffer) { return playback_ready_[buffer]; }

void FlipDirection() { reverse_ = !reverse_; }
bool returnState() { return reverse_; }

inline float readHermite(float position, int bufferIndex)
{
    const int32_t t = static_cast<int32_t>(position);
    const float f = position - static_cast<float>(t);

    const float xm1 = playback_buffer_[bufferIndex][(t - 1) % buffer_size_];
    const float x0 = playback_buffer_[bufferIndex][(t) % buffer_size_];
    const float x1 = playback_buffer_[bufferIndex][(t + 1) % buffer_size_];
    const float x2 = playback_buffer_[bufferIndex][(t + 2) % buffer_size_];

    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;

    return (((a * f) - b_neg) * f + c) * f + x0;
}



void SetSpeed(const float s1, const float s2, const float s3, const float s4) 
{
    playback_speed_[0] = s1;
    playback_speed_[1] = s2;
    playback_speed_[2] = s3;
    playback_speed_[3] = s4;
}


void FreezePlayback(size_t index)
{
    // Calculate the loop window boundaries
    size_t loopStart = playback_position_[index];
    size_t loopEnd = playback_position_[index] + window_;
    for (size_t i = 0; i < 4; i++)
    {
        if(playback_position_[i] >= loopEnd) { playback_position_[i] = loopStart; }
    }
}

void SetWindow(float window) { window_ = window; }

size_t GetPos() { return playback_position_[selected_buffer_]; }

void SetFreeze() { freeze_ = !freeze_; }

bool Freeze() { return freeze_; }

/*================================================================================*/
/*                                   test area                                    */
/*================================================================================*/

void ProcessTimeStretching(float *sample, size_t sampleSize, float stretchFactor, float *output, size_t FRAME_SIZE) 
{
    size_t hopSize = static_cast<size_t>(FRAME_SIZE * stretchFactor); // Adjust hop size accordingly
    float timePointer = 0.0f;
    float phase = 0.0f;

    for (size_t i = 0; i < FRAME_SIZE; ++i) {
        // Perform time-stretching processing for each output sample
        // ...
        // Calculate input sample indices
        size_t prevIndex = static_cast<size_t>(timePointer);
        size_t nextIndex = prevIndex + 1;

        // Interpolation factor between samples
        float fraction = timePointer - prevIndex;

        // Perform linear interpolation between samples
        float interpolatedSample = (1.0f - fraction) * sample[prevIndex] + fraction * sample[nextIndex];

        // Store the interpolated sample in the output buffer
        output[i] = interpolatedSample;

        // Update time pointer and phase for the next iteration
        timePointer += stretchFactor;
        phase += hopSize;

        // Wrap time pointer and phase
        if (timePointer >= sampleSize) {
            timePointer -= sampleSize;
        }
        if (phase >= sampleSize) {
            phase -= sampleSize;
        }
    }
}


/*================================================================================*/
/*                                   test area                                    */
/*================================================================================*/
private:
    // test
    // for main buffer
    float buffer_[BufferSize];

    size_t  buffer_size_, 
            write_index_;
    
    float   sample_rate_,
            speed_,
            read_position_;

    bool freeze_; 
    float window_;

    // for playback buffers
    int     selected_buffer_;
    float   playback_buffer_[4][BufferSize];
    float   playback_speed_[4];
    float   playback_position_[4];
    bool    playback_ready_[4];
    bool    capture_finished_;
    bool    reverse_;
    AdEnv   env_[4];
    DcBlock dc_;

};