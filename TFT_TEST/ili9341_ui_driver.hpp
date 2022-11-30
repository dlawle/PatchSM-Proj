#pragma once

#include <cstring>

#include "daisy_seed.h"
#include "daisy_patch_sm.h"
#include "util/oled_fonts.h"
#include "hid/disp/graphics_common.h"

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM    psm;

// #define SPI1_NSS 7    // LCD CS 3
// #define SPI1_SCK 8    // LCD SCK 7
// #define SPI1_MOSI 10  // LCD MOSI 6
// #define ADC1_INP15 16 // PIN 23 - LCD RST 4
// #define DAC1_OUT2 22  //PIN 29 - LCD DC 5

// #define SPI1_MISO 9 // Not used
// #define I2C1_SCL 11
// #define I2C1_SDA 12

enum TFT_COLOR
{
    COLOR_BLACK = 0,
    COLOR_WHITE,
    COLOR_BLUE,
    COLOR_DARK_BLUE,
    COLOR_CYAN,
    COLOR_YELLOW,
    COLOR_DARK_YELLOW,
    COLOR_ORANGE,
    COLOR_RED,
    COLOR_DARK_RED,
    COLOR_GREEN,
    COLOR_DARK_GREEN,
    COLOR_LIGHT_GREEN,
    COLOR_GRAY,
    COLOR_DARK_GRAY,
    COLOR_LIGHT_GRAY,
    COLOR_MEDIUM_GRAY,
    COLOR_ABL_BG,
    COLOR_ABL_LINE,
    COLOR_ABL_D_LINE,
    COLOR_ABL_L_GRAY,
    COLOR_ABL_M_GRAY,
    NUMBER_OF_TFT_COLORS
};

/**
 * SPI Transport for ILI9341 TFT display devices
 */
class ILI9341SpiTransport
{
  public:
    void Init()
    {
        // Initialize SPI
        SpiHandle::Config spi_config;
        spi_config.periph    = SpiHandle::Config::Peripheral::SPI_2;
        spi_config.mode      = SpiHandle::Config::Mode::MASTER;
        spi_config.direction = SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
        spi_config.clock_polarity  = SpiHandle::Config::ClockPolarity::LOW;
        spi_config.baud_prescaler  = SpiHandle::Config::BaudPrescaler::PS_2;
        spi_config.clock_phase     = SpiHandle::Config::ClockPhase::ONE_EDGE;
        spi_config.nss             = SpiHandle::Config::NSS::HARD_OUTPUT;
        spi_config.datasize        = 8;
        spi_config.pin_config.sclk = psm.GetPin(DaisyPatchSM::PinBank::D, 10);
        spi_config.pin_config.mosi = psm.GetPin(DaisyPatchSM::PinBank::D, 9);
        spi_config.pin_config.nss  = psm.GetPin(DaisyPatchSM::PinBank::D, 1);
        // spi_config.pin_config.miso = {DSY_GPIOX, 0}; // not used

        // v0.1 mix up
        //auto dc_pin    = psm.GetPin(DaisyPatchSM::PinBank::A, 2);
        //auto reset_pin = psm.GetPin(DaisyPatchSM::PinBank::A, 3);
        // auto dc_pin    = 22;
        // auto reset_pin = 16;


        // DC pin
        pin_dc_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_dc_.pin  = psm.GetPin(DaisyPatchSM::PinBank::A, 2);
        dsy_gpio_init(&pin_dc_);
        // Reset pin
        pin_reset_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_reset_.pin  = psm.GetPin(DaisyPatchSM::PinBank::A, 3);
        dsy_gpio_init(&pin_reset_);
        // CS pin
        pin_cs_.mode = DSY_GPIO_MODE_OUTPUT_PP;
        pin_cs_.pin  = spi_config.pin_config.nss;
        dsy_gpio_init(&pin_cs_);

        spi_.Init(spi_config);
    };

    void Reset()
    {
        dsy_gpio_write(&pin_reset_, 0);
        System::Delay(100);
        dsy_gpio_write(&pin_reset_, 1);
        System::Delay(100);
    }

