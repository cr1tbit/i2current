#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>
#include <BQ25792_Driver.h>
#include <Wire.h>
#include <PD_UFP.h>

#include "commonFwUtils.h"
#include "alfalog.h"
#include "advancedOledLogger.h"

#include "pinDefs.h"
#include "clueLamp.h"

#include "FS.h"
#include "SD_MMC.h"

BQ25792 charger(-1, -1);
MatrixPanel_I2S_DMA *dma_display = nullptr;

PD_UFP_Log_c PD_UFP;

// TwoWire i2c = TwoWire(0);
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
  dma_display->setBrightness8(255); //0-255
  dma_display->clearScreen();
  uint8_t wheelval = 0;
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

  // Serial0.begin(115200);

  Serial.begin(115200);
  Serial.println(alogGetInitString());
  Wire.begin(PIN_SDA, PIN_SCL, 400000);

  AlfaLogger.addBackend(&serialLogger);
  // AlfaLogger.addBackend(&debugLogger);

  AlfaLogger.begin();
  ALOGD("logger started");
  ALOG_I2CLS(Wire);

  init_SD();

  Serial.println("card initialized.");

  PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_5V);

  charger.reset();
  delay(500);  // give the charger time to reboot

  charger.setChargeCurrentLimit(0.5);

  xTaskCreatePinnedToCore( drawTask, "drawTask",
      4096, NULL, 31, NULL , 1);
}

void loop() {
  // PD_UFP.run();
  // delay(10);

  //run every 3 sec
  static uint32_t lastRun = 0;
  if (millis() - lastRun < 3000) return;
  lastRun = millis();
  
  // PD_UFP.print_status(Serial0);
  // if (PD_UFP.is_power_ready())
  // {
  //   ALOGI("PD supply connected\n");
  // }
  // else
  // {
  //   ALOGI("No PD supply available\n");
  // }

  ALOGI("BQ: {} VBat {}, I {}", 
    charger.getChargeStatus(), charger.getVBAT(), charger.getIBUS()
  );

  delay(10);

}