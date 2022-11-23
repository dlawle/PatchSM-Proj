#pragma once

#include "daisy.h"
#include "CustomPages.h"
#include "daisy_patch_sm.h"

#define PIN_OLED_DC A2
#define PIN_OLED_RESET A3

namespace daisy
{
void ClearDisplay(const daisy::UiCanvasDescriptor& canvas)
{
    display_.Fill(false);
}

void FlushDisplay(const daisy::UiCanvasDescriptor& canvas)
{
    display_.Update();
}

class TorusPatchUI : public UI
{
  public:
    ~TorusPatchUI(){};

    void InitDisplay(DaisyPatch* hw)
    {
        //there should be a way to set OledDisplay::driver_, instead we just reinit
        OledDisplay<SSD130x4WireSpi128x64Driver>::Config display_config;
        display_config.driver_config.transport_config.pin_config.dc
            = hw.GetPin(PIN_OLED_DC);
        display_config.driver_config.transport_config.pin_config.reset
            = hw.GetPin(PIN_OLED_RESET);
        display_config.driver_config.transport_config.spi_config.periph = SpiHandle::Config::Peripheral::SPI_2;
        display_config.driver_config.transport_config.spi_config.pin_config.sclk = hw.GetPin(DaisyPatchSM::PinBank::D, 10);
        display_config.driver_config.transport_config.spi_config.pin_config.mosi = hw.GetPin(DaisyPatchSM::PinBank::D, 9);
        display_config.driver_config.transport_config.spi_config.pin_config.nss  = hw.GetPin(DaisyPatchSM::PinBank::D, 1);

        display_.Init(display_config);

        desc_.handle_        = &hw->display;
        desc_.clearFunction_ = ClearDisplay;
        desc_.flushFunction_ = FlushDisplay;
    }

    void DoEvents()
    {
        // menu_one_.Draw(desc_);
        Rectangle rect;
        menu_one_.Draw(display_, false, false, rect, false);
    }

    void GenerateEvents() {}

  private:
    UiCanvasDescriptor                                    desc_;
    MenuOne                                               menu_one_;
    OledDisplay<OledDisplay<SSD130x4WireSpi128x64Driver>> display_;
};

} // namespace daisy