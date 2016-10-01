#include <xpcc/architecture/platform.hpp>
#include <xpcc/debug/logger.hpp>

// Set the log level
#undef	XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::INFO

xpcc::IODeviceWrapper< Usart2, xpcc::IOBuffer::BlockIfFull > loggerDevice;
xpcc::log::Logger xpcc::log::info(loggerDevice);

// Settings
static constexpr uint32_t Channels = 4;
static constexpr uint32_t Samples = 64; // per channel

static constexpr uint32_t BufferLength = Samples * Channels;
uint16_t buffer[Samples][Channels];

int
main()
{
	Board::systemClock::enable();

	// initialize Uart2 for XPCC_LOG_INFO
	GpioOutputA2::connect(Usart2::Tx);
	GpioInputA3::connect(Usart2::Rx);
	Usart2::initialize<Board::systemClock, 115200>(12);

	XPCC_LOG_INFO << xpcc::endl;
	XPCC_LOG_INFO << "# Number of Channels:  " << Channels << xpcc::endl;
	XPCC_LOG_INFO << "# Samples per Channel: " << Samples << xpcc::endl;

	// initialize Adc1
	Adc1::initialize<Board::systemClock, MHz10>();
	GpioInputA7::connect(Adc1::Channel7);
	GpioInputB0::connect(Adc1::Channel8);
	GpioInputB1::connect(Adc1::Channel9);
	GpioInputC0::connect(Adc1::Channel10);
	Adc1::setChannel(GpioInputA7::Adc1Channel, Adc1::SampleTime::Cycles3);
	Adc1::addChannel(GpioInputB0::Adc1Channel, Adc1::SampleTime::Cycles3);
	Adc1::addChannel(GpioInputB1::Adc1Channel, Adc1::SampleTime::Cycles3);
	Adc1::addChannel(GpioInputC0::Adc1Channel, Adc1::SampleTime::Cycles3);

	// enable scan mode
	ADC1->CR1 |= ADC_CR1_SCAN;

	// initialize DMA
	Dma2::enable();
	Dma2::Stream0::stop();
	Dma2::Stream0::configure(DmaBase::Channel::Channel0, BufferLength, DmaBase::Priority::VeryHigh);
	Dma2::Stream0::setPeripheralSource(reinterpret_cast<uint16_t*>(const_cast<uint32_t*>(&(ADC1->DR))));
	Dma2::Stream0::setMemoryDestination(&buffer[0][0]);
	Dma2::Stream0::start();

	// enable DMA
	ADC1->CR2 |= ADC_CR2_DMA;

	Adc1::enableFreeRunningMode();
	Adc1::startConversion();

	// Wait for DMA to fill up
	while(!Dma2::Stream0::isFinished());

	// Print results
	for(uint32_t channel = 0; channel < Channels; ++channel){
		XPCC_LOG_INFO << "Channel " << channel;
		XPCC_LOG_INFO <<" -------------------------------------------------" << xpcc::endl;
		for(uint32_t sample = 0; sample < Samples; ++sample) {
			int v = buffer[sample][channel] * 20 / 0xfff;
			for(uint8_t jj = 0; jj < v; ++jj) {
				XPCC_LOG_INFO << "#";
			}
			XPCC_LOG_INFO << xpcc::endl;
		}
	}

	while (1){}

	return 0;
}
