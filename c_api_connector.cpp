#include "SwiftySyncUniversalClient.h"
#include "Codable.h"
#include "JSON.h"
#include "Data.h"
#include <string>
#ifdef __cplusplus
extern "C" {
#endif

static SwiftyUniversalClient *client_instance = NULL;

void createClient(const char *uri) {
	if (client_instance == NULL) {
		client_instance = new SwiftyUniversalClient(uri);
	}
}

void authorize(unsigned provider, const char *credentials) {
	if (client_instance == NULL) {
		return;
	}
	client_instance->authorize(provider, credentials);
}

const char* get_field(const char* collectionName, const char* documentName, const char* path) {
	if (client_instance == NULL) {
		exit(-1);
	}
	JSONDecoder decoder;
	auto container = decoder.container(path);
	auto decodedPath = container.decode(vector<string>());
	auto field = client_instance->get_field(collectionName, documentName, decodedPath);
	JSONEncoder encoder;
	auto encodeContainer = encoder.container();
	encodeContainer.encode(field);
	return encodeContainer.content.c_str();
}

bool set_field(const char* collectionName, const char* documentName, const char* path, const char* value) {
	if (client_instance == NULL) {
        exit(-1);
	}
    JSONDecoder decoder;
    auto container = decoder.container(path);
    auto decodedPath = container.decode(vector<string>());
    auto field = client_instance->get_field(collectionName, documentName, decodedPath);
	return client_instance->set_field(collectionName, documentName, decodedPath, value);
}

const char* get_document(const char* collectionName, const char* documentName) {
	if (client_instance == NULL) {
        exit(-1);
	}
	auto document = client_instance->get_document(collectionName, documentName);
	JSONEncoder encoder;
	auto container = encoder.container();
	container.encode(document);
	return container.content.c_str();
}

bool set_document(const char* collectionName, const char* documentName, const char* content) {
	if (client_instance == NULL) {
        exit(-1);
	}
	auto document = Document();
	return client_instance->set_document(document);
}

const char* call_function(const char* name, const char* bytes) {
	if (client_instance == NULL) {
        exit(-1);
	}
    string sBytes = bytes;
	vector<char> vBytes(sBytes.length());
	for(int i=0; i<sBytes.length();i++) {
	    vBytes[i] = sBytes[i];
	}
	auto input = DataUnit(vBytes);
	auto unit = client_instance->call_function(name, input);
	string sUnit;
	for(int i=0;i<unit.bytes.size();i++) {
	    sUnit += unit.bytes[i];
	}
	return sUnit.c_str();
}

#ifdef __cplusplus
}
#endif