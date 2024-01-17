#include "daisy_patch_sm.h"
#include "processors.h"
#include "daisy.h"
#include "daisysp.h"
#include "daisy_bed.h"

using namespace daisy;
using namespace patch_sm;

using namespace peaks;
using namespace stmlib;

DaisyPatchSM hw;
const uint32_t kSampleRate = 48000;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	for (size_t i = 0; i < size; i++)
	{
		OUT_L[i] = IN_L[i];
		OUT_R[i] = IN_R[i];
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	processors[0].Init(1);

	hw.StartAudio(AudioCallback);
	while(1) {}
}
