# OLED

Implementation of the OLED example for Daisy Seed on the Daisy Patch Submodule/Daisy Patch.init().

Changes to pin configuration can be made in the OLED.cpp file, but were assigned with the Daisy Patch.Init() in mind (see Patch.Init()'s 12 pin jumper on right side of board marked M12).

While working on getting the OLED working (this has been successful) I'll also be testing geting a rotary encoder function working. 

# OLED (SPI) Pinout

OLED    Pin Name        M12 Num     Function
GND     GND             12
VCC     3v3             1
D0      SIG_SPI_SCK     7           SCK             
D1      SIG_SPI_MOSI    5           MOSI
RES     SIG_UART_TX     10          Reset
DC      SIG_UART_RX     9           Data/Cmd
CS      SIG_SPI_NSS     8           Chip Select


# Encoder Pinout

OLED    Pin Name        M12 Num     Function
GND     GND             12
VCC     3v3             1
SW      SIG_USB_DP      3           Switch
DT      SIG_USB_DM      4           B
CLK     SIG_SPI_MISO    6           A