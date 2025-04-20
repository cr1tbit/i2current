use py32_hal::{
    gpio::{AnyPin, Pin}, i2c::{Config, I2c, SclPin, SdaPin}, mode::Async, peripherals::I2C1, time::Hertz
};

use defmt::*;

use crate::bsp::interrupts::Irqs;

/// I2C peripheral configuration
pub struct I2cConfig {
    /// I2C clock speed in Hz
    pub speed: Hertz,
    /// I2C peripheral configuration
    pub config: Config,
}

impl Default for I2cConfig {
    fn default() -> Self {
        Self {
            speed: Hertz::khz(100),
            config: Config::default(),
        }
    }
}

/// I2C peripheral wrapper
pub struct I2cPeripheral<'a> {
    /// I2C peripheral instance
    pub i2c: I2c<'a, Async>,
    pub pin_isr: AnyPin
}

impl<'a> I2cPeripheral<'a> {
    /// Initialize I2C peripheral with default configuration
    pub fn new(i2c: I2C1,
            sda: impl SdaPin<I2C1>,
            scl: impl SclPin<I2C1>,
            isr: impl Pin
        ) -> Self {
        Self::with_config(i2c, sda, scl, isr, I2cConfig::default())
    }

    //  Initialize I2C peripheral with custom configuration
    pub fn with_config(
        i2c: I2C1,
        sda: impl SdaPin<I2C1>,
        scl: impl SclPin<I2C1>,
        isr: impl Pin,
        config: I2cConfig,
    ) -> Self {
        let i2c = I2c::new(
            i2c, scl, sda,
            Irqs, config.speed, config.config
        );
        Self { i2c, pin_isr: isr.into() }
    }

    pub fn scan(&mut self) {
        for addr in 1..=127 {
            // info!("Scanning Address {}", addr as u8);

            // Scan Address
            let res = self.i2c.blocking_read(addr as u8, &mut [0]);

            // Check and Print Result
            match res {
                Ok(_) => info!("Device Found at Address 0x{:02x}", addr as u8),
                Err(_) => trace!("No Device Found"),
            }
        }
    }
}
