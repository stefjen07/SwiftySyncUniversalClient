#include "c_api_connector.h"
#include "Authorization.h"
#include "FacebookAuthorization.h"
#include "GoogleAuthorization.h"
#include "SwiftySyncUniversalClient.h"
#include "Codable.h"
#include "JSON.h"
#include "Data.h"
#include <string>

class DebugProvider : public AuthorizationProvider {
public:
    string generateRequest(string body) {
        return "DEBUG";
    }

    DebugProvider() {}
};

#ifdef __cplusplus
extern "C" {
#endif

    static SwiftyUniversalClient *client_instance = NULL;
    static FacebookProvider facebookProvider = FacebookProvider();
    static GoogleProvider googleProvider = GoogleProvider();
    static DebugProvider debugProvider = DebugProvider();

    void createClient(const char *uri) {
        if (client_instance == NULL) {
            client_instance = new SwiftyUniversalClient(uri);

            auto castedFacebookProvider = dynamic_cast<AuthorizationProvider*>(&facebookProvider);
            auto castedGoogleProvider = dynamic_cast<AuthorizationProvider*>(&googleProvider);
            auto castedDebugProvider = dynamic_cast<AuthorizationProvider*>(&debugProvider);

            client_instance->supportedProviders = {
                    castedFacebookProvider,
                    castedGoogleProvider,
                    castedDebugProvider
            };

            client_instance->authHandler = [](AuthorizationStatus status) {
                if (status == AuthorizationStatus::authorized) {
                    cout << "Authorized successfully\n";
                }
            };
        }
    }

    void runClient() {
        if (client_instance != NULL) {
            client_instance->run();
        }
    }

    void authorize(unsigned provider, const char *credentials) {
        if (client_instance == NULL) {
            return;
        }
        client_instance->authorize(provider, credentials);
    }

    bool authorized() {
        return client_instance->authorized;
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
        static string result = encodeContainer.content;
        return result.c_str();
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
        static string result = container.content;
        return result.c_str();
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
        static string sUnit;
        for(int i=0;i<unit.bytes.size();i++) {
            sUnit += unit.bytes[i];
        }
        return sUnit.c_str();
    }

#ifdef __cplusplus
}
#endif