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
#include <Request.h>
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

	bool authorized = false;

	bool send(string content) {
		websocketpp::lib::error_code ec;

		c.send(con->get_handle(), content.c_str(), websocketpp::frame::opcode::text, ec);
		if (ec) {
			cout << "Unable to send message: " << ec.message() << "\n";
			return false;
		}
		return true;
	}

	Field get_field(string collectionName, string documentName, vector<string> path) {
		string request = REQUEST_PREFIX;
		request += DATA_REQUEST_PREFIX;
		request += FIELD_GET_PREFIX;
		JSONEncoder encoder;
		auto fieldContainer = encoder.container();
		auto fieldRequest = FieldRequest("", path);
		fieldContainer.encode(fieldRequest);
		auto dataRequest = DataRequest(collectionName, documentName, fieldContainer.content);
		dataRequest.type = RequestType::fieldGet;
		auto dataContainer = encoder.container();
		dataContainer.encode(dataRequest);
		request += dataContainer.content;
		send(request);
		exit(1); //Unimplemented
	}

	void set_field(string collectionName, string documentName, vector<string> path, string value) {
		string request = REQUEST_PREFIX;
		request += FIELD_SET_PREFIX;
		JSONEncoder encoder;
		auto fieldContainer = encoder.container();
		auto fieldRequest = FieldRequest(value, path);
		fieldContainer.encode(fieldRequest);
		auto dataRequest = DataRequest(collectionName, documentName, fieldContainer.content);
		dataRequest.type = RequestType::fieldSet;
		auto dataContainer = encoder.container();
		dataContainer.encode(dataRequest);
		request += dataContainer.content;
		send(request);
	}

	Document get_document(string collectionName, string documentName) {
		string request = REQUEST_PREFIX;
		request += DOCUMENT_GET_PREFIX;
		auto dataRequest = DataRequest(collectionName, documentName, "");
		dataRequest.type = RequestType::documentGet;
		JSONEncoder encoder;
		auto container = encoder.container();
		container.encode(dataRequest);
		request += container.content;
		send(request);
		exit(1); //Unimplemented
	}

	void set_document(Document document) {
		string request = REQUEST_PREFIX;
		request += DOCUMENT_SET_PREFIX;
		JSONEncoder encoder;
		auto container = encoder.container();
		container.encode(document.fields);
		auto dataRequest = DataRequest(document.collection->name, document.name, container.content);
		dataRequest.type = RequestType::documentSet;
		auto requestContainer = encoder.container();
		requestContainer.encode(dataRequest);
		request += requestContainer.content;
		send(request);
	}

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
		send(request);
	}

	void handleAuthRespond(string body) {
		AuthorizationStatus status;
		if (body == AUTHORIZED_LOCALIZE) {
			status = AuthorizationStatus::authorized;
			authorized = true;
		}
		else {
			authorized = false;
			if (body == CORR_CRED_LOCALIZE) {
				status = AuthorizationStatus::corruptedCredentials;
			}
			else if (body == AUTH_ERR_LOCALIZE) {
				status = AuthorizationStatus::error;
			}
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