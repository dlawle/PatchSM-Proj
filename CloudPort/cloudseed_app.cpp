#include "cloudseed_app.h"


#include "../../CloudSeed/Default.h"
#include "../../CloudSeed/ReverbController.h"
#include "../../CloudSeed/FastSin.h"
#include "../../CloudSeed/AudioLib/ValueTables.h"
#include "../../CloudSeed/AudioLib/MathDefs.h"

#include "daisysp.h"
#include "daisy_patch_sm.h"

using namespace daisy;
using namespace daisysp;
CloudSeed::ReverbController* reverb = 0;
  
CloudSeedApp::CloudSeedApp(daisy::patch_sm::DaisyPatchSM& patch)
  :m_patch(patch)
{
}

  
void CloudSeedApp::Init()
{
  AudioLib::ValueTables::Init();
  CloudSeed::FastSin::Init();
  float samplerate = m_patch.AudioSampleRate();
    
  reverb = new CloudSeed::ReverbController(samplerate);
  reverb->ClearBuffers();
    //reverb->initFactoryRubiKaFields();
    //reverb->initFactoryDullEchos();
    //reverb->initFactoryHyperplane();
    //reverb->initFactory90sAreBack();  
}
  
  
float prevCtrlVal[4] = {0};

void CloudSeedApp::AudioTickCallback(float ctrlVal[4], const float * const*in, float **out, size_t size)
{
    prevCtrlVal[0] = ctrlVal[0];
    float delta = 0.01;
    if ((prevCtrlVal[1] < (ctrlVal[1]-delta)) || (prevCtrlVal[1] > (ctrlVal[1]+delta)))
    {
      reverb->SetParameter(::Parameter::MainOut, ctrlVal[1]);
      prevCtrlVal[1] = ctrlVal[1];
    }

    if ((prevCtrlVal[2] < (ctrlVal[2]-delta)) || (prevCtrlVal[2] > (ctrlVal[2]+delta)))
    {
      reverb->SetParameter(::Parameter::LineDecay, ctrlVal[2]);
      prevCtrlVal[2] = ctrlVal[2];
    }
    if ((prevCtrlVal[3] < (ctrlVal[3]-delta)) || (prevCtrlVal[3] > (ctrlVal[3]+delta)))
    {
      reverb->SetParameter(::Parameter::LateDiffusionFeedback, ctrlVal[3]);
      prevCtrlVal[3] = ctrlVal[3];
    }

  float send = 1.0;
  float drylevel = ctrlVal[0];
  float dryL, dryR, sendL, sendR;

  float ins[2*48];
  float outs[2*48];
  for (size_t i = 0; i < size; i++)
  {
      // Read Inputs (only stereo in are used)
      ins[2*i] = in[0][i]*drylevel;
      ins[2*i+1]= in[1][i]*drylevel;
  }
  
  reverb->Process( ins, outs, 48);
  
  for (size_t i = 0; i < size; i++)
  {
      out[0][i] = outs[i*2];
      out[1][i] = outs[i*2+1];
      // Out 3 and 4 are just wet
      out[2][i] = 0;
      out[3][i] = 0;
    }
}

void CloudSeedApp::MainLoopCallback()
{
  
}
