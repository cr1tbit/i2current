use py32_hal::{adc, bind_interrupts, i2c::GlobalInterruptHandler, peripherals::I2C1};
use py32_hal::peripherals::ADC;

bind_interrupts!(pub struct Irqs {
    ADC_COMP => adc::InterruptHandler<ADC>;
    I2C1 => GlobalInterruptHandler<I2C1>;
}); 