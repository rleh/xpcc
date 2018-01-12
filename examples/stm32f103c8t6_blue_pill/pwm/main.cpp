#include <xpcc/architecture/platform.hpp>

using namespace Board;

/*
 * Blinks the green user LED with 1 Hz.
 * It is on for 90% of the time and off for 10% of the time.
 */

using Pin = GpioOutputA8;
static uint16_t Overflow = 1000;//65535 >> 5;

struct systemClockTimer1 {
	static constexpr uint32_t Timer1 = Board::systemClock::Apb2;
};

inline void
initializePwm()
{
	Pin::connect(Timer1::Channel1);
	Timer1::enable();
	Timer1::setMode(Timer1::Mode::UpCounter);
	//Overflow = Timer::setPeriod<systemClockTimer1>(200 * 1000); // 0.2s
	Timer1::setPrescaler(1000);
	Timer1::setOverflow(Overflow);
	Timer1::configureOutputChannel(1, Timer1::OutputCompareMode::Pwm, 0);
	Timer1::applyAndReset();
	Timer1::start();
	Timer1::enableOutput();
}

int
main()
{
	//Disable JTAG
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
	// Enable AxB clocks
	RCC->APB2ENR |= (RCC_APB2ENR_AFIOEN | RCC_APB2ENR_TIM1EN | RCC_APB2ENR_USART1EN);
	RCC->APB1ENR |= (RCC_APB1ENR_I2C1EN | RCC_APB1ENR_SPI2EN);

	Board::initialize();

	LedGreen::set();
	
	initializePwm();
	
	Timer1::setCompareValue(1, Overflow/4);
	

	while (true)
	{
		LedGreen::set();
		xpcc::delayMilliseconds(200);

		LedGreen::reset();
		xpcc::delayMilliseconds(200);
	}

	return 0;
}
