#pragma once
#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include <stddef.h>

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
            float out = buffer_[read_ptr_];
            read_ptr_++;
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

#endif // SAMPLEBUFFER_H