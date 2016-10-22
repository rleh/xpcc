#include <xpcc/architecture/platform.hpp>
#include <xpcc/debug/logger.hpp>

struct systemClockSpi
{
	static constexpr int Spi1 = Board::systemClock::Spi2;
};

// Create an IODeviceWrapper around the Uart Peripheral we want to use
xpcc::IODeviceWrapper< Usart1, xpcc::IOBuffer::BlockIfFull > loggerDevice;

// Set all four logger streams to use the UART
xpcc::log::Logger xpcc::log::debug(loggerDevice);
xpcc::log::Logger xpcc::log::info(loggerDevice);
xpcc::log::Logger xpcc::log::warning(loggerDevice);
xpcc::log::Logger xpcc::log::error(loggerDevice);

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;

/* One WS2812B bit is represented by 3 SPI bits:
 * 0 -> 100
 * 1 -> 110
 * Each WS2812B: 24 bits -> 72 SPI bits -> 9 SPI bytes
 * 50µs reset: 120 bits @2.4MHz -> 15 Bytes
 * 50µs reset: 150 bits @3MHz   -> 20 Bytes
 *
 */
static constexpr int ledSpiResetLength = 20;
static constexpr int numLeds = 12;
static constexpr std::size_t ledSpiBufferLength = numLeds * 9 + ledSpiResetLength;
rgb_t ledBuffer[numLeds];
uint8_t ledSpiBuffer[ledSpiBufferLength];
uint8_t spiDummyReadBuffer[ledSpiBufferLength];

/* Uart baud rate:
 * WS2812B bit duration 1250ns <-> 800kHz
 * SPI bit frequency = 800 kHz * (3/8) * 8 = 2400 kHz
 *
 */
static constexpr int spiBaudRate = 2400 * 1000; // nearest SPI frequency: 3 MHz

