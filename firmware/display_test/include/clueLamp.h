
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>

extern volatile uint8_t glo_brightness;
extern volatile int glo_voltage, glo_power, glo_set;

uint16_t colorWheel(MatrixPanel_I2S_DMA* dma_display, uint8_t pos) {
  if(pos < 85) {
    return dma_display->color565(pos * 3, 255 - pos * 3, 0);
  } else if(pos < 170) {
    pos -= 85;
    return dma_display->color565(255 - pos * 3, 0, pos * 3);
  } else {
    pos -= 170;
    return dma_display->color565(0, pos * 3, 255 - pos * 3);
  }
}

uint16_t myBLACK = MatrixPanel_I2S_DMA::color565(0, 0, 0);
uint16_t myWHITE = MatrixPanel_I2S_DMA::color565(255, 0, 255);
uint16_t myRED   = MatrixPanel_I2S_DMA::color565(255, 0, 0);
uint16_t myGREEN = MatrixPanel_I2S_DMA::color565(0, 255, 0);
uint16_t myBLUE  = MatrixPanel_I2S_DMA::color565(0, 0, 255);
uint16_t myORANG = MatrixPanel_I2S_DMA::color565(255, 64, 0);

void powerTest(MatrixPanel_I2S_DMA* dma_display){
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
  while(1){
    // if (glo_brightness < 255) {
    //   glo_brightness ++;
    // }
    dma_display->setBrightness8(glo_brightness);
    dma_display->fillScreen(myWHITE);
    dma_display->setTextColor(myBLACK);
    dma_display->setCursor(2, 2);
    dma_display->println("Voltage: ");
    dma_display->println(glo_voltage);
    dma_display->setCursor(2, 20);
    dma_display->println("Current: ");
    dma_display->println(glo_power);
    dma_display->setCursor(2, 38);
    dma_display->println("Set: ");
    dma_display->println(glo_set);
    dma_display->flipDMABuffer();

    delay(100);
  }
}

void drawText(MatrixPanel_I2S_DMA* dma_display, int colorWheelOffset)
{
  dma_display->fillScreen(myBLACK);
  // draw text with a rotating colour
  dma_display->setTextSize(1);     // size 1 == 8 pixels high
  dma_display->setTextWrap(false); // Don't wrap at end of line - will do ourselves

  dma_display->setCursor(0, 0);    // start at top left, with 8 pixel of spacing
  uint8_t w = 0;
  const char *str = "CYBERBADGE";
  for (w=0; w<strlen(str); w++) {
    dma_display->setTextColor(colorWheel(dma_display,(w*32)+colorWheelOffset));
    dma_display->print(str[w]);
  }

  dma_display->println();
  dma_display->print(" ");
  for (w=9; w<18; w++) {
    dma_display->setTextColor(colorWheel(dma_display,(w*32)+colorWheelOffset));
    dma_display->print("*");
  }
  
  dma_display->println();

  dma_display->setTextColor(dma_display->color444(0,15,4));
  dma_display->println("   2.1    ");

  dma_display->fillCircle(32, 44, 18, dma_display->color444(0, 15, 4));

  dma_display->setTextColor(dma_display->color444(0,0,0));

  dma_display->setCursor(19, 39);
  dma_display->print("truj");
  dma_display->setCursor(20, 47);
  dma_display->print("zomb");
}