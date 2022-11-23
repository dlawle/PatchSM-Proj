#include "daisy_patch_sm.h"
#include "daisysp.h"
#include <string>
#include <hid/encoder.h>
#include "dev/oled_ssd130x.h"

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
using MyOledDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;

// Hardware
DaisyPatchSM hw;
MyOledDisplay display;
Encoder myEnc;

#define NUM_VOICES 32
#define MAX_DELAY ((size_t)(10.0f * 48000.0f))

// Synthesis
PolyPluck<NUM_VOICES> synth;
// 10 second delay line on the external SDRAM
DelayLine<float, MAX_DELAY> DSY_SDRAM_BSS delay;
ReverbSc                                  verb;

// Persistent filtered Value for smooth delay time changes.
float smooth_time;

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig, delsig;           // Mono Audio Vars
    float trig, nn, decay;       // Pluck Vars
    float deltime, delfb, kval;  // Delay Vars
    float dry, send, wetl, wetr; // Effects Vars
    float samplerate;

    // Assign Output Buffers
    float *out_left, *out_right;
    out_left  = out[0];
    out_right = out[1];

    samplerate = hw.AudioSampleRate();
    hw.ProcessDigitalControls();
    hw.ProcessAnalogControls();

    // Handle Triggering the Plucks
    trig = 0.0f;
    if(myEnc.RisingEdge() || hw.gate_in_1.Trig())
        trig = 1.0f;

    // Set MIDI Note for new Pluck notes.
    // nn = 24.0f + hw.GetAdcValue(CV_1) * 60.0f;
    // nn = static_cast<int32_t>(nn); // Quantize to semitones

    float coarse_knob = hw.GetAdcValue(CV_1);
    float coarse      = fmap(coarse_knob, 36.f, 96.f);

    float voct_cv = hw.GetAdcValue(CV_5);
    float voct    = fmap(voct_cv, 0.f, 60.f);

    float midi_nn = fclamp(coarse + voct, 0.f, 127.f);
    nn = mtof(midi_nn);
    nn = static_cast<int32_t>(nn);

    // Read knobs for decay;
    decay = 0.5f + (hw.GetAdcValue(CV_2) * 0.5f);
    synth.SetDecay(decay);

    // Get Delay Parameters from knobs.
    kval    = hw.GetAdcValue(CV_3);
    deltime = (0.001f + (kval * kval) * 5.0f) * samplerate;
    delfb   = hw.GetAdcValue(CV_4);

    // Synthesis.
    for(size_t i = 0; i < size; i++)
    {
        // Smooth delaytime, and set.
        fonepole(smooth_time, deltime, 0.0005f);
        delay.SetDelay(smooth_time);

        // Synthesize Plucks
        sig = synth.Process(trig, nn);

        //		// Handle Delay
        delsig = delay.Read();
        delay.Write(sig + (delsig * delfb));

        // Create Reverb Send
        dry  = sig + delsig;
        send = dry * 0.6f;
        verb.Process(send, send, &wetl, &wetr);

        // Output
        OUT_L[i]  = dry + wetl;
        OUT_R[i] = dry + wetr;
    }
}

int main(void)
{
    // Init everything.
    float samplerate;
    hw.Init();
    samplerate = hw.AudioSampleRate();

    /* Init Encoder (left pin, right pin, click pin)*/
	myEnc.Init(DaisyPatchSM::D8,DaisyPatchSM::A8,DaisyPatchSM::B7);

    /** Configure the Display */
    MyOledDisplay::Config disp_cfg;

    disp_cfg.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_2;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.sclk = hw.GetPin(DaisyPatchSM::PinBank::D, 10);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.mosi = hw.GetPin(DaisyPatchSM::PinBank::D, 9);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.miso = Pin(); // Calling Pin() as it's not needed
    disp_cfg.driver_config.transport_config.spi_config.pin_config.nss  = hw.GetPin(DaisyPatchSM::PinBank::D, 1);
    disp_cfg.driver_config.transport_config.pin_config.dc    = hw.GetPin(DaisyPatchSM::PinBank::A, 2);
    disp_cfg.driver_config.transport_config.pin_config.reset = hw.GetPin(DaisyPatchSM::PinBank::A, 3);

    /** And Initialize */
    display.Init(disp_cfg);
    
    //briefly display the module name
    std::string str  = "Pluck Echo";
    char *      cstr = &str[0];
    display.Fill(false);
    display.SetCursor(0, 0);
    display.WriteString(cstr, Font_7x10, true);
    display.Update();
    hw.Delay(1000);

    synth.Init(samplerate);

    delay.Init();
    delay.SetDelay(samplerate * 0.8f); // half second delay

    verb.Init(samplerate);
    verb.SetFeedback(0.85f);
    verb.SetLpFreq(2000.0f);

    // Start the ADC and Audio Peripherals on the Hardware
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
}
