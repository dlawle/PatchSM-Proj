#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "granular_processor.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 			hw;
GranularProcessorClouds processor;
Parameters* 			parameters;

uint8_t butt_idx = 0;

// Pre-allocate big blocks in main memory and CCM. No malloc here.
uint8_t block_mem[118784];
uint8_t block_ccm[65536 - 128];
char qualityNames[4];

void Controls();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	Controls();

	FloatFrame input[size];
    FloatFrame output[size];

	for(size_t i = 0; i < size; i++)
    {
        input[i].l  = in[0][i] * .5f;
        input[i].r  = in[1][i] * .5f;
        output[i].l = output[i].r = 0.f;
    }

	processor.Process(input, output, size);

	for (size_t i = 0; i < size; i++)
	{
        OUT_L[i] = output[i].l;
        OUT_R[i] = output[i].r;
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(32); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	float sample_rate = hw.AudioSampleRate();

	// init luts
	InitResources(sample_rate);

	InitBed();

	// granular processor init 
	processor.Init(sample_rate, block_mem, sizeof(block_mem), block_ccm, sizeof(block_ccm));
	processor.set_playback_mode(PLAYBACK_MODE_GRANULAR);
	processor.set_quality(GRAIN_QUALITY_HIGH);
	
	// init luts
	InitResources(sample_rate);

	parameters = processor.mutable_parameters();

	hw.StartAudio(AudioCallback);
	while(1) {
		// why does this go in while(1)? 
		processor.Prepare();
	}
}

void Controls()
{
    hw.ProcessAllControls();
	b1.Debounce();
	b2.Debounce();

	// general parameters (knobs)
	parameters->position =  hw.GetAdcValue(CV_1);
	parameters->size =      fmap(hw.GetAdcValue(CV_2), 0.f, 1.f); // needs top end adjusted
						  //fmap(hw.GetAdcValue(CV_2), 0.f, 1.f);???
	parameters->pitch =     powf(9.798f * (hw.GetAdcValue(CV_3) - .5f), 2.f);
	parameters->pitch *= 	hw.GetAdcValue(CV_3) < .5f ? -1.f : 1.f;
	parameters->density =   hw.GetAdcValue(CV_4);
	parameters->texture =   hw.GetAdcValue(ADC_9);
	parameters->dry_wet =   hw.GetAdcValue(ADC_10);
	parameters->feedback =  hw.GetAdcValue(ADC_11);
	parameters->reverb =    hw.GetAdcValue(ADC_12);

	if(b2.FallingEdge()){
		butt_idx = (butt_idx + 1) % 4;
	}

	switch (butt_idx)
	{
	case 0:
		processor.set_playback_mode(PlaybackMode::PLAYBACK_MODE_GRANULAR);
		ledOff();
		gen_led1.Toggle();
		break;
	
	case 1:
		processor.set_playback_mode(PlaybackMode::PLAYBACK_MODE_LOOPING_DELAY);
		ledOff();
		gen_led2.Toggle();
		break;

	case 2:
		processor.set_playback_mode(PlaybackMode::PLAYBACK_MODE_SPECTRAL);
		ledOff();
		cv_led1.Toggle();
		break;

	case 3:
		processor.set_playback_mode(PlaybackMode::PLAYBACK_MODE_STRETCH);
		ledOff();
		cv_led2.Toggle();
		break;

	default:
		processor.set_playback_mode(PlaybackMode::PLAYBACK_MODE_GRANULAR);
		ledOff();
		gen_led1.Toggle();
		break;
	}

	// testing parameters (CV)
	//				hw.GetAdcValue(CV_5);
	//              hw.GetAdcValue(CV_6);
	//             	hw.GetAdcValue(CV_7);
	//    			hw.GetAdcValue(CV_8);
    // gate ins
    parameters->freeze  = b1.RisingEdge();
    parameters->trigger = hw.gate_in_1.Trig();
    parameters->gate    = hw.gate_in_1.State();
}