    // an internal function to handle SPI DMA callbacks
    // called when an DMA transmission completes and the next driver must be updated
    static void TxCompleteCallback(void* context, SpiHandle::Result result)
    {
        if(result == SpiHandle::Result::OK)
        {
            auto transport     = static_cast<ILI9341SpiTransport*>(context);
            auto transfer_size = transport->GetTransferSize();
            transport->remaining_buff -= transfer_size;
            if(transport->remaining_buff > 0)
            {
                transport->SendDataDMA(
                    &frame_buffer[buffer_size - transport->remaining_buff],
                    transfer_size);

                // 16bit transfer
                // transport->SendDataDMA(
                //     &frame_buffer[2 * transport->buf_chunk_size],
                //     transport->buf_chunk_size);
            }
            else
            {
                transport->dma_busy = false;

                // 8 bit transfer
                // auto* spi_h = transport->spi_.GetHandle();
                // spi_h->Instance->CFG1 &= ~SPI_CFG1_DSIZE_3;
                // spi_h->Instance->CFG2 &= ~SPI_CFG2_LSBFRST;
                // spi_h->Init.DataSize = SPI_DATASIZE_8BIT;
            }
        }
    }

    SpiHandle::Result SendDataDMA()
    {
        remaining_buff = buffer_size;
        dma_busy       = true;

        // auto* spi_h = spi_.GetHandle();
        // spi_h->Instance->CFG1 |= SPI_CFG1_DSIZE_3;
        // spi_h->Instance->CFG2 |= SPI_CFG2_LSBFRST;
        // spi_h->Init.DataSize = SPI_DATASIZE_16BIT; // 15;

        return SendDataDMA(frame_buffer, buf_chunk_size);
    };

    SpiHandle::Result SendDataDMA(uint8_t* buff, size_t size)
    {
        dsy_gpio_write(&pin_dc_, 1);
        return spi_.DmaTransmit(buff, size, nullptr, &TxCompleteCallback, this);
    };

    uint32_t GetTransferSize() const
    {
        return remaining_buff < buf_chunk_size ? remaining_buff
                                               : buf_chunk_size;
        // return remaining_buff < 2 * buf_chunk_size ? remaining_buff
        //                                            : 2 * buf_chunk_size;
    }

    SpiHandle::Result SendCommand(uint8_t cmd)
    {
        dsy_gpio_write(&pin_dc_, 0);
        return spi_.BlockingTransmit(&cmd, 1);
    };

    SpiHandle::Result SendData(uint8_t* buff, size_t size)
    {
        dsy_gpio_write(&pin_dc_, 1);
        return spi_.BlockingTransmit(buff, size);
    };

    void SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
    {
        // column address set
        SendCommand(0x2A); // CASET
        {
            uint8_t data[4] = {static_cast<uint8_t>((x0 >> 8) & 0xFF),
                               static_cast<uint8_t>(x0 & 0xFF),
                               static_cast<uint8_t>((x1 >> 8) & 0xFF),
                               static_cast<uint8_t>(x1 & 0xFF)};
            SendData(data, 4);
        }

        // row address set
        SendCommand(0x2B); // RASET
        {
            uint8_t data[4] = {static_cast<uint8_t>((y0 >> 8) & 0xFF),
                               static_cast<uint8_t>(y0 & 0xFF),
                               static_cast<uint8_t>((y1 >> 8) & 0xFF),
                               static_cast<uint8_t>(y1 & 0xFF)};
            SendData(data, 4);
        }

        // write to RAM
        SendCommand(0x2C); // RAMWR
    }

    void PaintPixel(uint32_t id, uint8_t color_id, uint8_t alpha = 255) const
    {
        auto color = tftPalette[color_id];

        if(alpha != 255)
        {
            auto bg_color = tftPalette[color_mem[id]];
            color         = Blend565(color, bg_color, alpha);
        }

        color_mem[id] = color_id;

        frame_buffer[id]     = color >> 8;
        frame_buffer[id + 1] = color & 0xFF;
    }

    uint16_t GetPixel(uint32_t id) { return color_mem[id]; }

    bool     dma_busy       = false;
    uint32_t remaining_buff = 0;

