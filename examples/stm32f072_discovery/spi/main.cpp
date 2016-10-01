#include <xpcc/architecture/platform.hpp>

uint8_t dataArray[5] = {0x33, 0xFF, 0x33, 0x00, 0x33};

struct spi1SystemClock
{
	static constexpr int Frequency = MHz48;
	static constexpr int Usart1 = Frequency;
	static constexpr int Can1 = Frequency;
	static constexpr int Spi2 = Frequency;
	static constexpr int Spi1 = Frequency; // added

	/* static bool inline
	enable()
	{
		// Enable the interal 48MHz clock
		ClockControl::enableInternalClockMHz48();
		// set flash latency for 48MHz
		ClockControl::setFlashLatency(Frequency);
		// Switch to the 48MHz clock
		ClockControl::enableSystemClock(ClockControl::SystemClockSource::InternalClockMHz48);
		// update frequencies for busy-wait delay functions
		xpcc::clock::fcpu     = Frequency;
		xpcc::clock::fcpu_kHz = Frequency / 1000;
		xpcc::clock::fcpu_MHz = Frequency / 1000000;
		xpcc::clock::ns_per_loop = ::round(4000 / (Frequency / 1000000));

		return true;
	} */
};

int
main()
{
	Board::initialize();


	// Enable SPI 2
	GpioOutputB12::connect(SpiMaster2::Nss);
	GpioOutputB15::connect(SpiMaster2::Mosi);
	GpioInputB14::connect(SpiMaster2::Miso);
	GpioOutputB13::connect(SpiMaster2::Sck);
	SpiMaster2::initialize<Board::systemClock, kHz125, xpcc::Tolerance::DontCare>();


	// Enable SPI 1
	GpioOutputA15::connect(SpiMaster1::Nss);
	GpioOutputA7::connect(SpiMaster1::Mosi);
	GpioInputA6::connect(SpiMaster1::Miso);
	GpioOutputA5::connect(SpiMaster1::Sck);
	SpiMaster1::initialize<spi1SystemClock, kHz125, xpcc::Tolerance::DontCare>();

	while (1)
	{
	/*	SpiMaster2::transferBlocking(dataArray, static_cast<uint8_t*>(0), 5);
		Board::LedDown::toggle();
		xpcc::delayMilliseconds(2);
	*/
		SpiMaster1::transferBlocking(dataArray, static_cast<uint8_t*>(0), 5);
		Board::LedUp::toggle();
		xpcc::delayMilliseconds(2);
	}

	return 0;
}
