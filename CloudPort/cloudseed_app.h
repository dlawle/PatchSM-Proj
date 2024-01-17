#include "daisysp.h"
#include "daisy_patch_sm.h"
#include <string>

class CloudSeedApp
{
  daisy::patch_sm::DaisyPatchSM& m_patch;
  public:
    
  CloudSeedApp(daisy::patch_sm::DaisyPatchSM& patch);
  
  
  void Init();
  
  void AudioTickCallback(float ctrlVal[4], const float * const*in, float **out, size_t size);
  
  void MainLoopCallback();
  
  
};
