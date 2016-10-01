#include <xpcc/architecture/platform.hpp>
#include <xpcc/architecture.hpp>

// Board has logger
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
	Board::initialize();

	Board::LedD13::setOutput(xpcc::Gpio::Low);

	XPCC_LOG_INFO << xpcc::endl;
	XPCC_LOG_INFO << "Sampling freqeuncy: " << kHz(SampleFrequency) << "kHz" << xpcc::endl;
	XPCC_LOG_INFO << "Sample length: " << SampleLength << xpcc::endl;

	// Init Gpio Inputs
	GpioInputB0::setInput(Gpio::InputType::Floating);
	GpioInputB1::setInput(Gpio::InputType::Floating);
	GpioInputB2::setInput(Gpio::InputType::Floating);
	GpioInputB3::setInput(Gpio::InputType::Floating);
	GpioInputB4::setInput(Gpio::InputType::Floating);
	GpioInputB5::setInput(Gpio::InputType::Floating);
	GpioInputB6::setInput(Gpio::InputType::Floating);
	GpioInputB7::setInput(Gpio::InputType::Floating);
	GpioInputB8::setInput(Gpio::InputType::Floating);
	GpioInputB9::setInput(Gpio::InputType::Floating);
	GpioInputB10::setInput(Gpio::InputType::Floating);
	GpioInputB11::setInput(Gpio::InputType::Floating);
	GpioInputB12::setInput(Gpio::InputType::Floating);
	GpioInputB13::setInput(Gpio::InputType::Floating);
	GpioInputB14::setInput(Gpio::InputType::Floating);
	GpioInputB15::setInput(Gpio::InputType::Floating);


	for(uint32_t i = 0; i < SampleLength; ++i) {
		samples[i] = 0xffff;
	}

	Dma1::enable();
	Dma1::Stream6::stop();
	Dma1::Stream6::configure(SampleLength, DmaBase::Priority::VeryHigh, DmaBase::CircularMode::Disabled);
	Dma1::Stream6::setPeripheralSource(reinterpret_cast<uint16_t*>(const_cast<uint32_t*>(&(GPIOB->IDR))));
	Dma1::Stream6::setMemoryDestination(&samples[0]);

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


	Board::LedD13::set();
	Dma1::Stream6::start();
	while(!Dma1::Stream6::isFinished());
	Board::LedD13::reset();

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

	xpcc::delayMilliseconds(1000);

	while (1)
	{
		Board::LedD13::toggle();
		xpcc::delayMilliseconds(500);
	}

	return 0;
}
