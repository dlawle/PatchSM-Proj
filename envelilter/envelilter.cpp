#include "daisy_patch_sm.h"
#include "daisysp.h"

using namespace daisy;
using namespace patch_sm;
using namespace daisysp;


DaisyPatchSM patch;
Adsr envelope;
Switch button;
Parameter  cutoff_ctrl, res_ctrl, drive_ctrl;
Svf        svf;

/** Similar to the audio callback, you can generate audio rate CV signals out of the CV outputs. 
 *  These signals are 12-bit DC signals that range from 0-5V out of the Patch SM
*/
void EnvelopeCallback(uint16_t **output, size_t size)
{
    /** Process the controls */
    patch.ProcessAnalogControls();
    button.Debounce();

    /** Set the input value of the ADSR */
    bool env_state;
    if(button.Pressed() || patch.gate_in_1.State())
        env_state = true;
    else
        env_state = false;

    /** Assign four controls to the times and level of the envelope segments 
     *  Attack, Decay, and Release will be set between instantaneous to 1 second
     *  Sustain will be set between 0 and 1 
     */
    float knob_attack = patch.GetAdcValue(CV_1);
    envelope.SetAttackTime(knob_attack);

    float knob_decay = patch.GetAdcValue(CV_2);
    envelope.SetDecayTime(knob_decay);

    float knob_sustain = patch.GetAdcValue(CV_3);
    envelope.SetSustainLevel(knob_sustain);

    float knob_release = patch.GetAdcValue(CV_4);
    envelope.SetReleaseTime(knob_release);

    for(size_t i = 0; i < size; i++)
    {
        // convert to 12-bit integer (0-4095)
        uint16_t value = (envelope.Process(env_state) * 4095.0);
        output[0][i]   = value; /**< To CV OUT 1 - Jack */
        output[1][i]   = value; /**< To CV OUT 2 - LED */
    }
}

    void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
    {
        //get new control values
        float cutoff = cutoff_ctrl.Process();
        float res    = res_ctrl.Process();
        float drive  = drive_ctrl.Process();

        //Set filter to the values we got
        svf.SetFreq(cutoff);
        svf.SetRes(res);
        svf.SetDrive(drive);

        float amplitude;
        for (size_t i = 0; i < size; i++)
        {
            svf.Process(in[0][i]);
            amplitude = envelope.Process(true); // get the next value from the envelope, staying aligned with the bitrate of the samples
            OUT_L[i] = IN_L[i] * amplitude; // apply the value to the left sample
            OUT_R[i] = IN_R[i] * amplitude; // apply the value to the right sample
            OUT_L[i] = svf.Low();
            OUT_R[i] = svf.High();
        }
    }

int main(void)
{
    float samplerate;
    /** Initialize the hardware */
    patch.Init();

    /** Initialize the button input to pin B7 (Button on the MicroPatch Eval board) */
    button.Init(patch.B7, 1000);

    /** Initialize the ADSR */
    envelope.Init(48000);

    /** Configure the DAC to generate audio-rate signals in a callback */
    patch.StartDac(EnvelopeCallback);

    /** configure filters*/
    samplerate = patch.AudioSampleRate();

    //Initialize filter
    svf.Init(samplerate);

    //setup controls
    cutoff_ctrl.Init(patch.controls[CV_5], 20, 20000, Parameter::LOGARITHMIC);
    res_ctrl.Init(patch.controls[CV_6], .3, 1, Parameter::LINEAR);
    drive_ctrl.Init(patch.controls[CV_7], .3, 1, Parameter::LINEAR);

    /** Set and Start Audio*/
    patch.StartAdc();
    patch.SetAudioSampleRate(96000);
    patch.SetAudioBlockSize(4);
    patch.StartAudio(AudioCallback);

    while(1) {}
}
