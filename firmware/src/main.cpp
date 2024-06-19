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

BQ25792 charger(0, 0);
MatrixPanel_I2S_DMA *dma_display = nullptr;

PD_UFP_c PD_UFP;

TwoWire i2c = TwoWire(0);
SerialLogger serialLogger = SerialLogger(
    [](const char* str) {Serial.println(str);},
    LOG_DEBUG, ALOG_FANCY
);

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

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }    
}

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0); // prevent logger slowdown when no usb connected

  Serial.begin(115200);
  Serial.println(alogGetInitString());
  i2c.begin(PIN_SDA, PIN_SCL);

  AlfaLogger.addBackend(&serialLogger);

  AlfaLogger.begin();
  ALOGD("logger started");
  ALOG_I2CLS(i2c);

  PD_UFP.init(FUSB302_INT_PIN, PD_POWER_OPTION_MAX_9V);

  charger.reset();
  delay(500);  // give the charger time to reboot

  xTaskCreatePinnedToCore( drawTask, "drawTask",
      4096, NULL, 31, NULL , 1);
}

void loop() {
  ALOGI("Status {} Cells {} Max Voltage {} MinVoltage {} VBat {}", 
    charger.getChargeStatus(), charger.getCellCount(), 
    charger.getChargeVoltageLimit(), charger.getVSYSMIN(), charger.getVBAT()
  );  
  delay(1000);
}