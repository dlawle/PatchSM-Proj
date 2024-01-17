#include "daisysp.h"
#include "daisy_patch_sm.h"
#include <string>

#define MAX_DELAY static_cast<size_t>(48000 * 1.f)

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;

DaisyPatchSM patch;

DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delMems[3];

struct delay
{
    DelayLine<float, MAX_DELAY> *del;
    float                        currentDelay;
    float                        delayTarget;

    float Process(float feedback, float in)
    {
        //set delay times
        fonepole(currentDelay, delayTarget, .0002f);
        del->SetDelay(currentDelay);

        float read = del->Read();
        del->Write((feedback * read) + in);

        return read;
    }
};

delay     delays[3];
Parameter params[4];

float feedback;
int   drywet;

void ProcessControls();

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    ProcessControls();

    for(size_t i = 0; i < size; i++)
    {
        float mix     = 0;
        float fdrywet = (float)drywet / 100.f;

        //update delayline with feedback
        for(int d = 0; d < 3; d++)
        {
            float sig = delays[d].Process(feedback, IN_L[i]+IN_R[i]);
            mix += sig;
            //out[d][i] = sig * fdrywet + (1.f - fdrywet) * in[0][i];
            OUT_L[i] = sig;
        }

        //apply drywet and attenuate
        mix       = fdrywet * mix * .3f + (1.0f - fdrywet) * (IN_L[i]+IN_R[i]);
        OUT_R[i] = mix;
    }
}

void InitDelays(float samplerate)
{
    for(int i = 0; i < 3; i++)
    {
        //Init delays
        delMems[i].Init();
        delays[i].del = &delMems[i];
        //3 delay times
        params[i].Init(patch.controls[i],
                       samplerate * .05,
                       MAX_DELAY,
                       Parameter::LOGARITHMIC);
    }

    //feedback
    params[3].Init(patch.controls[8], 0, 1, Parameter::LINEAR);
}

int main(void)
{
    float samplerate;
    patch.Init(); // Initialize hardware (daisy seed, and patch)
    samplerate = patch.AudioSampleRate();

    InitDelays(samplerate);

    drywet = 50;

    patch.StartAdc();
    patch.StartAudio(AudioCallback);
    while(1)
    { }
}

void ProcessControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

    //knobs
    for(int i = 0; i < 3; i++)
    {
        delays[i].delayTarget = params[i].Process();
    }
    feedback = params[3].Process();
}
