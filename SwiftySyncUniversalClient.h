#ifndef SWIFTY_SYNC_UNIVERSAL_CLIENT
#define SWIFTY_SYNC_UNIVERSAL_CLIENT

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <Authorization.h>
#include <SwiftySyncStorage.h>
#include <Codable.h>
#include <JSON.h>
#include <functional>
#include <vector>
#include <iostream>

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

	void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		string respond = msg->get_payload();

		if (respond.find(AUTH_PREFIX) == 0) {
			handleAuthRespond(respond.substr(strlen(AUTH_PREFIX), respond.length() - strlen(AUTH_PREFIX)));
		}
	}

	void authorize(int providerIndex, string credentials) {
		string request = AUTH_PREFIX;
		auto provider = supportedProviders[providerIndex];
		request += provider->generateRequest(credentials);
		websocketpp::lib::error_code ec;
		c.send(con->get_handle(), &request, request.length(), websocketpp::frame::opcode::text, ec);
		if (ec) {
			cout << ec.message();
		}
	}

	void handleAuthRespond(string body) {
		AuthorizationStatus status;
		if (body == AUTHORIZED_LOCALIZE) {
			status = AuthorizationStatus::authorized;
		}
		else if (body == CORR_CRED_LOCALIZE) {
			status = AuthorizationStatus::corruptedCredentials;
		}
		else if (body == AUTH_ERR_LOCALIZE) {
			status = AuthorizationStatus::error;
		}
		authHandler(status);
	}

	void run() {
		try {
			c.set_access_channels(websocketpp::log::alevel::all);
			c.clear_access_channels(websocketpp::log::alevel::frame_payload);

			c.init_asio();

			c.set_message_handler([this](websocketpp::connection_hdl hdl, message_ptr msg) {
				on_message(hdl, msg);
			});

			websocketpp::lib::error_code ec;
			con = c.get_connection(uri, ec);
			if (ec) {
				cout << "Could not create connection: " << ec.message() << "\n";
				return;
			}

			c.connect(con);

			c.start_perpetual();

			thread.reset(new websocketpp::lib::thread(&client::run, &c));
		}
		catch (websocketpp::exception const& e) {
			cout << e.what() << "\n";
		}
	}

	SwiftyUniversalClient(string uri) {
		this->uri = uri;
	}

	~SwiftyUniversalClient() {
		c.stop_perpetual();
	}
};

#endif