#ifndef SWIFTY_SYNC_UNIVERSAL_CLIENT
#define SWIFTY_SYNC_UNIVERSAL_CLIENT

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include "Codable.h"
#include "JSON.h"
#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_client> client;

using namespace std;

#define PRINT_LOG [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl;  }

class SwiftyUniversalClient {
	client cli;

	void run() {
		
	}

	SwiftyUniversalClient() {}

	~SwiftyUniversalClient() {}
};

#endif