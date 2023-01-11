#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
SampleHold	 sh;
Bitcrush   bitcrush;

uint8_t     depth;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float trig, sig;
	hw.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		sig = IN_L[1] + IN_R[1];
		trig = hw.gate_in_1.Trig();
		float   newsample = sh.Process(trig, hw.GetAdcValue(CV_5),sh.MODE_SAMPLE_HOLD);

		if(trig)
			{
				hw.WriteCvOut(CV_OUT_BOTH, newsample);
				depth++;
            	depth %= 6;
			}

		bitcrush.SetBitDepth(depth + 2);
        bitcrush.SetCrushRate((depth + 2) * 2000);
        sig = bitcrush.Process(sig);

		OUT_L[i] = sig;
		OUT_R[i] = IN_R[i] + sig;
	}
}

int main(void)
{
	float sample_rate;
	sample_rate = hw.AudioSampleRate();
	bitcrush.Init(sample_rate);
	bitcrush.SetBitDepth(6);
	bitcrush.SetCrushRate(10000);
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);
	while(1) {}
}
