#include "ili9341_ui_driver.hpp"

uint8_t DMA_BUFFER_MEM_SECTION
    ILI9341SpiTransport::frame_buffer[ILI9341SpiTransport::buffer_size]
    = {0}; // DMA max (?) 65536 // full screen - 153600

uint8_t DSY_SDRAM_BSS
    ILI9341SpiTransport::color_mem[ILI9341SpiTransport::buffer_size / 2]
    = {0};