static constexpr uint8_t wsHighBit = 0b110;
static constexpr uint8_t wsLowBit = 0b110;
void ledsToSpiBuffer(rgb_t* leds, uint8_t* spiBuffer, std::size_t numLeds) {
	for (unsigned int i = 0; i < numLeds; i++) {
		for (uint8_t b = 0; b < 9; b++) {
			spiBuffer[i+b] = 0;
		}

		leds[i].g & xpcc::Bit0 ? spiBuffer[i+0] |= (wsHighBit << 0) : spiBuffer[i+0] |= (wsLowBit << 0);

		leds[i].g & xpcc::Bit1 ? spiBuffer[i+0] |= (wsHighBit << 3) : spiBuffer[i+0] |= (wsLowBit << 3);

		leds[i].g & xpcc::Bit2 ? spiBuffer[i+0] |= (wsHighBit << 6) : spiBuffer[i+0] |= (wsLowBit << 6);
		leds[i].g & xpcc::Bit2 ? spiBuffer[i+1] |= (wsHighBit >> 2) : spiBuffer[i+1] |= (wsLowBit >> 2);

		leds[i].g & xpcc::Bit3 ? spiBuffer[i+1] |= (wsHighBit << 1) : spiBuffer[i+1] |= (wsLowBit << 1);

		leds[i].g & xpcc::Bit4 ? spiBuffer[i+1] |= (wsHighBit << 4) : spiBuffer[i+1] |= (wsLowBit << 4);

		leds[i].g & xpcc::Bit5 ? spiBuffer[i+1] |= (wsHighBit << 7) : spiBuffer[i+1] |= (wsLowBit << 7);
		leds[i].g & xpcc::Bit5 ? spiBuffer[i+2] |= (wsHighBit >> 1) : spiBuffer[i+2] |= (wsLowBit >> 1);

		leds[i].g & xpcc::Bit6 ? spiBuffer[i+2] |= (wsHighBit << 2) : spiBuffer[i+2] |= (wsLowBit << 2);

		leds[i].g & xpcc::Bit7 ? spiBuffer[i+2] |= (wsHighBit << 5) : spiBuffer[i+2] |= (wsLowBit << 5);


		leds[i].r & xpcc::Bit0 ? spiBuffer[i+3] |= (wsHighBit << 0) : spiBuffer[i+3] |= (wsLowBit << 0);

		leds[i].r & xpcc::Bit1 ? spiBuffer[i+3] |= (wsHighBit << 3) : spiBuffer[i+3] |= (wsLowBit << 3);

		leds[i].r & xpcc::Bit2 ? spiBuffer[i+3] |= (wsHighBit << 6) : spiBuffer[i+3] |= (wsLowBit << 6);
		leds[i].r & xpcc::Bit2 ? spiBuffer[i+4] |= (wsHighBit >> 2) : spiBuffer[i+4] |= (wsLowBit >> 2);

		leds[i].r & xpcc::Bit3 ? spiBuffer[i+4] |= (wsHighBit << 1) : spiBuffer[i+4] |= (wsLowBit << 1);

		leds[i].r & xpcc::Bit4 ? spiBuffer[i+4] |= (wsHighBit << 4) : spiBuffer[i+4] |= (wsLowBit << 4);

		leds[i].r & xpcc::Bit5 ? spiBuffer[i+4] |= (wsHighBit << 7) : spiBuffer[i+4] |= (wsLowBit << 7);
		leds[i].r & xpcc::Bit5 ? spiBuffer[i+5] |= (wsHighBit >> 1) : spiBuffer[i+5] |= (wsLowBit >> 1);

		leds[i].r & xpcc::Bit6 ? spiBuffer[i+5] |= (wsHighBit << 2) : spiBuffer[i+5] |= (wsLowBit << 2);

		leds[i].r & xpcc::Bit7 ? spiBuffer[i+5] |= (wsHighBit << 5) : spiBuffer[i+5] |= (wsLowBit << 5);


		leds[i].b & xpcc::Bit0 ? spiBuffer[i+6] |= (wsHighBit << 0) : spiBuffer[i+6] |= (wsLowBit << 0);

		leds[i].b & xpcc::Bit1 ? spiBuffer[i+6] |= (wsHighBit << 3) : spiBuffer[i+6] |= (wsLowBit << 3);

		leds[i].b & xpcc::Bit2 ? spiBuffer[i+6] |= (wsHighBit << 6) : spiBuffer[i+6] |= (wsLowBit << 6);
		leds[i].b & xpcc::Bit2 ? spiBuffer[i+7] |= (wsHighBit >> 2) : spiBuffer[i+7] |= (wsLowBit >> 2);

		leds[i].b & xpcc::Bit3 ? spiBuffer[i+7] |= (wsHighBit << 1) : spiBuffer[i+7] |= (wsLowBit << 1);

		leds[i].b & xpcc::Bit4 ? spiBuffer[i+7] |= (wsHighBit << 4) : spiBuffer[i+7] |= (wsLowBit << 4);

		leds[i].b & xpcc::Bit5 ? spiBuffer[i+7] |= (wsHighBit << 7) : spiBuffer[i+7] |= (wsLowBit << 7);
		leds[i].b & xpcc::Bit5 ? spiBuffer[i+8] |= (wsHighBit >> 1) : spiBuffer[i+8] |= (wsLowBit >> 1);

		leds[i].b & xpcc::Bit6 ? spiBuffer[i+8] |= (wsHighBit << 2) : spiBuffer[i+8] |= (wsLowBit << 2);

		leds[i].b & xpcc::Bit7 ? spiBuffer[i+8] |= (wsHighBit << 5) : spiBuffer[i+8] |= (wsLowBit << 5);
	}
	for (unsigned int j = (ledSpiBufferLength - ledSpiResetLength); j < ledSpiBufferLength; j++) {
		spiBuffer[j] = 0x00;
	}
}

int
main()
{
	Board::initialize();

	Board::LedUp::set();

	for (unsigned int i = 0; i < numLeds; i++) {
		ledBuffer[i].r = 0x33;
		ledBuffer[i].g = 0x33;
		ledBuffer[i].b = 0x33;
	}
	ledsToSpiBuffer(ledBuffer, ledSpiBuffer, numLeds);

	// Initialize Usart
	GpioOutputA9::connect(Usart1::Tx);
	GpioInputA10::connect(Usart1::Rx, Gpio::InputType::PullUp);
	Usart1::initialize<Board::systemClock, 115200>(12);

	GpioOutputB15::connect(SpiMaster2::Mosi);
	GpioOutputB13::connect(SpiMaster2::Sck); // debug
	SpiMaster2::initialize<Board::systemClock, spiBaudRate, xpcc::Tolerance::DontCare>();

	while (1)
	{
		XPCC_LOG_INFO << "Loop" << xpcc::endl;

		Board::LedUp::toggle();
		Board::LedDown::toggle();

		//RF_CALL_BLOCKING(SpiMaster2::transfer(ledSpiBuffer, reinterpret_cast<uint8_t*>(0), ledSpiBufferLength));
		//SpiMaster2::transferBlocking(ledSpiBuffer, spiDummyReadBuffer, ledSpiBufferLength);
		for (unsigned int i = 0; i < ledSpiBufferLength; i++) {
			SpiMaster2::transferBlocking(ledSpiBuffer[i]);
		}

		xpcc::delayMilliseconds(5);
	}

	return 0;
}
