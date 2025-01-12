#include <Arduino.h>

#include <BQ25792_Driver.h>
#include <Wire.h>
#include <PD_UFP.h>
extern volatile int glo_voltage, glo_power, glo_set;

typedef struct {
    int pinSda;
    int pinScl;
    int pinIsr;
    PD_power_option_t powerOption;
} I2currentBoardConfig;

class I2currentBoard
{
private:
    BQ25792 charger;
    PD_UFP_Log_c pdController;

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

                    board->charger.setInputCurrentLimit(availableCurrent_mA/1000.0);
                }
            }
        }
    }

public:
    I2currentBoard(I2currentBoardConfig config): charger(-1, -1), pdController(){
        this->config = config;
        }

    void start()
    {
        pdController.init(config.pinIsr, PD_POWER_OPTION_MAX_POWER);

        charger.reset();
        delay(500);  // give the charger time to reboot

        charger.setChargeCurrentLimit(5.0);
        charger.setInputCurrentLimit(0.5);

        xTaskCreate( i2currentWatchdogTask, "i2currentWatchdogTask",
                      4096, this, 31, NULL);
    }

    String getInputStats()
    {
        String ret = "";
        float vbus = charger.getADC(MEAS_TYPE::VBUS);
        float vbus2 = charger.getADC(MEAS_TYPE::VAC2);
        
        float ibus = charger.getADC(MEAS_TYPE::IBUS);

        float power = vbus * ibus / 1000;
        ret += "Vbus: " + String(vbus) + "V, ";
        ret += "Vbus2: " + String(vbus2) + "V, ";
        ret += "Ibus: " + String(ibus) + "mA, ";
        ret += "Power: " + String(power) + "W\n\r";
        // ALOGI("Vbus {:.2f}V, IBus {:.2f}mA Power {:.2f}W",
        //       vbus, ibus, power);

        float vbat = charger.getADC(MEAS_TYPE::VBAT);
        float ibat = charger.getADC(MEAS_TYPE::IBAT);
        float baT_power = vbat * ibat / 1000;
        ret += "Vbat: " + String(vbat) + "V, ";
        ret += "Ibat: " + String(ibat) + "mA, ";
        ret += "Power: " + String(baT_power) + "W\n\r";       

        glo_voltage = (int)(vbus*1000.0);
        glo_power = (int)(ibus);
 

        // ALOGI("VBat {:.2f}V, IBat {:.2f}mA Power {:.2f}W\n",
        //       vbat, ibat, power);
        return ret;
    }

    void dumpAllPwrStats()
    {
        for (int i = 0; i < 11; i++)
        {
            float val = charger.getADC((MEAS_TYPE)i);
            // ALOGI("ADC {}: {:.2f}", i, val);
        }
    }
};