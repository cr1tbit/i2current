#include <Arduino.h>

#include <BQ25792_Driver.h>
#include <Wire.h>
#include <PD_UFP.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern volatile int glo_voltage, glo_power, glo_set;

typedef struct {
    int pinSda;
    int pinScl;
    int pinIsr;
    PD_power_option_t powerOption;
} I2currentBoardConfig;

class I2currentBoard
{
public:
    BQ25792 charger;
    PD_UFP_Log_c pdController;
    Adafruit_SSD1306 display;

    I2currentBoardConfig config;

    static void i2currentWatchdogTask(void *pvParameters)
    {
        I2currentBoard *board = (I2currentBoard *)pvParameters;
        int loop = 1;
        while (1)
        {
            board->pdController.run();
            vTaskDelay(10);
            if(loop++ % 3 == 0){
                // board->pdController.print_status(Serial0);
                if (board->pdController.is_power_ready())
                {
                    float availableCurrent_mA = board->pdController.get_current() * 10.0;
                    float availableVoltage_mV = board->pdController.get_voltage() * 50;

                    glo_set = (int)(availableCurrent_mA);
                }
            }

            //run every 2sec
            static uint32_t lastRun = 0;
            if (millis() - lastRun < 2000) continue;
            lastRun = millis();

            if (board->charger.isBatteryPresent()){
                float vbat = board->charger.getADC(MEAS_TYPE::VBAT);
                // float ibat = board->charger.getADC(MEAS_TYPE::IBAT);

                float vbus = board->charger.getADC(MEAS_TYPE::VBUS);
                int ibus = (int)board->charger.getADC(MEAS_TYPE::IBUS);
                float power = vbus * ibus / 1000;

                board->drawBatt(
                    int((vbat-7.4f)*100.0f),
                    String(power) + "W"
                );
            } else {
                board->drawBatt(0, "No bat");
            }

            
        }
    }

public:
    I2currentBoard(I2currentBoardConfig config): charger(-1, -1), pdController(){
        this->config = config;
        }

    void start()
    {
        pdController.init(config.pinIsr, PD_POWER_OPTION_MAX_5V);

        charger.reset();
        delay(500);  // give the charger time to reboot

        charger.setChargeCurrentLimit(1.0);
        charger.setInputCurrentLimit(1.0);
        //default 0x16, set 0x36 to enable discharge current monitoring
        //doesnt work yet, TODO
        charger.writeByte(REG14_Charger_Control_5, 0x36); 

        display = Adafruit_SSD1306(128, 64, &Wire);
        display.begin(SSD1306_SWITCHCAPVCC, 0x3C);      
        display.setRotation(1);

        xTaskCreate( i2currentWatchdogTask, "i2currentWatchdogTask",
                      4096, this, 31, NULL);
    }

    String getInputStats()
    {
        String ret = "";
        float vac1 = charger.getADC(MEAS_TYPE::VAC1);
        float vac2 = charger.getADC(MEAS_TYPE::VAC2);

        float vbus = charger.getADC(MEAS_TYPE::VBUS);
        int ibus = (int)charger.getADC(MEAS_TYPE::IBUS);
        float power = vbus * ibus / 1000;

        ret += "Voltages on DC / USB: " + String(vac1) + "V, " + String(vac2) + "V \n\r";
        ret += "Charging from " + String(vac2) + "V @ " + String(ibus) + "mA ("
            + String(power) + "W), max: " + charger.getInputCurrentLimit() + "A";
        // ret += "Power: " + String(power) + "W\n\r";
        // ALOGI("Vbus {:.2f}V, IBus {:.2f}mA Power {:.2f}W",
        //       vbus, ibus, power);

        glo_voltage = (int)(vac1*1000.0);
        glo_power = (int)(ibus); 

        // ALOGI("VBat {:.2f}V, IBat {:.2f}mA Power {:.2f}W\n",
        //       vbat, ibat, power);
        return ret;
    }

    String getBattStats(){
        String ret = "";

        int errorInt = charger.getErrorInt();

        if (errorInt){
            ret += "Error detected, code: ";
            ret += errorInt;
            ret += "\n\r";
        }

        if (charger.isBatteryPresent()){
            ret += "State: ";
            ret += charger.getChargeStatus();

            float vsys = charger.getADC(MEAS_TYPE::VSYS);
            float vbat = charger.getADC(MEAS_TYPE::VBAT);
            int ibat = (int)charger.getADC(MEAS_TYPE::IBAT);
            float baT_power = vbat * ibat / 1000;
            ret += " : " + String(vbat) + "V @ " + String(ibat) + "mA (" + 
                String(baT_power) + "W), max: " + charger.getChargeCurrentLimit() + "A\n\r";
        } else {
            ret += "battery not detected";
        }

        ret+= "VSYS: " + String(charger.getADC(MEAS_TYPE::VSYS)) + "V, "; 
        // ret+= "TEMP: " + String(charger.getADC(MEAS_TYPE::TS)) + "C\n\r";
        ret+= "BQ25792 IC temp: " + String(charger.getADC(MEAS_TYPE::TDIE)) + "C\n\r";

        return ret;
    }

    // void dumpAllPwrStats()
    // {
    //     for (int i = 0; i < 11; i++)
    //     {
    //         float val = charger.getADC((MEAS_TYPE)i);
    //         // ALOGI("ADC {}: {:.2f}", i, val);
    //     }
    // }

    void dumpRegisters(){
        for (int i = 0; i < 0x50; i++)
        {
          Serial.printf("0x%02x: 0x%02x\n\r", 
            i, charger.readByte(i));
        }
    }
    

    //https://forum.arduino.cc/t/adafruit_gfx-fillarc/397741/7
    void fillArc2(
        int x, int y, int start_angle, int seg_count, 
        int rx, int ry, int w, unsigned int colour)
    {
        const float DEG2RAD = PI / 180.0f;
        byte seg = 3;
        byte inc = 3;

        // Calculate first pair of coordinates for segment start
        float sx = cos((start_angle - 90) * DEG2RAD);
        float sy = sin((start_angle - 90) * DEG2RAD);
        uint16_t x0 = sx * (rx - w) + x;
        uint16_t y0 = sy * (ry - w) + y;
        uint16_t x1 = sx * rx + x;
        uint16_t y1 = sy * ry + y;

        // Draw colour blocks every inc degrees
        for (int i = start_angle; i < start_angle + seg * seg_count; i += inc) {

            // Calculate pair of coordinates for segment end
            float sx2 = cos((i + seg - 90) * DEG2RAD);
            float sy2 = sin((i + seg - 90) * DEG2RAD);
            int x2 = sx2 * (rx - w) + x;
            int y2 = sy2 * (ry - w) + y;
            int x3 = sx2 * rx + x;
            int y3 = sy2 * ry + y;

            display.fillTriangle(x0, y0, x1, y1, x2, y2, colour);
            display.fillTriangle(x1, y1, x2, y2, x3, y3, colour);

            // Copy segment end to sgement start for next segment
            x0 = x2;
            y0 = y2;
            x1 = x3;
            y1 = y3;
        }
    }


    void drawBatt(int percent, String text){
        // resolution: 40 x 72
        // the middle: 48; 64
        // top: 28 
        // bottom: 68 (48 + 20)
        // left: 40 
        // right: 104 (40 + 64)
        
        // display.drawRect(29, 28, 34, 42, SSD1306_WHITE);

        int angle = int(percent * 3.6 / 3);
        display.clearDisplay();

        fillArc2(48, 50, 95, angle, 15, 15, 5, SSD1306_WHITE);

        display.setCursor(32, 82);
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.print(text);
        display.display();
    }
};