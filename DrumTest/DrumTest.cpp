#include "daisysp.h"
#include "daisy_patch_sm.h"
#include "daisy_bed.h"
#include "engine.h"
#include "drum.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM 	hw;
engine			euclid;
drumSet			drums;
CpuLoadMeter	loadMeter;


/* pot lock and drop it */
int modifier = 0; 
bool modmanager;
void modManager();

void syslog(){
    // testing cpuloads
    // get the current load (smoothed value and peak values)
    const float avgLoad = loadMeter.GetAvgCpuLoad();
    const float maxLoad = loadMeter.GetMaxCpuLoad();
    const float minLoad = loadMeter.GetMinCpuLoad();
    // print it to the serial connection (as percentages)
    hw.PrintLine("Processing Load %:");
    hw.PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
    hw.PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
    hw.PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
}

void logInit(){
    hw.StartLog();
    hw.PrintLine("Hello World!");
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	loadMeter.OnBlockStart();
	hw.ProcessAllControls();

	for (size_t i = 0; i < size; i++)
	{
		float sig;

		drums.TriggerKick(euclid.GetTrig1());
		drums.TriggerSnare(euclid.GetTrig2());	
		drums.TriggerHat(euclid.GetTrig3());
		drums.TriggerHat2(euclid.GetTrig4());

		sig = drums.Process();

		OUT_L[i] = sig;
		OUT_R[i] = sig;
	}
	loadMeter.OnBlockEnd();
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	loadMeter.Init(hw.AudioSampleRate(), hw.AudioBlockSize());
    logInit();

	float sample_rate = hw.AudioSampleRate();
	InitBed();
	modmanager = false;
	drums.Init(sample_rate);
	euclid.randSetup();

	hw.StartAudio(AudioCallback);
	while(1) 
	{
		syslog();
		modManager();
		euclid.run();
	}
}

void modManager() 
{
	b1.Debounce();
	b2.Debounce();

	if(b1.RisingEdge())
	{
		modmanager = !modmanager;	
	}

	if(modmanager)
	{
		//you're now in mod manager
	}

	if(b2.RisingEdge() && modmanager)
	{
		euclid.randSetup();
	}

	if(b2.RisingEdge() && !modmanager)
	{
		drums.updateAmp(0.2f);
	}
	gate_in_led1.Write(modmanager);

}
