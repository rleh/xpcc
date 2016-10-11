#include <xpcc/architecture/platform.hpp>


#include <xpcc/driver/inertial/lis3dsh.hpp>

// ----------------------------------------------------------------------------
/**
 * Very basic example of USART usage.
 * The ASCII sequence 'A', 'B', 'C, ... , 'Z', 'A', 'B', 'C', ...
 * is printed with 9600 baud, 8N1 at pin PA2.
 */

constexpr std::size_t textLength = 42;
uint8_t text[textLength] = "Hello World. This string is sent via DMA.";

int
main()
{
	Board::initialize();

	Board::LedRed::set();


	Dma1::enable();
	Dma1::Stream6::stop();

	GpioOutputA2::connect(DmaUsart2<Dma1::Stream6>::Tx);
	GpioInputA3::connect(DmaUsart2<Dma1::Stream6>::Rx, Gpio::InputType::PullUp);
	DmaUsart2<Dma1::Stream6>::initialize<Board::systemClock, 9600>();

	//Usart2::initialize<Board::systemClock, 9600>(12);

	//Dma1::Stream6::setChannel(DmaBase::Channel::Channel4);

	volatile uint32_t debugUartSr = 42; // USART_SR_TXE
	volatile uint32_t debugDmaCr = 42; // & DMA_SxCR_EN
	volatile uint32_t debugReturn = 0; // & DMA_SxCR_EN

	while (1)
	{
		debugReturn = 0;
		//Usart2::write(text, textLength);
		while(!debugReturn) {
			debugReturn = DmaUsart2<Dma1::Stream6>::write(text, textLength);

			debugUartSr = USART2->SR & USART_SR_TXE;
			debugDmaCr = DMA1_Stream6->CR & DMA_SxCR_EN;
		}

		xpcc::delayMilliseconds(500);
	}

	return 0;
}
