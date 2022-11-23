#include "daisysp.h"
#include "daisy_patch_sm.h"
#include "envelope.h"
#include "oscillator.h"
#include <stdio.h>
#include <string.h>
#include "dev/oled_ssd130x.h"
#include <hid/encoder.h>

using namespace daisy;
using namespace daisysp;
using namespace patch_sm;
using MyOledDisplay = OledDisplay<SSD130x4WireSpi128x64Driver>;

DaisyPatchSM patch;
MyOledDisplay display;
Encoder myEnc;

float      sampleRate;

constexpr uint8_t NUM_CV_IN   = 4;
constexpr uint8_t NUM_GATE_IN = 2;

constexpr float C1_FREQ = 32.70f;

float ctrl[NUM_CV_IN];
bool  gate[NUM_GATE_IN];

EnvelopeOscillator::Envelope   env;
EnvelopeOscillator::Oscillator osc;

inline float static CalcFreq(const float value)
{
    return powf(2.f, value * 5.f) * C1_FREQ; //Hz
}

void UpdateControls()
{
    patch.ProcessAnalogControls();
    patch.ProcessDigitalControls();

//  for(uint8_t n = 0; n < NUM_CV_IN; n++)
//  {
//      ctrl[n] = patch.GetAdcValue(patch_sm::CV_(n));
//  }

//  for(uint8_t n = 0; n < NUM_GATE_IN; n++)
//  {
//      gate[n] = patch.gate_in_[n].Trig();
//  }


    ctrl[0] = patch.GetAdcValue(CV_1);
    ctrl[1] = patch.GetAdcValue(CV_2);
    ctrl[2] = patch.GetAdcValue(CV_3);
    ctrl[3] = patch.GetAdcValue(CV_4);

    gate[0] = patch.gate_in_1.Trig();
    gate[1] = patch.gate_in_2.Trig();

    osc.SetFreq(CalcFreq(ctrl[0]));
    osc.SetMorph(ctrl[1]);
    env.SetRise(ctrl[2]);
    env.SetFall(ctrl[3]);

    if(gate[0] || myEnc.FallingEdge())
    {
        env.Trigger();
    }
}

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    UpdateControls();

    for(size_t n = 0; n < size; n++)
    {
        out[0][n] = env.Process() * osc.Process();
        out[1][n] = 0.f;
        out[2][n] = 0.f;
        out[3][n] = 0.f;
    }

    patch.dac.WriteValue(DacHandle::Channel::BOTH,
                              static_cast<uint16_t>(4095 * env.GetCurrValue()));
}

void UpdateDisplay()
{
    uint32_t minX       = 0;
    uint32_t minY       = 0;
    uint32_t lineOffset = 8;

    display.Fill(false);

    std::string str  = "ENVELOPE OSCILLATOR";
    char*       cstr = &str[0];
    display.SetCursor(minX, minY);
    display.WriteString(cstr, Font_6x8, true);

    str = "FREQ:" + std::to_string(static_cast<uint32_t>(osc.GetFreq()));
    display.SetCursor(minX, lineOffset + minY);
    display.WriteString(cstr, Font_6x8, true);

    str = "MRPH:" + std::to_string(static_cast<uint8_t>(100 * osc.GetMorph()));
    display.SetCursor(minX, 2 * lineOffset + minY);
    display.WriteString(cstr, Font_6x8, true);

    str = "RISE:" + std::to_string(static_cast<uint32_t>(1000 * env.GetRise()));
    display.SetCursor(minX, 3 * lineOffset + minY);
    display.WriteString(cstr, Font_6x8, true);

    str = "FALL:" + std::to_string(static_cast<uint32_t>(1000 * env.GetFall()));
    display.SetCursor(minX, 4 * lineOffset + minY);
    display.WriteString(cstr, Font_6x8, true);

    str = "ENV:"
          + std::to_string(static_cast<uint8_t>(100 * env.GetCurrValue()));
    display.SetCursor(minX, 5 * lineOffset + minY);
    display.WriteString(cstr, Font_6x8, true);

    display.Update();
}

int main(void)
{
    patch.Init();
    sampleRate = patch.AudioSampleRate();

    /* Init Encoder (left pin, right pin, click pin)*/
	myEnc.Init(DaisyPatchSM::D8,DaisyPatchSM::A8,DaisyPatchSM::B7); /* pins are currently placeholder values*/

    /** Configure the Display */
    MyOledDisplay::Config disp_cfg;

    disp_cfg.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_2;
    disp_cfg.driver_config.transport_config.spi_config.pin_config.sclk = patch.GetPin(DaisyPatchSM::PinBank::D, 10);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.mosi = patch.GetPin(DaisyPatchSM::PinBank::D, 9);
    disp_cfg.driver_config.transport_config.spi_config.pin_config.nss  = patch.GetPin(DaisyPatchSM::PinBank::D, 1);
    disp_cfg.driver_config.transport_config.pin_config.dc    = patch.GetPin(DaisyPatchSM::PinBank::A, 2);
    disp_cfg.driver_config.transport_config.pin_config.reset = patch.GetPin(DaisyPatchSM::PinBank::A, 3);
    /** And Initialize */
    display.Init(disp_cfg);

    env.Init(sampleRate);
    osc.Init(sampleRate);

    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    while(1)
    {
        UpdateDisplay();
    }
}
