use py32_hal::{adc, bind_interrupts};
use py32_hal::peripherals::ADC;

bind_interrupts!(pub struct Irqs {
    ADC_COMP => adc::InterruptHandler<ADC>;
}); 