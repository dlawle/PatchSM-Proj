#include "patch_bay.h"

using namespace daisy;
using namespace patch_sm;

void Patch_Bay::InitDisplay()
{
    /** OLED display configuration */
	OledDisplay<SSD130xI2c128x64Driver>::Config display_config;
    display_config.driver_config.transport_config.i2c_address = 0x3C;
	display_config.driver_config.transport_config.i2c_config.periph = I2CHandle::Config::Peripheral::I2C_1;
	display_config.driver_config.transport_config.i2c_config.speed  = I2CHandle::Config::Speed::I2C_100KHZ;
	display_config.driver_config.transport_config.i2c_config.mode = I2CHandle::Config::Mode::I2C_MASTER;
 	display_config.driver_config.transport_config.i2c_config.pin_config.scl = {DSY_GPIOB, 6};
	display_config.driver_config.transport_config.i2c_config.pin_config.sda = {DSY_GPIOB, 7};

	//display_config.driver_config.transport_config.Defaults();
	display.Init(display_config);
};

void Patch_Bay::InitButtons()
{
    shift.Init(DaisyPatchSM::D10);
};

void Patch_Bay::InitEncoder()
{
    enc.Init(DaisyPatchSM::D1,DaisyPatchSM::A9,DaisyPatchSM::A8);
};

void Patch_Bay::InitSD()
{
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd.Init(sd_cfg);
};

void Patch_Bay::InitLeds()
{
    cv_ind.Init(DaisyPatchSM::C10,false);
    gate_1.Init(DaisyPatchSM::B5, false);
    gate_2.Init(DaisyPatchSM::B6, false);
}

void Patch_Bay::Init()
{
    Patch_Bay::InitButtons();
    Patch_Bay::InitDisplay();
    Patch_Bay::InitEncoder();
    Patch_Bay::InitSD();
};