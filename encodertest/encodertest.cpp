#include "daisy_patch_sm.h"
#include "daisysp.h"
#include <hid/encoder.h>

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

/** Hardware object for the patch_sm */
DaisyPatchSM patch;
Encoder myEnc;
static int32_t  inc;


/** Switch object */
//Switch button;

int main(void)
{
    /** Initialize the patch_sm hardware object */
    patch.Init();

    /* Initialize the switch
	 - We'll read the switch on pin B7
	*/
    myEnc.Init(DaisyPatchSM::D8,DaisyPatchSM::A8,DaisyPatchSM::B7); /* pins are currently placeholder values*/
    inc = myEnc.Increment();
    /** Debounce the switch */
    myEnc.Debounce();

    /** Get the current button state */
    if(inc > 0)
    {
      patch.SetLed(true);
    }
    else if(inc < 0)
    {
        patch.SetLed(false);
    }
    /** Set the onboard led to the current state */
    /** loop forever */
    while(1)
      {  }
}
