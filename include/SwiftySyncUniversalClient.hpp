#ifndef SWIFTY_SYNC_UNIVERSAL_CLIENT
#define SWIFTY_SYNC_UNIVERSAL_CLIENT

#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_
#define ASIO_STANDALONE

#ifdef SSL_CLIENT
#include <websocketpp/config/asio_client.hpp>
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#endif
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <Authorization.hpp>
#include <SwiftySyncStorage.hpp>
#include <Codable.hpp>
#include <JSON.hpp>
#include <Request.hpp>
#include <UUID.hpp>
#include <functional>
#include <vector>
#include <map>
#include <iostream>

#ifndef RESPOND_WAIT_INTERVAL
#define RESPOND_WAIT_INTERVAL 200
#endif

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class SwiftyUniversalClient {
public:
	client c;
	client::connection_ptr con;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread;

	std::vector<AuthorizationProvider*> supportedProviders;
	std::string uri;
	std::function<void(AuthorizationStatus)> authHandler;

	std::map<std::string, std::string> responds;

	bool authorized = false;

	bool send(std::string content);
	std::string sendRequest(std::string id, std::string request);
	std::string waitForMessage();

	Field get_field(std::string collectionName, std::string documentName, std::vector<std::string> path);
	bool set_field(std::string collectionName, std::string documentName, std::vector<std::string> path, Field value);
	std::vector<Field> get_document(std::string collectionName, std::string documentName);
	bool set_document(Document document);

	DataUnit call_function(std::string name, DataUnit input);

	void on_message(websocketpp::connection_hdl hdl, message_ptr msg);

	void authorize(int providerIndex, std::string credentials);

	void handleAuthRespond(std::string body);

	void run();

	SwiftyUniversalClient(std::string uri) {
		this->uri = uri;
	}

	~SwiftyUniversalClient() {
		c.stop_perpetual();
	}
};

#endif
