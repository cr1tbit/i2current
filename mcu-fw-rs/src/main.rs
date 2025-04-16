#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]
#![feature(impl_trait_in_assoc_type)]

use defmt::*;
use embassy_executor::Spawner;

#[allow(unused)]
use embassy_time::{Duration, Timer, Ticker, Instant};

#[allow(unused)]
use py32_hal::{
    adc::{Adc, SampleTime},
    gpio::{Level, Pull, Speed},
    peripherals::{ADC, I2C1, USART1, PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA12, PA13, PA14, PB0, PB1},
    rcc::{Pll, PllSource, Sysclk},
    time::Hertz,
};
use {defmt_rtt as _, panic_probe as _};

// use cortex_m::peripheral::SCB;
use cortex_m_rt::{exception, ExceptionFrame};

mod bsp;
use bsp::pins::Board;
// use bsp::interrupts::Irqs;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    info!("Hello World!");

    let mut cfg: py32_hal::Config = Default::default();
    cfg.rcc.hsi = Some(Hertz::mhz(24));
    cfg.rcc.pll = Some(Pll {
        src: PllSource::HSI,
    });
    cfg.rcc.sys = Sysclk::PLL;
    let p = py32_hal::init(cfg);

    // Initialize board
    let mut board = Board::init(p);

    // Initialize battery monitor
    // let bat_monitor = bsp::bat::BatteryMonitor::new(board.adc, board.pins.pa0);

    // // Spawn battery monitor task
    // spawner.spawn(bsp::bat::run_bat_monitor(bat_monitor)).unwrap();

    board.i2c.scan();

    // Main loop
    let mut ticker = Ticker::every(Duration::from_secs(1));
    loop {
        ticker.next().await;
        info!("Tick");
    }
}

#[exception]
unsafe fn HardFault(_ef: &ExceptionFrame) -> ! {
    defmt::panic!("HardFault");
}

#[exception]
unsafe fn DefaultHandler(irq: i16) {
    defmt::panic!("Unhandled exception (IRQ {})", irq);
}

