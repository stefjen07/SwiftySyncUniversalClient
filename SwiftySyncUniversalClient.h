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

#define RESPOND_WAIT_INTERVAL 200

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

	bool send(string content) {
		websocketpp::lib::error_code ec;

		c.send(con->get_handle(), content.c_str(), websocketpp::frame::opcode::text, ec);
		if (ec) {
			cout << "Unable to send message: " << ec.message() << "\n";
			return false;
		}
		return true;
	}

	string sendRequest(string id, string request) {
		send(request);
		cout << "Request with id " << id << " sent\n";
		while (responds.find(id) == responds.end()) {
            sleep(RESPOND_WAIT_INTERVAL);
		}
		string result = responds[id];
		responds.erase(id);
		return result;
	}

	Field get_field(string collectionName, string documentName, vector<string> path) {
		string request = REQUEST_PREFIX;
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
		string respond = sendRequest(dataRequest.id, request);
		unsigned prefixSize = strlen(REQUEST_PREFIX) + strlen(DATA_REQUEST_PREFIX) + UUID_SIZE;
		string encoded = respond.substr(prefixSize, respond.length() - prefixSize);
		JSONDecoder decoder;
		auto container = decoder.container(encoded);
		return container.decode(Field());
	}

	bool set_field(string collectionName, string documentName, vector<string> path, string value) {
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
		string respond = sendRequest(dataRequest.id, request);
		return respond.find(string(REQUEST_PREFIX) + string(DATA_REQUEST_FAILURE)) != 0;
	}

	vector<Field> get_document(string collectionName, string documentName) {
		string request = REQUEST_PREFIX;
		request += DOCUMENT_GET_PREFIX;
		auto dataRequest = DataRequest(collectionName, documentName, "");
		dataRequest.type = RequestType::documentGet;
		JSONEncoder encoder;
		auto container = encoder.container();
		container.encode(dataRequest);
		request += container.content;
		string respond = sendRequest(dataRequest.id, request);
		unsigned prefixSize = strlen(REQUEST_PREFIX) + strlen(DATA_REQUEST_PREFIX) + UUID_SIZE;
		string encoded = respond.substr(prefixSize, respond.length() - prefixSize);
		JSONDecoder decoder;
		auto decodeContainer = decoder.container(encoded);
		auto result = Document();
		return decodeContainer.decode(vector<Field>());
	}

	bool set_document(Document document) {
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
		string respond = sendRequest(dataRequest.id, request);
		return respond.find(string(REQUEST_PREFIX) + string(DATA_REQUEST_FAILURE)) != 0;
	}

	DataUnit call_function(string name, DataUnit input) {
		string request = REQUEST_PREFIX;
		request += FUNCTION_REQUEST_PREFIX;
		JSONEncoder encoder;
		auto container = encoder.container();
		auto functionRequest = FunctionRequest(name, input);
		container.encode(functionRequest);
		request += container.content;
		unsigned prefixSize = strlen(REQUEST_PREFIX) + strlen(FUNCTION_REQUEST_PREFIX) + UUID_SIZE;
		string respond = sendRequest(functionRequest.id, request);
		respond = respond.substr(prefixSize, respond.length() - prefixSize);
		JSONDecoder decoder;
		auto decodeContainer = decoder.container(respond);
		return decodeContainer.decode(DataUnit());
	}

	void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		string respond = msg->get_payload();

		if (respond.find(AUTH_PREFIX) == 0) {
			handleAuthRespond(respond.substr(strlen(AUTH_PREFIX), respond.length() - strlen(AUTH_PREFIX)));
		}

		if (respond.find(REQUEST_PREFIX) == 0) {
			if (respond.find(DATA_REQUEST_PREFIX) == strlen(REQUEST_PREFIX)) {
				unsigned prefixSize = strlen(REQUEST_PREFIX) + strlen(DATA_REQUEST_PREFIX);
				string id = respond.substr(prefixSize, UUID_SIZE);
				cout << "Received response for data request with id " << id << "\n";
				responds[id] = respond;
			}
			if (respond.find(FUNCTION_REQUEST_PREFIX) == strlen(REQUEST_PREFIX)) {
				unsigned prefixSize = strlen(REQUEST_PREFIX) + strlen(FUNCTION_REQUEST_PREFIX);
				string id = respond.substr(prefixSize, UUID_SIZE);
				cout << "Received response for function request with id " << id << "\n";
				responds[id] = respond;
			}
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
