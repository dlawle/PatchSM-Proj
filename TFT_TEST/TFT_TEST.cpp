#include "daisy_patch_sm.h"
#include "ili9341_ui_driver.cpp"

using namespace daisy;
using namespace patch_sm;

DaisyPatchSM hw;
UiDriver driver;

int main(void)
{
  hw.Init();
  driver.Init();
  
  // Here all the drawing happening in the memory buffer, so no drawing happening at this point. 
  driver.Fill(COLOR_BLACK);
  driver.FillRect(Rectangle(100, 100, 50, 50), COLOR_RED);
  driver.DrawRect(Rectangle(100, 100, 50, 50), COLOR_WHITE);

  for(;;)
  {
        // Update() is required to actually flush the screen buffer to the display
        driver.Update();
  }
}