    static uint32_t const buffer_size    = 153600;          // 320 * 240 * 2
    const uint16_t        buf_chunk_size = buffer_size / 3; // 8bit data
    // const uint16_t buf_chunk_size = buffer_size / 4; // 16bit data

    static uint8_t DMA_BUFFER_MEM_SECTION frame_buffer[buffer_size];
    static uint8_t DSY_SDRAM_BSS          color_mem[buffer_size / 2];
    SpiHandle                             spi_;

    uint16_t tftPalette[NUMBER_OF_TFT_COLORS];

  private:
    dsy_gpio pin_reset_;
    dsy_gpio pin_dc_;
    dsy_gpio pin_cs_;


    //   rrrrrggggggbbbbb
#define MASK_RB 63519       // 0b1111100000011111
#define MASK_G 2016         // 0b0000011111100000
#define MASK_MUL_RB 4065216 // 0b1111100000011111000000
#define MASK_MUL_G 129024   // 0b0000011111100000000000
#define MAX_ALPHA 64        // 6bits+1 with rounding

    uint16_t Blend565(uint16_t fg, uint16_t bg, uint8_t alpha) const
    {
        // alpha for foreground multiplication
        // convert from 8bit to (6bit+1) with rounding
        // will be in [0..64] inclusive
        alpha = (alpha + 2) >> 2;
        // "beta" for background multiplication; (6bit+1);
        // will be in [0..64] inclusive
        uint8_t beta = MAX_ALPHA - alpha;
        // so (0..64)*alpha + (0..64)*beta always in 0..64

        return (uint16_t)((((alpha * (uint32_t)(fg & MASK_RB)
                             + beta * (uint32_t)(bg & MASK_RB))
                            & MASK_MUL_RB)
                           | ((alpha * (fg & MASK_G) + beta * (bg & MASK_G))
                              & MASK_MUL_G))
                          >> 6);
    }

    /*
  result masks of multiplications
  uppercase: usable bits of multiplications
  RRRRRrrrrrrBBBBBbbbbbb // 5-5 bits of red+blue
        1111100000011111 // from MASK_RB * 1
  1111100000011111000000 //   to MASK_RB * MAX_ALPHA // 22 bits!


  -----GGGGGGgggggg----- // 6 bits of green
        0000011111100000 // from MASK_G * 1
  0000011111100000000000 //   to MASK_G * MAX_ALPHA
*/


    void InitPalette()
    {
        // HEX to RBG565 converter: https://trolsoft.ru/en/articles/rgb565-color-picker
        tftPalette[COLOR_BLACK]       = 0x0000;
        tftPalette[COLOR_WHITE]       = 0xffff;
        tftPalette[COLOR_BLUE]        = 0x5AFF;
        tftPalette[COLOR_DARK_BLUE]   = 0x18EB;
        tftPalette[COLOR_YELLOW]      = 0xFFE0;
        tftPalette[COLOR_DARK_YELLOW] = 0x49E1;
        tftPalette[COLOR_RED]         = 0xF9E1; // 0xff4010
        tftPalette[COLOR_DARK_RED]    = 0x4880; // 0x401000
        tftPalette[COLOR_GREEN]       = 0x3FE7; // 0x40ff40
        tftPalette[COLOR_DARK_GREEN]  = 0x01E0; // 0x004000
        tftPalette[COLOR_LIGHT_GRAY]  = 0xAD75; // 0xb0b0b0
        tftPalette[COLOR_MEDIUM_GRAY] = 0x8C71; // 0x909090
        tftPalette[COLOR_GRAY]        = 0x5AEB; // 0x606060
        tftPalette[COLOR_DARK_GRAY]   = 0x2965; // 0x303030
        tftPalette[COLOR_CYAN]        = 0x76FD; // 0x76dfef
        tftPalette[COLOR_ORANGE]      = 0xFBE0; // 0xff7f00
        tftPalette[COLOR_LIGHT_GREEN] = 0x6FED; // 0x70ff70
        // tftPalette[COLOR_ABL_BG]      = (0x2104); // 0x212121
        tftPalette[COLOR_ABL_BG]     = 0x4A69; // #4D4D4D
        tftPalette[COLOR_ABL_LINE]   = 0x39E7; // 0x3d3d3d
        tftPalette[COLOR_ABL_D_LINE] = 0x31A6; // 0x363636
        tftPalette[COLOR_ABL_L_GRAY] = 0x52AA; // 0x555555
        tftPalette[COLOR_ABL_M_GRAY] = 0x4228; // 0x454545
    }
};


