#![no_std]
#![no_main]
#![feature(impl_trait_in_assoc_type)]
#![feature(generic_const_exprs)]

use cortex_m::peripheral::SCB;
use cortex_m_rt::{exception, ExceptionFrame};
use embassy_time::{Ticker, Duration, Instant};
use embassy_executor::Spawner;

use py32_hal::gpio::Level;
use py32_hal::rcc::{Pll, PllSource, Sysclk};
use py32_hal::time::Hertz;
use defmt::*;
use {defmt_rtt as _, panic_halt as _};

mod bat;
mod pins;
mod interrupts;

use pins::Board;

#[embassy_executor::main]
async fn main(_spawner: Spawner) {
    info!("Hello World!");

    let mut cfg: py32_hal::Config = Default::default();
    cfg.rcc.hsi = Some(Hertz::mhz(24));
    cfg.rcc.pll = Some(Pll {
        src: PllSource::HSI,
    });
    cfg.rcc.sys = Sysclk::PLL;
    let p = py32_hal::init(cfg);

    // Initialize the board
    let mut board = Board::init(p);

    board.pins.pa12.set_as_output(py32_hal::gpio::Speed::VeryHigh);
    board.pins.pa12.set_low();

    _spawner.spawn(bat::run_bat_monitor(board.adc)).unwrap();
   
    let mut cnt:u16 = 0;
    let mut ticker = Ticker::every(Duration::from_millis(100));

    loop {
        let time_start: Instant = Instant::now();
        
        if board.pins.pa12.is_high() {
            board.pins.pa12.set_low();
            info!("low")
        } else {
            board.pins.pa12.set_high();
            info!("high")
        }

        // cnt += 1;
        // if cnt % 10 == 0 {
            let duration: Duration = Instant::now() - time_start;
            info!("tick {}, render time {}us", cnt, duration.as_micros());
        // }
        ticker.next().await;        
    }
}

//conserve flash space with the non-handler
#[exception]
unsafe fn HardFault(_frame: &ExceptionFrame) -> ! {
    SCB::sys_reset()
}

