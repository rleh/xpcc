#include <xpcc/architecture/platform.hpp>
#include <xpcc/architecture.hpp>
#include <xpcc/debug/logger.hpp>

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper<Usart2, xpcc::IOBuffer::BlockIfFull> loggerDevice;

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

constexpr int SampleFrequency = kHz10;


// ----------------------------------------------------------------------------
int main()
{
	Board::systemClock::enable();

	Board::LedOrange::setOutput(xpcc::Gpio::High);
	Board::LedGreen::setOutput(xpcc::Gpio::Low);
	Board::LedRed::setOutput(xpcc::Gpio::Low);
	Board::LedBlue::setOutput(xpcc::Gpio::Low);

	// Initialize Usart
	GpioOutputA2::connect(Usart2::Tx);
	GpioInputA3::connect(Usart2::Rx);
	Usart2::initialize<Board::systemClock, 115200>(10);

	XPCC_LOG_INFO << "Sampling freqeuncy: " << kHz(SampleFrequency) << "kHz" << xpcc::endl;
	XPCC_LOG_INFO << "Sample length: " << SampleLength << xpcc::endl;

	// Init Gpio Inputs
	GpioInputE0::setInput(Gpio::InputType::Floating);
	GpioInputE1::setInput(Gpio::InputType::Floating);
	GpioInputE2::setInput(Gpio::InputType::Floating);
	GpioInputE3::setInput(Gpio::InputType::Floating);
	GpioInputE4::setInput(Gpio::InputType::Floating);
	GpioInputE5::setInput(Gpio::InputType::Floating);
	GpioInputE6::setInput(Gpio::InputType::Floating);
	GpioInputE7::setInput(Gpio::InputType::Floating);
	GpioInputE8::setInput(Gpio::InputType::Floating);
	GpioInputE9::setInput(Gpio::InputType::Floating);
	GpioInputE10::setInput(Gpio::InputType::Floating);
	GpioInputE11::setInput(Gpio::InputType::Floating);
	GpioInputE12::setInput(Gpio::InputType::Floating);
	GpioInputE13::setInput(Gpio::InputType::Floating);
	GpioInputE14::setInput(Gpio::InputType::Floating);
	GpioInputE15::setInput(Gpio::InputType::Floating);


	for(uint32_t i = 0; i < SampleLength; ++i) {
		samples[i] = 0xffff;
	}

	Dma2::enable();
	Dma2::Stream6::stop();
	Dma2::Stream6::configure(DmaBase::Channel::Channel0, SampleLength, DmaBase::Priority::VeryHigh);
	Dma2::Stream6::setPeripheralSource(reinterpret_cast<uint16_t*>(const_cast<uint32_t*>(&(GPIOE->IDR))));
	Dma2::Stream6::setMemoryDestination(&samples[0]);

	// Start Timer1
	constexpr uint16_t Overflow = (Board::systemClock::Timer1 / SampleFrequency) - 1;
	constexpr uint16_t Compare  = (Overflow / 2) + 1;	// 50% duty cycle
	Timer1::enable();
	Timer1::setMode(Timer1::Mode::UpCounter);
	Timer1::setPrescaler(1);	// 168MHz
	Timer1::setOverflow(Overflow);
	Timer1::configureOutputChannel(3, Timer1::OutputCompareMode::Pwm, Compare);
	GpioOutputA10::configure(Gpio::OutputType::PushPull);
	GpioOutputA10::connect(Timer1::Channel3);
	Timer1::applyAndReset();
	Timer1::start();
	Timer1::enableOutput();

	// Use Timer1 Channel3 as DMA Trigger
	TIM1->DIER |= TIM_DIER_CC3DE;	// CC 3 DMA Request enable


	Board::LedBlue::set();
	Dma2::Stream6::start();
	while(!Dma2::Stream6::isFinished());
	Board::LedBlue::reset();

	// Output Sampled Data
	for(int i = 0; i < SampleLength; ++i) {
		uint16_t sample = samples[i];
		for(int bit = 0; bit < 16; ++bit) {
			if(sample & 0x8000) {
				XPCC_LOG_INFO << "1";
			} else {
				XPCC_LOG_INFO << "0";
			}
			sample <<= 1;
		}
		XPCC_LOG_INFO << xpcc::endl;
	}

	while (1)
	{
		Board::LedOrange::toggle();
		xpcc::delayMilliseconds(500);
	}

	return 0;
}