/**
 * A driver implementation for the ILI9341
 */
// class ILI9341Driver
class UiDriver
{
  public:
    enum class Orientation
    {
        Default = 0,
        RRight,
        RLeft,
        UpsideDown,
    };

    void Init()
    {
        screen_update_period_ = 17; // 17 is roughly 60Hz
        screen_update_last_   = System::GetNow();

        InitDriver();
    }

    void InitDriver()
    {
        transport_.Init();

        SetOrientation(Orientation::RLeft);

        transport_.Reset();

        //Software Reset
        transport_.SendCommand(0x01);
        System::Delay(100); // TODO: maybe less?

        // command list is based on https://github.com/martnak/STM32-ILI9341

        // POWER CONTROL A
        transport_.SendCommand(0xCB);
        {
            uint8_t data[5] = {0x39, 0x2C, 0x00, 0x34, 0x02};
            transport_.SendData(data, 5);
        }

        // POWER CONTROL B
        transport_.SendCommand(0xCF);
        {
            uint8_t data[3] = {0x00, 0xC1, 0x30};
            transport_.SendData(data, 3);
        }

        // DRIVER TIMING CONTROL A
        transport_.SendCommand(0xE8);
        {
            uint8_t data[3] = {0x85, 0x00, 0x78};
            transport_.SendData(data, 3);
        }

        // DRIVER TIMING CONTROL B
        transport_.SendCommand(0xEA);
        {
            uint8_t data[2] = {0x00, 0x00};
            transport_.SendData(data, 2);
        }

        // POWER ON SEQUENCE CONTROL
        transport_.SendCommand(0xED);
        {
            uint8_t data[4] = {0x64, 0x03, 0x12, 0x81};
            transport_.SendData(data, 4);
        }

        // PUMP RATIO CONTROL
        transport_.SendCommand(0xF7);
        {
            uint8_t data[1] = {0x20};
            transport_.SendData(data, 1);
        }

        // POWER CONTROL,VRH[5:0]
        transport_.SendCommand(0xC0);
        {
            uint8_t data[1] = {0x23};
            transport_.SendData(data, 1);
        }

        // POWER CONTROL,SAP[2:0];BT[3:0]
        transport_.SendCommand(0xC1);
        {
            uint8_t data[1] = {0x10};
            transport_.SendData(data, 1);
        }

        // VCM CONTROL
        transport_.SendCommand(0xC5);
        {
            uint8_t data[2] = {0x3E, 0x28};
            transport_.SendData(data, 2);
        }

        // VCM CONTROL 2
        transport_.SendCommand(0xC7);
        {
            uint8_t data[1] = {0x86};
            transport_.SendData(data, 1);
        }

        // MEMORY ACCESS CONTROL
        transport_.SendCommand(0x36);
        {
            uint8_t data[1] = {0x48};
            transport_.SendData(data, 1);
        }

        // PIXEL FORMAT
        transport_.SendCommand(0x3A);
        {
            uint8_t data[1] = {0x55};
            transport_.SendData(data, 1);
        }

        // FRAME RATIO CONTROL, STANDARD RGB COLOR
        transport_.SendCommand(0xB1);
        {
            uint8_t data[2] = {0x00, 0x18};
            transport_.SendData(data, 2);
        }

        // DISPLAY FUNCTION CONTROL
        transport_.SendCommand(0xB6);
        {
            uint8_t data[3] = {0x08, 0x82, 0x27};
            transport_.SendData(data, 3);
        }

        // 3GAMMA FUNCTION DISABLE
        transport_.SendCommand(0xF2);
        {
            uint8_t data[1] = {0x00};
            transport_.SendData(data, 1);
        }

        // GAMMA CURVE SELECTED
        transport_.SendCommand(0x26);
        {
            uint8_t data[1] = {0x01};
            transport_.SendData(data, 1);
        }

        // POSITIVE GAMMA CORRECTION
        transport_.SendCommand(0xE0);
        {
            uint8_t data[15] = {0x0F,
                                0x31,
                                0x2B,
                                0x0C,
                                0x0E,
                                0x08,
                                0x4E,
                                0xF1,
                                0x37,
                                0x07,
                                0x10,
                                0x03,
                                0x0E,
                                0x09,
                                0x00};
            transport_.SendData(data, 15);
        }

        // NEGATIVE GAMMA CORRECTION
        transport_.SendCommand(0xE1);
        {
            uint8_t data[15] = {0x00,
                                0x0E,
                                0x14,
                                0x03,
                                0x11,
                                0x07,
                                0x31,
                                0xC1,
                                0x48,
                                0x08,
                                0x0F,
                                0x0C,
                                0x31,
                                0x36,
                                0x0F};
            transport_.SendData(data, 15);
        }

        // EXIT SLEEP
        transport_.SendCommand(0x11);
        System::Delay(100);

        // TURN ON DISPLAY
        transport_.SendCommand(0x29);

        // MADCTL
        transport_.SendCommand(0x36);
        System::Delay(10);

        {
            uint8_t data[1] = {rotation};
            transport_.SendData(data, 1);
        }
    };

