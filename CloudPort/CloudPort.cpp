#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "daisy_bed.h"
#include "cloudseed_app.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

DaisyPatchSM hw;
::daisy::Parameter lpParam;

CloudSeedApp* gApp = 0;
CloudSeedApp reverb_app(hw);

bool gRisingEdge = false;
bool gAudioStarted = false;


#define CUSTOM_POOL_SIZE (48*1024*1024)
DSY_SDRAM_BSS char custom_pool[CUSTOM_POOL_SIZE];
size_t pool_index = 0;
int allocation_count = 0;
void* custom_pool_allocate(size_t size)
{
        if (pool_index + size >= CUSTOM_POOL_SIZE)
        {
                return 0;
        }
        void* ptr = &custom_pool[pool_index];
        pool_index += size;
        return ptr;
}


float ctrlVal[4]={1,1,1,1};


void updateControls()
{
   hw.ProcessAnalogControls();
   hw.ProcessDigitalControls();

   //the encoders are not precise and stop above 0 or below 1, 
   // so add some deadzone around 0 and 1
   for (int i = 0; i < 4; i++)
   {
        //Get the four control values
        ctrlVal[i] = hw.controls[i].Process();
        if (ctrlVal[i]<0.01)
           ctrlVal[i] = 0;
        else
          {
            ctrlVal[i]-= 0.01;
          }
        ctrlVal[i]/=0.95;

        if (ctrlVal[i]>1.)
           ctrlVal[i]=1.;
    }
   
}

static void BouquetCallback(const float * const*in, float **out, unsigned int size)
{
  
     // read some controls
    updateControls();
    
    if (gApp)
    {
      gApp->AudioTickCallback(ctrlVal, in,out,size); 
    } else
    {
      for (size_t i = 0; i < size; i++)
      {
          out[0][i] = 0.;
          out[1][i] = 0.;
      }
    }
    
}



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
    lpParam.Init(hw.controls[3], 20, 20000, ::daisy::Parameter::LOGARITHMIC);
    hw.StartAdc();
    
    
    gAudioStarted = true;
    reverb_app.Init();
    gApp = &reverb_app;
    hw.StartAudio(BouquetCallback);

	hw.SetAudioBlockSize(4); // number of samples handled per callback

	InitBed();

	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	while(1) 
	{
		hw.Delay(1);

		if (!gAudioStarted)
		{
			updateControls();
		}
	
		if (gApp)
		{
			gApp->MainLoopCallback(); 
		}
	
		
		//turn off display after 3 seconds hold
		if (gRisingEdge)
		{
			
			gRisingEdge = false;
		}
	}
}
