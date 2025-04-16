// Please note, this code has been EXTREMELY vibecoded, and looks like shit.
// I only tested GPIO for now. Putting it on the repo for reference.

use defmt::*;
use embassy_time::Timer;
use py32_hal::{
    adc::{Adc, SampleTime},
    gpio::{Flex, Level, Pull, Speed},
    peripherals::{ADC, I2C1, PA0, PA1, PA12, PA13, PA14, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PB0, PB1, USART1}, Peripheral,
};

use crate::bsp::{interrupts::Irqs, i2c::{I2cPeripheral, I2cSdaPin, I2cSclPin}};

/// Board Support Package for the PY32 MCU
#[allow(unused)]
pub struct Board<'a> {
    /// ADC peripheral
    pub adc: Adc<'a, ADC>,
    /// I2C peripheral
    pub i2c: I2cPeripheral<'a>,
    /// UART peripheral
    pub uart: USART1,
    /// SWD pins
    pub swd: SwdPins,
    /// Available GPIO pins
    pub pins: Pins,
}

/// SWD debug pins
#[allow(unused)]
pub struct SwdPins {
    /// SWDIO pin
    pub swdio: Flex<'static>,
    /// SWCLK pin
    pub swclk: Flex<'static>,
}

/// Available GPIO pins
#[allow(unused)]
pub struct Pins {
    /// PA0 - ADC input
    pub pa0: Flex<'static>,
    /// PA1 - I2C internal
    pub pa1: Flex<'static>,
    /// PA2 - I2C SDA
    /// PA3 - I2C SCL
    /// PA4 - Battery charger chip select
    pub pa4: Flex<'static>,
    /// PA5 - OLED reset
    pub pa5: Flex<'static>,
    /// PA6 - Global power off
    pub pa6: Flex<'static>,
    /// PA7 - Power board UART TX
    pub pa7: Flex<'static>,
    /// PA8 - Power board UART RX
    pub pa8: Flex<'static>,
    /// PA12 - 5V enable
    pub pa12: Flex<'static>,
    /// PB0 - Power board boot
    pub pb0: Flex<'static>,
    /// PB1 - 3.3V enable
    pub pb1: Flex<'static>,
}

impl<'a> Board<'a> {
    /// Initialize the board and its peripherals
    pub fn init(p: py32_hal::Peripherals) -> Self {
        // Initialize SWD pins
        let swd = SwdPins {
            swdio: Flex::new(p.PA13),
            swclk: Flex::new(p.PA14),
        };

        // Initialize GPIO pins
        let pins = Pins {
            pa0: Flex::new(p.PA0),
            pa1: Flex::new(p.PA1),
            pa4: Flex::new(p.PA4),
            pa5: Flex::new(p.PA5),
            pa6: Flex::new(p.PA6),
            pa7: Flex::new(p.PA7),
            pa8: Flex::new(p.PA8),
            pa12: Flex::new(p.PA12),
            pb0: Flex::new(p.PB0),
            pb1: Flex::new(p.PB1),
        };
        unsafe {
            // Initialize ADC
            let mut adc = Adc::new(p.ADC, Irqs);
            adc.set_sample_time(SampleTime::CYCLES71_5);

            // Initialize I2C
            let i2c = I2cPeripheral::new(
                p.I2C1,  
                p.PA2.clone_unchecked(), 
                p.PA3.clone_unchecked());

            // Initialize UART
            let uart = p.USART1;

            Self {
                adc,
                i2c,
                uart,
                swd,
                pins,
            }
        }
    }
}

/// Helper functions for configuring pins
#[allow(unused)]
impl Pins {
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

impl I2cSdaPin for Flex<'static> {}
impl I2cSclPin for Flex<'static> {}