#ifndef SWIFTY_SYNC_UNIVERSAL_CLIENT
#define SWIFTY_SYNC_UNIVERSAL_CLIENT

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include "Codable.h"
#include "JSON.h"
#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_client> client;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using namespace std;

typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

namespace {
	void on_message(client* c, websocketpp::connection_hdl hdl, message_ptr msg) {
		websocketpp::lib::error_code ec;

		c->send(hdl, msg->get_payload(), msg->get_opcode(), ec);
		if (ec) {
			cout << "Message send failed: " << ec.message() << "\n";
		}
	}
}

class SwiftyUniversalClient {
public:
	client c;

	SwiftyUniversalClient(string uri) {
		try {
			c.set_access_channels(websocketpp::log::alevel::all);
			c.clear_access_channels(websocketpp::log::alevel::frame_payload);

			c.init_asio();

			c.set_message_handler(bind(&on_message, &c, ::_1, ::_2));

			websocketpp::lib::error_code ec;
			client::connection_ptr con = c.get_connection(uri, ec);
			if (ec) {
				cout << "Could not create connection: " << ec.message() << "\n";
				return;
			}

			c.connect(con);

			c.run();
		}
		catch (websocketpp::exception const& e) {
			cout << e.what() << "\n";
		}
	}

	~SwiftyUniversalClient() {}
};

#endif