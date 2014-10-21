#include <xpcc/architecture/driver/delay.hpp>
#include <xpcc/communication.hpp>
#include <xpcc_config.hpp>


#ifdef USE_TCPIP
#include <xpcc/communication/xpcc/backend/tcpip/tcpip.hpp>
xpcc::TcpIpConnector connector;
#endif

#ifdef USE_TIPC
#include <xpcc/communication/xpcc/backend/tipc/tipc.hpp>
xpcc::TipcConnector connector;
#endif

#include <xpcc/debug/logger.hpp>

// set new log level
#undef XPCC_LOG_LEVEL
#define	XPCC_LOG_LEVEL xpcc::log::DEBUG

#include "component_receiver/receiver.hpp"


#include "../communication/postman.hpp"
#include "../communication/identifier.hpp"



// create an instance of the generated postman
Postman postman;

xpcc::Dispatcher dispatcher(&connector, &postman);


namespace component
{
	Receiver receiver(robot::component::RECEIVER, &dispatcher);
}

int
main(void)
{
	
	XPCC_LOG_INFO << "Welcome to the communication test!" << xpcc::endl;

#ifdef USE_TCPIP
	connector.connect("127.0.0.1", 7666);
#endif

	connector.addReceiverId(robot::component::RECEIVER);
	 
	XPCC_LOG_INFO << "Component Receiver"<< xpcc::endl;
	
	while (1)
	{
		// deliver received messages
		dispatcher.update();
		
		component::receiver.update();
		
		xpcc::delayMicroseconds(100);
	}
}
