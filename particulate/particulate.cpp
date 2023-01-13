#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "granular_processor.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
GranularProcessorClouds processor;

// Pre-allocate big blocks in main memory and CCM. No malloc here.
uint8_t block_mem[118784];
uint8_t block_ccm[65536 - 128];

Parameters* parameters;

void Controls();

bool held;
bool freeze_btn;
int  pbMode;
int  quality;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	Controls();

    FloatFrame input[size];
    FloatFrame output[size];
	
	for (size_t i = 0; i < size; i++)
	{
		input[i].l  = in[0][i] * .5f + in[2][i] * .5f;
        input[i].r  = in[1][i] * .5f + in[3][i] * .5f;
        output[i].l = output[i].r = 0.f;
	}

	processor.Process(input, output, size);

    for(size_t i = 0; i < size; i++)
    {
        OUT_L[i] = output[i].l;
        OUT_R[i] = output[i].r;
    }
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	//init the luts
    InitResources(sample_rate);

    processor.Init(sample_rate,
                   block_mem,
                   sizeof(block_mem),
                   block_ccm,
                   sizeof(block_ccm));

    parameters = processor.mutable_parameters();

	hw.StartAudio(AudioCallback);

	while(1) {
		processor.Prepare();

	}
}

void Controls()
{
    hw.ProcessAllControls();

    // gate ins
    parameters->freeze  = hw.gate_in_1.State() || freeze_btn;
    parameters->trigger = hw.gate_in_2.Trig();
    parameters->gate    = hw.gate_in_2.State();
}