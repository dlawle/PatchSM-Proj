#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "Grain.h"
#include "diffuser.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 	hw;
Diffuser		diffuser;

const int   kCPUReportInterval  = 500;                      // In ms
const int   kBufferDuration     = 10;                       // In seconds
const int   kBufferSize         = 48000 * kBufferDuration;  // Assume 48kHz sample rate

const int kGrainCount = 5;
static graindelay::Grain grains[kGrainCount];
float DSY_SDRAM_BSS grainBuffer[kBufferSize * kGrainCount];

void Controls(){
	// rewrite this? 
	
	float speed, feedback, duration, density, time, amount;
	
	speed = fmap(hw.GetAdcValue(CV_1), 0.0f, 1.0f);
	feedback = fmap(hw.GetAdcValue(CV_2), 0.0f, 1.0f);
	duration = fmap(hw.GetAdcValue(CV_3), 0.0f, 1.0f);
	density = fmap(hw.GetAdcValue(CV_4), 0.0f, 1.0f);
	time = fmap(hw.GetAdcValue(ADC_12), 0.0f, 1.0f);
 	amount = fmap(hw.GetAdcValue(ADC_11), 0.0f, 1.0f);

	for (int i = 0; i < kGrainCount; i++) {
        grains[i].SetSpeed(speed);
		grains[i].SetFeedback(feedback);
		grains[i].SetDuration(duration);
		grains[i].SetGrainDensity(density);
    }
	
	diffuser.SetAmount(amount);
	diffuser.SetTime(time);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	float inputs, grain_process, mix;

	Controls();

	for (size_t i = 0; i < size; i++)
	{
		inputs = IN_L[i] + IN_R[i];

        for (int j = 0; j < kGrainCount; j++)
        {
            grain_process = grains[j].Process(inputs);
        }	
	
		mix = diffuser.Process(grain_process);

		OUT_L[i] = mix;
		OUT_R[i] = mix;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	InitBed();

	// init grains
	for (int i = 0; i < kGrainCount; i++) {
        grains[i].Init(sample_rate, &grainBuffer[kBufferSize * i], kBufferSize);
        grains[i].SetAmp(1.0f);
        grains[i].SetSpeed(1.0f);
    }

	// init diffuser
	diffuser.Init();

	hw.StartAudio(AudioCallback);
	while(1) {}
}
