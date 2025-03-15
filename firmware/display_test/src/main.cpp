#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>

#include "commonFwUtils.h"
#include "alfalog.h"
#include "advancedOledLogger.h"

#include "pinDefs.h"
#include "clueLamp.h"

#include "i2current_BSP.hpp"

#include "FS.h"
#include "SD_MMC.h"

volatile uint8_t glo_brightness = 16;
volatile int glo_voltage = 69, glo_power = 70;
volatile int glo_set = 0;

MatrixPanel_I2S_DMA *dma_display = nullptr;

I2currentBoardConfig i2currentConfig = {
    .pinSda = PIN_SDA,
    .pinScl = PIN_SCL,
    .pinIsr = FUSB302_INT_PIN,
    .powerOption = PD_POWER_OPTION_MAX_5V
};

I2currentBoard i2currentBoard(i2currentConfig);

SerialLogger serialLogger = SerialLogger(
    [](const char* str) {Serial.println(str);},
    LOG_DEBUG, ALOG_FANCY
);

// SerialLogger debugLogger = SerialLogger(
//     [](const char* str) {Serial0.println(str);},
//     LOG_DEBUG, ALOG_FANCY
// );

void drawTask(void * parameter)
{
  dma_display = new MatrixPanel_I2S_DMA(getMxConfig());
  dma_display->begin();
  dma_display->setBrightness8(glo_brightness); //0-255
  dma_display->clearScreen();

  delay(1000);

  uint8_t wheelval = 0;
  powerTest(dma_display);
  while(1){
    drawText(dma_display, wheelval);
    wheelval +=1;
    dma_display->flipDMABuffer();
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }    
}

void init_SD(){
  if(!SD_MMC.setPins(sd_clk, sd_cmd, sd_d0, sd_d1, sd_d2, sd_d3)){
    Serial.println("Pin change failed!");
    return;
  }

  if (!SD_MMC.begin()) {
    Serial.println("Card failed, or not present");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
      Serial.println("No SD_MMC card attached");
      return;
  }

  Serial.print("SD_MMC Card Type: ");
  if(cardType == CARD_MMC){
      Serial.println("MMC");
  } else if(cardType == CARD_SD){
      Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
  } else {
      Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n\r", cardSize);

  Serial.printf("Using: %llu/%lluMB\n\r",
    SD_MMC.usedBytes() / (1024 * 1024),
    SD_MMC.totalBytes() / (1024 * 1024)
  );
}


void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); // prevent logger slowdown when no usb connected

  Serial.begin(115200);
  Serial.println(alogGetInitString());

  AlfaLogger.addBackend(&serialLogger);
  // AlfaLogger.addBackend(&debugLogger);
  Wire.begin(PIN_SDA, PIN_SCL, 400000);
  i2currentBoard.start();

  AlfaLogger.begin();
  ALOGD("logger started");
  ALOG_I2CLS(Wire);

  // init_SD();

  // Serial.println("card initialized.");

  xTaskCreatePinnedToCore( drawTask, "drawTask",
      4096, NULL, 31, NULL , 1);
}


void loop() {
  if(Serial.available()){
    char c = Serial.read();
    if(c == 'b'){
      glo_brightness += 16;
    }
    if(c == 'd'){
      glo_brightness -= 16;
    }
    if(c == 'r'){
      //reset esp32
      ESP.restart();
    }
    if (c == 's'){
      Serial.printf("0x%02x\n\r", 
        i2currentBoard.charger.readByte(REG14_Charger_Control_5));
      // i2currentBoard.charger.setInputCurrentLimit(0.2);
      // Serial.println("saving settings");
      // saveSettings();
    }
    if (c == 'l'){
      i2currentBoard.charger.writeByte(REG14_Charger_Control_5, 0x36);
      // loadSettings();
    }
  }

  vTaskDelay(10 / portTICK_PERIOD_MS);

  //run every 3 sec
  static uint32_t lastRun = 0;
  if (millis() - lastRun < 3000) return;
  lastRun = millis();
  
  Serial.println(i2currentBoard.getInputStats());
  Serial.println(i2currentBoard.getBattStats());
  Serial.println("----");
}