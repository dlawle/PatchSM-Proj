#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "Grain.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

const float kOnePoleCvCoeff     = 0.5f;                     // time = 1 / (sampleRate * coeff) so ~0.05s at 40Hz (1/kDelayLoopLength)
const int   kBufferDuration     = 15;                       // In seconds
const int   kBufferSize         = 48000 * kBufferDuration;  // Assume 48kHz sample rate
const int kGrainCount = 10;

static graindelay::Grain 	grains[kGrainCount];
float DSY_SDRAM_BSS 		grainBuffer[kBufferSize * kGrainCount];
DaisyPatchSM 				hw;
ReverbSc DSY_SDRAM_BSS		rev;

void updateControls() {
	float grainSize, grainSpeed, grainDensity, grainFeedback, revFeedback, lpFreq;
	grainSize = fmap(hw.GetAdcValue(CV_1), 0.f, kBufferDuration);
	grainSpeed = fmap(hw.GetAdcValue(CV_2), 0.f, 1.f);
	grainDensity = fmap(hw.GetAdcValue(CV_3), 0.f, 1.f);
	grainFeedback = fmap(hw.GetAdcValue(CV_4), 0.f, 1.f);
	revFeedback = fmap(hw.GetAdcValue(ADC_10), 0.f, .99f);
	lpFreq = fmap(hw.GetAdcValue(ADC_11), 1000.f, 19000.f, Mapping::LOG);
	
	for (size_t i = 0; i < kGrainCount; i++){
		grains[i].SetDuration(grainSize);
		grains[i].SetSpeed(grainSpeed);
		grains[i].SetGrainDensity(grainDensity);
		grains[i].SetFeedback(grainFeedback);
	}

	rev.SetFeedback(revFeedback);
	rev.SetLpFreq(lpFreq);
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	updateControls();
	for (size_t i = 0; i < size; i++)
	{
		float inputs = IN_L[i] + IN_R[i];
		float voicesMix[2], out1, out2;
        voicesMix[0] = 0;
        voicesMix[1] = 0;
		
        for (size_t j = 0; j < kGrainCount; j++)
        {
            float grainProcess = grains[j].Process(inputs);
            voicesMix[0] += grainProcess * (grains[j].GetPan() * -0.5 + 0.5);
            voicesMix[1] += grainProcess * (grains[j].GetPan() * 0.5 + 0.5);
        }

		rev.Process(voicesMix[0], voicesMix[1], &out1, &out2);

		OUT_L[i] = out1;
		OUT_R[i] = out2 + (inputs * 0.5f);
	}
}


void InitGrains(const float sampleRate)
{
    const float mixValues[kGrainCount] = {0.25f, 0.2f, 0.1f, 0.3f, 0.15f};         // Sum to 1
    for (int i = 0; i < kGrainCount; i++) {
        grains[i].Init(sampleRate, &grainBuffer[kBufferSize * i], kBufferSize);
        grains[i].SetAmp(mixValues[i] * 1.66f);
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	InitBed();
	InitGrains(sample_rate);
	rev.Init(sample_rate);
	
	hw.StartAudio(AudioCallback);
	while(1) {}
}
