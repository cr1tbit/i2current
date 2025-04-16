use defmt::*;
use embassy_time::Timer;
use py32_hal::adc::{Adc, SampleTime};
use py32_hal::peripherals::ADC;
use py32_hal::gpio::Flex;

// use crate::bsp::interrupts::Irqs;

// use crate::pins;

#[allow(unused)]
pub struct BatteryMonitor {
    adc: Adc<'static, ADC>,
    pin: Flex<'static>,
}

impl BatteryMonitor {
    pub fn new(adc: Adc<'static, ADC>, pin: Flex<'static>) -> Self {
        Self { adc, pin }
    }
}

#[embassy_executor::task]
pub async fn run_bat_monitor(mut monitor: BatteryMonitor) {
    monitor.adc.set_sample_time(SampleTime::CYCLES71_5);

    let mut vrefint = monitor.adc.enable_vref();

    loop {
        let vrefint_sample: u32 = monitor.adc.read(&mut vrefint).await.into();
        let vcc: u32 = 1200 * 4095 / vrefint_sample;
        info!("--> {} - {} mV", vrefint_sample, vcc);
        Timer::after_millis(2000).await;
    }
}