    void SetOrientation(Orientation ori)
    {
        uint8_t ili_bgr = 0x08;
        uint8_t ili_mx  = 0x40;
        uint8_t ili_my  = 0x80;
        uint8_t ili_mv  = 0x20;
        switch(ori)
        {
            case Orientation::RRight:
            {
                width    = 320;
                height   = 240;
                rotation = ili_mx | ili_my | ili_mv | ili_bgr;
                return;
            }
            case Orientation::RLeft:
            {
                width    = 320;
                height   = 240;
                rotation = ili_mv | ili_bgr;
                return;
            }
            case Orientation::UpsideDown:
            {
                width    = 240;
                height   = 320;
                rotation = ili_my | ili_bgr;
                return;
            }
            default:
            {
                width    = 240;
                height   = 320;
                rotation = ili_mx | ili_bgr;
            };
        }
    }

    Rectangle GetDrawableFrame() const
    {
        return Rectangle(int16_t(width), int16_t(height))
            .WithTrimmedTop(header)
            .WithTrimmedBottom(footer);
    }

    Rectangle GetBounds() const
    {
        return Rectangle(int16_t(width), int16_t(height));
    }

    void Fill(uint8_t color)
    {
        for(size_t i = 0; i < transport_.buffer_size / 2; i++)
        {
            transport_.PaintPixel(i * 2, color);
        }
    };

    void DrawLine(uint_fast16_t x1,
                  uint_fast16_t y1,
                  uint_fast16_t x2,
                  uint_fast16_t y2,
                  uint8_t       color,
                  uint8_t       alpha = 255)
    {
        if(x1 == x2)
        {
            return DrawVLine(x1, y1, y2 - y1 + 1, color, alpha);
        }
        else if(y1 == y2)
        {
            return DrawHLine(x1, y1, x2 - x1 + 1, color, alpha);
        }

        auto deltaX = abs((int_fast16_t)x2 - (int_fast16_t)x1);
        auto deltaY = abs((int_fast16_t)y2 - (int_fast16_t)y1);
        auto signX  = ((x1 < x2) ? 1 : -1);
        auto signY  = ((y1 < y2) ? 1 : -1);
        auto error  = deltaX - deltaY;

        DrawPixel(x2, y2, color, alpha);

        while((x1 != x2) || (y1 != y2))
        {
            DrawPixel(x1, y1, color, alpha);
            auto error2 = error * 2;
            if(error2 > -deltaY)
            {
                error -= deltaY;
                x1 += signX;
            }

            if(error2 < deltaX)
            {
                error += deltaX;
                y1 += signY;
            }
        }
    }

    void FillArea(uint_fast16_t x,
                  uint_fast16_t y,
                  uint_fast16_t w,
                  uint_fast16_t h,
                  uint8_t       color,
                  uint8_t       alpha = 255)
    {
        // Loop through every Y sector
        for(size_t i = 0; i < h; i++)
        {
            for(size_t j = 0; j < w; j++)
            {
                DrawPixel(x + j, y + i, color, alpha);
            }
        }
    };

