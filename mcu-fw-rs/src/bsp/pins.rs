// Please note, this code has been EXTREMELY vibecoded, and looks like shit.
// I only tested GPIO for now. Putting it on the repo for reference.

use py32_hal::{
    adc::{Adc, SampleTime},
    gpio::{Flex, Level, Pull, Speed},
    peripherals::ADC,
    rcc::{Pll, PllSource, Sysclk},
    time::Hertz,
    Peripheral, Peripherals,
};

use crate::bsp::{
    i2c::{I2cPeripheral},
    interrupts::Irqs,
};

/// Board Support Package for the PY32 MCU
#[allow(unused)]
pub struct Board<'a> {
    /// ADC peripheral
    pub adc: Adc<'a, ADC>,
    /// I2C peripheral
    pub i2c: I2cPeripheral<'a>,
    /// Available GPIO pins
    pub pins: ControlPins,
}

/// SWD debug pins
#[allow(unused)]
pub struct SwdPins {
    /// SWDIO pin
    pub swdio: Flex<'static>,
    /// SWCLK pin
    pub swclk: Flex<'static>,
}

// /// PA0 - ADC input
// /// PA1 - I2C interrupt
// /// PA2 - I2C SDA
// /// PA3 - I2C SCL
// /// PA4 - Battery charger chip select
// /// PA5 - OLED reset
// /// PA6 - Global power off
// /// PA7 - Power board UART TX
// /// PA8 - Power board UART RX
// /// PA12 - 5V enable
// /// PB0 - Power board boot
// /// PB1 - 3.3V enable

/// Available GPIO pins
#[allow(unused)]
pub struct ControlPins {
    pub chg_cs: Flex<'static>,   //CS
    pub oled_rst: Flex<'static>, //OLED rst
    pub glob_off: Flex<'static>, //GLOB_OFF
    pub ena_5v: Flex<'static>,   //5V ENA
    pub ena_3v3: Flex<'static>,  //3v3 ENA
}

impl<'a> Board<'a> {
    pub fn get_periph_handle() -> Peripherals {
        let mut cfg: py32_hal::Config = Default::default();
        cfg.rcc.hsi = Some(Hertz::mhz(24));
        cfg.rcc.pll = Some(Pll {
            src: PllSource::HSI,
        });
        cfg.rcc.sys = Sysclk::PLL;

        py32_hal::init(cfg)
    }

    /// Initialize the board and its peripherals
    pub fn init() -> Self {
        let p = Self::get_periph_handle();

        // Initialize GPIO pins
        let pins = ControlPins {
            chg_cs: Flex::new(p.PA4),
            oled_rst: Flex::new(p.PA5),
            glob_off: Flex::new(p.PA6),
            ena_5v: Flex::new(p.PA12),
            ena_3v3: Flex::new(p.PB1),
        };
        unsafe {
            // Initialize ADC
            let mut adc = Adc::new(p.ADC, Irqs);
            adc.set_sample_time(SampleTime::CYCLES71_5);

            // Initialize I2C
            let i2c = I2cPeripheral::new(
                p.I2C1,
                p.PA2.clone_unchecked(),
                p.PA3.clone_unchecked(),
                p.PA1.clone_unchecked(),
            );

            Self { adc, i2c, pins }
        }
    }
}

/// Helper functions for configuring pins
#[allow(unused)]
impl ControlPins {
    /// Configure a pin as output
    pub fn configure_output(&mut self, pin: &mut Flex<'static>, level: Level) {
        pin.set_as_output(Speed::VeryHigh);
        match level {
            Level::High => pin.set_high(),
            Level::Low => pin.set_low(),
        }
    }

    /// Configure a pin as input
    pub fn configure_input(&mut self, pin: &mut Flex<'static>, pull: Pull) {
        pin.set_as_input(pull);
    }
}