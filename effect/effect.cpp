#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
Overdrive	 od;
SampleRateReducer sr;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
	float sig = IN_L[0] + IN_R[0];

	float drive = hw.GetAdcValue(CV_1);
	float driveAmt = fclamp(drive, 0, 1);

	float reduction = hw.GetAdcValue(CV_2);
	float sampRed = fclamp(reduction, 0, 1);

	od.SetDrive(driveAmt);
	sr.SetFreq(sampRed);

	for (size_t i = 0; i < size; i++)
	{
		float preDrive = sr.Process(sig);
		float driveOut = od.Process(preDrive);

		OUT_L[i] = driveOut;
		OUT_R[i] = driveOut;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	od.Init();
	sr.Init();

	hw.StartAudio(AudioCallback);
	while(1) {}
}