    void DrawRect(uint16_t x,
                  uint16_t y,
                  uint16_t w,
                  uint16_t h,
                  uint8_t  color,
                  uint8_t  alpha = 255)
    {
        auto x2 = x + w;
        auto y2 = y + h;
        DrawLine(x, y, x, y2, color, alpha);
        DrawLine(x, y, x2, y, color, alpha);
        DrawLine(x, y2, x2, y2, color, alpha);
        DrawLine(x2, y, x2, y2, color, alpha);
    }

    void DrawRect(const Rectangle& rect, uint8_t color, uint8_t alpha = 255)
    {
        DrawRect(rect.GetX(),
                 rect.GetY(),
                 rect.GetWidth(),
                 rect.GetHeight(),
                 color,
                 alpha);
    }

    void FillRect(const Rectangle& rect, uint8_t color, uint8_t alpha = 255)
    {
        FillArea(rect.GetX(),
                 rect.GetY(),
                 rect.GetWidth(),
                 rect.GetHeight(),
                 color,
                 alpha);
    };

    void DrawTriangle(int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      int16_t x2,
                      int16_t y2,
                      uint8_t color,
                      uint8_t alpha = 255)
    {
        DrawLine(x0, y0, x1, y1, color, alpha);
        DrawLine(x1, y1, x2, y2, color, alpha);
        DrawLine(x2, y2, x0, y0, color, alpha);
    }

    void FillTriangle(int16_t x0,
                      int16_t y0,
                      int16_t x1,
                      int16_t y1,
                      int16_t x2,
                      int16_t y2,
                      uint8_t color,
                      uint8_t alpha = 255)
    {
        int16_t a, b, y, last;

        // Sort coordinates by Y order (y2 >= y1 >= y0)
        if(y0 > y1)
        {
            std::swap(y0, y1);
            std::swap(x0, x1);
        }
        if(y1 > y2)
        {
            std::swap(y2, y1);
            std::swap(x2, x1);
        }
        if(y0 > y1)
        {
            std::swap(y0, y1);
            std::swap(x0, x1);
        }

        if(y0 == y2)
        { // Handle awkward all-on-same-line case as its own thing
            a = b = x0;
            if(x1 < a)
                a = x1;
            else if(x1 > b)
                b = x1;
            if(x2 < a)
                a = x2;
            else if(x2 > b)
                b = x2;
            DrawHLine(a, y0, b - a + 1, color, alpha);
            return;
        }

        int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
                dx12 = x2 - x1, dy12 = y2 - y1;
        int32_t sa = 0, sb = 0;

        // For upper part of triangle, find scanline crossings for segments
        // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
        // is included here (and second loop will be skipped, avoiding a /0
        // error there), otherwise scanline y1 is skipped here and handled
        // in the second loop...which also avoids a /0 error here if y0=y1
        // (flat-topped triangle).
        if(y1 == y2)
        {
            last = y1; // Include y1 scanline
        }
        else
        {
            last = y1 - 1; // Skip it
        }

        for(y = y0; y <= last; y++)
        {
            a = x0 + sa / dy01;
            b = x0 + sb / dy02;
            sa += dx01;
            sb += dx02;
            /* longhand:
            a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
            b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            */
            if(a > b)
            {
                std::swap(a, b);
            }
            DrawHLine(a, y, b - a + 1, color, alpha);
        }

        // For lower part of triangle, find scanline crossings for segments
        // 0-2 and 1-2.  This loop is skipped if y1=y2.
        sa = (int32_t)dx12 * (y - y1);
        sb = (int32_t)dx02 * (y - y0);
        for(; y <= y2; y++)
        {
            a = x1 + sa / dy12;
            b = x0 + sb / dy02;
            sa += dx12;
            sb += dx02;
            /* longhand:
                a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
                b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
            */
            if(a > b)
            {
                std::swap(a, b);
            }
            DrawHLine(a, y, b - a + 1, color, alpha);
        }
    }


