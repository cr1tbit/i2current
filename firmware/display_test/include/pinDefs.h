#pragma once
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>


#define PIN_SDA 9
#define PIN_SCL 10

#define FUSB302_INT_PIN   5

const int sd_clk = 21;
const int sd_cmd = 14;
const int sd_d0  = 47;
const int sd_d1  = 48;
const int sd_d2  = 11;
const int sd_d3  = 12;


#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1      // Total number of panels chained one to another


HUB75_I2S_CFG getMxConfig(){
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );
  // mxconfig.gpio.r1   = 4;
  // mxconfig.gpio.g1   = 1;
  // mxconfig.gpio.b1   = 6;
  // mxconfig.gpio.r2   = 7;
  // mxconfig.gpio.g2   = 42;
  // mxconfig.gpio.b2   = 15;
  // mxconfig.gpio.a    = 16;  //AD0
  // mxconfig.gpio.b    = 40;  //AD1
  // mxconfig.gpio.c    = 17;  //AD2
  // mxconfig.gpio.d    = 39;  //AD3
  // mxconfig.gpio.e    = 41;  //AD4
  // mxconfig.gpio.lat  = 13;  
  // mxconfig.gpio.oe   = 8;
  // mxconfig.gpio.clk  = 18;



        mxconfig.gpio.r1   = 4;
        mxconfig.gpio.g1   = 1;
        mxconfig.gpio.b1   = 6;
        mxconfig.gpio.r2   = 7;
        mxconfig.gpio.g2   = 42;
        mxconfig.gpio.b2   = 15;
        mxconfig.gpio.a    = 16;
        mxconfig.gpio.b    = 40;
        mxconfig.gpio.c    = 17;
        mxconfig.gpio.d    = 39;
        mxconfig.gpio.e    = 41;
        mxconfig.gpio.lat  = 13;
        mxconfig.gpio.oe   = 8;
        mxconfig.gpio.clk  = 18;

  // mxconfig.gpio.r1 = 12; //12
  // mxconfig.gpio.g1 = 16;
  // mxconfig.gpio.b1 = 3; //12
  // mxconfig.gpio.r2 = 42; //TMS
  // mxconfig.gpio.g2 = 41; //TDI
  // mxconfig.gpio.b2 = 39; //TCK
  // mxconfig.gpio.a = 8;
  // mxconfig.gpio.b = 17;
  // mxconfig.gpio.c = 5;
  // mxconfig.gpio.d = 45;//3-1
  // mxconfig.gpio.e = 18;
  // mxconfig.gpio.lat = 4;
  // mxconfig.gpio.oe = 40;//TDO
  // mxconfig.gpio.clk = 11;//12-1

  mxconfig.clkphase = false;
  // mxconfig.setPixelColorDepthBits(32);
  mxconfig.double_buff = true;
  return mxconfig;
}