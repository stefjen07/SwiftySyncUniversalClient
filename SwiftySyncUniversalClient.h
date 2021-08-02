#ifndef SWIFTY_SYNC_UNIVERSAL_CLIENT
#define SWIFTY_SYNC_UNIVERSAL_CLIENT

#include <Usage.h>

#ifdef SSL_CLIENT
#include <websocketpp/config/asio_client.hpp>
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#endif
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <Authorization.h>
#include <SwiftySyncStorage.h>
#include <Codable.h>
#include <JSON.h>
#include <Request.h>
#include <UUID.h>
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
using namespace std;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

class SwiftyUniversalClient {
public:
	client c;
	client::connection_ptr con;
	websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread;

	vector<AuthorizationProvider*> supportedProviders;
	string uri;
	function<void(AuthorizationStatus)> authHandler;

	map<string, string> responds;

	bool authorized = false;

	bool send(string content);

	string sendRequest(string id, string request);

	Field get_field(string collectionName, string documentName, vector<string> path);

	bool set_field(string collectionName, string documentName, vector<string> path, string value);

	vector<Field> get_document(string collectionName, string documentName);

	bool set_document(Document document);

	DataUnit call_function(string name, DataUnit input);

	void on_message(websocketpp::connection_hdl hdl, message_ptr msg);

	void authorize(int providerIndex, string credentials);

	void handleAuthRespond(string body);

	void run();

	SwiftyUniversalClient(string uri) {
		this->uri = uri;
	}

	~SwiftyUniversalClient() {
		c.stop_perpetual();
	}
};

#endif