    Rectangle WriteStringAligned(const char*    str,
                                 const FontDef& font,
                                 Rectangle      boundingBox,
                                 Alignment      alignment,
                                 uint8_t        color)
    {
        const auto alignedRect
            = GetTextRect(str, font).AlignedWithin(boundingBox, alignment);
        WriteString(str, alignedRect.GetX(), alignedRect.GetY(), font, color);
        return alignedRect;
    }

    Rectangle GetTextRect(const char* text, const FontDef& font) const
    {
        return {int16_t(strlen(text) * font.FontWidth), font.FontHeight};
    }

    char WriteChar(char ch, FontDef font, uint8_t color)
    {
        // Check if character is valid
        if(ch < 32 || ch > 126)
            return 0;

        // Check remaining space on current line
        if(width < (currentX_ + font.FontWidth)
           || height < (currentY_ + font.FontHeight))
        {
            return 0;
        }

        // Use the font to write
        for(auto i = 0; i < font.FontHeight; i++)
        {
            auto b = font.data[(ch - 32) * font.FontHeight + i];
            for(auto j = 0; j < font.FontWidth; j++)
            {
                if((b << j) & 0x8000)
                {
                    DrawPixel(currentX_ + j, (currentY_ + i), color);
                }
            }
        }

        // The current space is now taken
        SetCursor(currentX_ + font.FontWidth, currentY_);

        return ch;
    }

    void WriteString(const char* str, uint16_t x, uint16_t y, FontDef font)
    {
        WriteString(str, x, y, font, COLOR_WHITE);
    }

    void WriteString(const char* str,
                     uint16_t    x,
                     uint16_t    y,
                     FontDef     font,
                     uint8_t     color)
    {
        SetCursor(x, y);
        while(*str) // Write until null-byte
        {
            if(WriteChar(*str, font, color) != *str)
            {
                return; // Char could not be written
            }
            str++; // Next char
        }
    }

    uint16_t GetStringWidth(const char* str, FontDef font) const
    {
        uint16_t font_width = 0;
        // Loop until null-byte
        while(*str)
        {
            font_width += font.FontWidth;
            str++;
        }

        return font_width;
    }

