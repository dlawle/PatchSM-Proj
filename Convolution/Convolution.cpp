#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "customLooper.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

#define kBuffSize 48000 * 60 // 60 seconds at 48kHz
float DSY_SDRAM_BSS buffer[kBuffSize];

DaisyPatchSM 			hw;
cLooper					loop;
ReverbSc DSY_SDRAM_BSS	rev;

void loopControl(){
    b2.Debounce();
	b1.Debounce();

	if(hw.gate_in_1.Trig()||b1.RisingEdge()) {
        loop.TrigRecord();
    }

	if(b2.TimeHeldMs() >= 1000.f) {
        loop.Clear();
    }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	loopControl();
	float in_level, loop_level, rev_feedback, rev_lpf, speed_level;

	in_level = hw.GetAdcValue(CV_4);
	loop_level = hw.GetAdcValue(ADC_12);

	rev_feedback = fmap(hw.GetAdcValue(CV_3), 0.3f, 0.99f);
	rev_lpf = fmap(hw.GetAdcValue(ADC_11), 1000.f, 19000.f, Mapping::LOG);

	rev.SetFeedback(rev_feedback);
	rev.SetLpFreq(rev_lpf);

	speed_level = fmap(hw.GetAdcValue(CV_1), 0.0f, 2.0f);
	loop.setSpeed(speed_level);
	
	for (size_t i = 0; i < size; i++)
	{
		float inputs = (IN_L[i] + IN_R[i]) * in_level;
		float sig = loop.Process(inputs) * loop_level + inputs;

		float outs;
		rev.Process(sig,sig,&outs,&outs);

		OUT_L[i] = outs;
		OUT_R[i] = outs;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(64); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	InitBed();
	loop.Init(buffer, kBuffSize);
	rev.Init(sample_rate);

	hw.StartAudio(AudioCallback);
	while(1) {
		gate_in_led1.Write(loop.Recording());

		std::pair<bool, bool> start_and_halfway = loop.gateLoop(); 
		gate_out_led1.Write(start_and_halfway.first);  // first loop
		dsy_gpio_write(&hw.gate_out_1, start_and_halfway.first);
		gate_out_led2.Write(start_and_halfway.second);  // half loop
		dsy_gpio_write(&hw.gate_out_2, start_and_halfway.second);
	}
}
