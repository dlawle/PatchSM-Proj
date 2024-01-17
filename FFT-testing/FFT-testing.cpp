#include "daisy_patch_sm.h"
#include "daisy.h"
#include "daisysp.h"
#include <arm_math.h>

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM hw;

// Define the FFT size and related arrays
#define SAMPLES 1024
#define FFT_SIZE SAMPLES / 2
float32_t inputSignal[SAMPLES];
float32_t outputBuffer[FFT_SIZE * 2]; // Complex output
float32_t magnitudeBuffer[FFT_SIZE];   // Magnitude of complex output
float32_t signalFrequency;

// Perform FFT
arm_cfft_radix4_instance_f32 fftInstance;

// Function to calculate the frequency from an FFT bin index
float32_t CalculateFrequency(uint32_t binIndex, float32_t samplingFrequency) {
    return (float32_t)binIndex * samplingFrequency / (float32_t)FFT_SIZE;
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    hw.ProcessAllControls();
    
    // Copy the input signal for FFT processing
    for (size_t i = 0; i < size; i++)
    {
        inputSignal[i] = IN_L[i]+IN_R[i]; // Assuming mono input, modify if stereo
        //inputSignal[i + size] = in[1][i]; // Assuming stereo input
    }
    
    arm_cfft_radix4_f32(&fftInstance, inputSignal);
    
    // Calculate magnitude of complex output
    arm_cmplx_mag_f32(inputSignal, magnitudeBuffer, FFT_SIZE);
    
    // Find the maximum magnitude and its index
    float32_t maxMagnitude = magnitudeBuffer[0];
    uint32_t maxIndex = 0;
    for (uint32_t i = 1; i < FFT_SIZE; ++i) {
        if (magnitudeBuffer[i] > maxMagnitude) {
            maxMagnitude = magnitudeBuffer[i];
            maxIndex = i;
        }
    }
    
    // Calculate the frequency corresponding to the maximum magnitude index
    float32_t samplingFrequency = static_cast<float32_t>(hw.AudioSampleRate());
    signalFrequency = CalculateFrequency(maxIndex, samplingFrequency);
    
    // Now 'signalFrequency' contains the estimated frequency of the input signal
    
    // Copy the input to the output
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i]; // Assuming mono input/output, modify if stereo
        out[1][i] = in[1][i];
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartLog(true);
	arm_cfft_radix4_init_f32(&fftInstance, FFT_SIZE, 0, 1);
    
    hw.StartAudio(AudioCallback);
    while (1) 
	{ hw.PrintLine("1"); }
}
