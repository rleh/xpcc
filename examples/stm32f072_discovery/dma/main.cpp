#include <xpcc/architecture/platform.hpp>
#include <xpcc/debug/logger.hpp>

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper<Usart1, xpcc::IOBuffer::BlockIfFull> loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG


// Allocate 200 bytes memory
constexpr uint16_t SampleLength = 100;
uint16_t samples[SampleLength];

constexpr int SampleFrequency = kHz125;

int
main()
{
	Board::initialize();

	// Enable USART 1
	GpioOutputA9::connect(Usart1::Tx);
	GpioInputA10::connect(Usart1::Rx, Gpio::InputType::PullUp);
	Usart1::initialize<Board::systemClock, 115200>(12);

	for(uint16_t i = 0; i < SampleLength; ++i) {
		samples[i] = i;
	}

	Dma1::enable();
	Dma1::Stream4::stop();
	Dma1::Stream4::configure(SampleLength, DmaBase::Priority::VeryHigh, DmaBase::CircularMode::Enabled);
	Dma1::Stream4::setPeripheralDestination(reinterpret_cast<uint16_t*>(const_cast<uint32_t*>(&(TIM1->CCR4))));
	Dma1::Stream4::setMemorySource(&samples[0]);

	// Start Timer1
	constexpr uint16_t Overflow = (Board::systemClock::Frequency / SampleFrequency) - 1;
	constexpr uint16_t Compare  = (Overflow / 2) + 1;	// 50% duty cycle
	Timer1::enable();
	Timer1::setMode(Timer1::Mode::UpCounter);
	Timer1::setPrescaler(1);	// 48MHz
	Timer1::setOverflow(Overflow);
	Timer1::configureOutputChannel(4, Timer1::OutputCompareMode::Pwm, Compare);
	GpioOutputA11::configure(Gpio::OutputType::PushPull);
	GpioOutputA11::connect(Timer1::Channel4);
	Timer1::applyAndReset();
	Timer1::start();
	Timer1::enableOutput();

	// Use Timer1 Channel4 as DMA Trigger
	Timer1::enableDmaRequest(GeneralPurposeTimer::DmaRequestEnable::CaptureCompare4);
	//TIM1->DIER |= TIM_DIER_CC4DE; // CC 4 DMA Request enable


	while (1)
	{
		Board::LedUp::set();
		Dma1::Stream6::start();
		while(!Dma1::Stream6::isFinished());
		Board::LedUp::reset();
		xpcc::delayMilliseconds(10);
	}

	return 0;
}