    void DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color)
    {
        int16_t f     = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x     = 0;
        int16_t y     = r;

        DrawPixel(x0, y0 + r, color);
        DrawPixel(x0, y0 - r, color);
        DrawPixel(x0 + r, y0, color);
        DrawPixel(x0 - r, y0, color);

        while(x < y)
        {
            if(f >= 0)
            {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;

            DrawPixel(x0 + x, y0 + y, color);
            DrawPixel(x0 - x, y0 + y, color);
            DrawPixel(x0 + x, y0 - y, color);
            DrawPixel(x0 - x, y0 - y, color);
            DrawPixel(x0 + y, y0 + x, color);
            DrawPixel(x0 - y, y0 + x, color);
            DrawPixel(x0 + y, y0 - x, color);
            DrawPixel(x0 - y, y0 - x, color);
        }
    }

    void FillCircle(int16_t x0, int16_t y0, int16_t r, uint8_t color)
    {
        DrawLine(x0, y0, x0, y0 + 2 * r + 1, color);
        FillCircleHelper(x0, y0, r, 3, 0, color);
    }

    void FillCircleHelper(int16_t x0,
                          int16_t y0,
                          int16_t r,
                          uint8_t cornername,
                          int16_t delta,
                          uint8_t color)
    {
        int16_t f     = 1 - r;
        int16_t ddF_x = 1;
        int16_t ddF_y = -2 * r;
        int16_t x     = 0;
        int16_t y     = r;

        delta++;

        while(x < y)
        {
            if(f >= 0)
            {
                y--;
                ddF_y += 2;
                f += ddF_y;
            }
            x++;
            ddF_x += 2;
            f += ddF_x;

            if(cornername & 0x1)
            {
                DrawLine(
                    x0 + x, y0 - y, x0 + x, y0 - y + 2 * y + delta - 1, color);
                DrawLine(
                    x0 + y, y0 - x, x0 + y, y0 - x + 2 * x + delta - 1, color);
            }
            if(cornername & 0x2)
            {
                DrawLine(
                    x0 - x, y0 - y, x0 - x, y0 - y + 2 * y + delta - 1, color);
                DrawLine(
                    x0 - y, y0 - x, x0 - y, y0 - x + 2 * x + delta - 1, color);
            }
        }
    }

    /**
     * @brief Moves the 'Cursor' position used for WriteChar, and WriteStr to the specified coordinate.
     * 
     * 
     * @param x x pos
     * @param y y pos
     */
    void SetCursor(uint16_t x, uint16_t y)
    {
        currentX_ = (x >= width) ? width - 1 : x;
        currentY_ = (y >= height) ? height - 1 : y;
    }

    void Update()
    {
        Start();
        transport_.SendDataDMA();
    }

    bool IsRender()
    {
        if(transport_.dma_busy == false)
        {
            diff = System::GetNow() - screen_update_last_;
            if(diff > screen_update_period_)
            {
                UpdateFrameRate();
                screen_update_last_ = System::GetNow();
                return true;
            }
        }

        return false;
    }

    void UpdateFrameRate()
    {
        ++frames;
        if(System::GetNow() - fps_update_last_ > 1000)
        {
            fps              = frames;
            frames           = 0;
            fps_update_last_ = System::GetNow();
        }
    }

    uint16_t Fps() const { return fps; }

    void TrimString(char*    str,
                    char*    str_trimmed,
                    uint16_t str_len,
                    uint16_t str_width,
                    FontDef  font) const
    {
        if(str_len <= str_width)
        {
            return;
        }

        uint16_t max_chars = str_width / font.FontWidth;
        strncpy(str_trimmed, str, max_chars);
        str_trimmed[max_chars] = '\0';
    }

  private:
    void Start() { transport_.SetAddressWindow(0, 0, width - 1, height - 1); }

    void DrawPixel(uint_fast16_t x,
                   uint_fast16_t y,
                   uint8_t       color,
                   uint8_t       alpha = 255)
    {
        if(x >= width || y >= height)
            return;

        auto id = 2 * (x + y * width);

        // NOTE: Probably we should check the color id before accessing the array
        transport_.PaintPixel(id, color, alpha);

        // Lets divide the whole screen in 10 sectors, 32 pixel high each
        // uint8_t screen_sector     = y / 32;
        // dirty_buff[screen_sector] = 1;
    }

    void DrawVLine(int16_t x,
                   int16_t y,
                   int16_t h,
                   uint8_t color,
                   uint8_t alpha = 255)
    {
        for(int16_t i = y; i < y + h; i++)
        {
            DrawPixel(x, i, color, alpha);
        }
    }

    void DrawHLine(int16_t x,
                   int16_t y,
                   int16_t w,
                   uint8_t color,
                   uint8_t alpha = 255)
    {
        for(int16_t i = x; i < x + w; i++)
        {
            DrawPixel(i, y, color, alpha);
        }
    }


    /*!
    @brief   Given 8-bit red, green and blue values, return a 'packed'
             16-bit color value in '565' RGB format (5 bits red, 6 bits
             green, 5 bits blue). This is just a mathematical operation,
             no hardware is touched.
    @param   red    8-bit red brightnesss (0 = off, 255 = max).
    @param   green  8-bit green brightnesss (0 = off, 255 = max).
    @param   blue   8-bit blue brightnesss (0 = off, 255 = max).
    @return  'Packed' 16-bit color value (565 format).
    */
    uint16_t Color565(uint8_t red, uint8_t green, uint8_t blue) const
    {
        return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
    }

    uint32_t screen_update_last_, screen_update_period_, fps_update_last_;

    ILI9341SpiTransport transport_;

    uint16_t      width;
    uint16_t      height;
    uint8_t       rotation;
    const uint8_t header = 20;
    const uint8_t footer = 13;
    uint32_t      diff;
    uint16_t      frames = 0;

    uint16_t currentX_;
    uint16_t currentY_;
    // 2 * width * 32; // 2 bits per pixel, 32 rows
    // static uint16_t const num_sectors     = 10;
    // static uint16_t const sector_size     = buffer_size / num_sectors;
    // static uint16_t       dirty_buff[num_sectors]; // = {0};
    // = {0}; // DMA max (?) 65536 // full screen - 153600

    uint16_t fps = 0;
};