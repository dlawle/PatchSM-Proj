#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "patch_bay.h"
#include "arm_math.h"
#include "arm_const_structs.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

DaisyPatchSM  hw;
Patch_Bay pb;

int8_t data[128], im[128]; //data and im are used for spectrum , cv is used for oscilo.


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	hw.ProcessAllControls();
    for(size_t i = 0; i < size; i += 2)
    {

		OUT_L[i] = IN_L[i];
		OUT_R[i] = IN_R[i];
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4);
	pb.Init();

    hw.StartAudio(AudioCallback);

	while(1)
	{	
	} 
}