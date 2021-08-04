#define CLIENT

#include "c_api_storage_helper.hpp"
#include "c_api_connector.h"
#include "FacebookAuthorization.hpp"
#include "GoogleAuthorization.hpp"
#include "SwiftySyncUniversalClient.hpp"
#include "JSON.hpp"
#include "Data.hpp"
#include <iostream>
#include <stdlib.h>

class DebugProvider : public AuthorizationProvider {
public:
    std::string generateRequest(std::string body) {
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

    void create_client(const char *uri) {
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
                    std::cout << "Authorized successfully\n";
                }
            };
        }
    }

    void run_client() {
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

    CFieldArray* CFieldArray_new(size_t size) {
        CField* arr = (CField*) malloc(sizeof(CField) * size);
        CFieldArray* result = (CFieldArray*) malloc(sizeof(CFieldArray));
        result->ptr = arr;
        result->size = size;
        return result;
    }

    size_t CField_size() {
        return sizeof(CField);
    }

    char* allocate_string(char* ptr, size_t size) {
        size_t bytes = sizeof(char) * (size + 1);
        char* result = (char*) malloc(bytes);
        memcpy(result, ptr, bytes);
        result[size] = '\0';
        return result;
    }

    CField *get_array_child(struct CFieldArray array, const char *key) {
        for(int i=0;i<array.size;i++) {
            if(strcmp(array.ptr[i].name,key) == 0) {
                return &array.ptr[i];
            }
        }
        return NULL;
    }

    CField* get_field(const char* collectionName, const char* documentName, const char* path) {
        if (client_instance == NULL) {
            exit(-1);
        }
        JSONDecoder decoder;
        auto container = decoder.container(path);
        auto decodedPath = container.decode(std::vector<std::string>());
        auto field = client_instance->get_field(collectionName, documentName, decodedPath);
        return CField_fromField(&field);
    }

    bool set_field(const char* collectionName, const char* documentName, const char* path, CField* value) {
        if (client_instance == NULL) {
            exit(-1);
        }
        JSONDecoder decoder;
        auto container = decoder.container(path);
        auto decodedPath = container.decode(std::vector<std::string>());
        auto field = client_instance->get_field(collectionName, documentName, decodedPath);
        return client_instance->set_field(collectionName, documentName, decodedPath, *Field_fromCField(value));
    }

    CFieldArray get_document(const char* collectionName, const char* documentName) {
        if (client_instance == NULL) {
            exit(-1);
        }
        auto document = client_instance->get_document(collectionName, documentName);
        CField* arr = (CField*) malloc(sizeof(CField) * document.size());
        for(int i=0;i<document.size();i++) {
            copy(&arr[i], CField_fromField(&document[i]));
        }
        CFieldArray result;
        result.ptr = arr;
        result.size = document.size();
        return result;
    }

    bool set_document(const char* collectionName, const char* documentName, CFieldArray fields) {
        if (client_instance == NULL) {
            exit(-1);
        }
        auto document = Document();
        document.name = documentName;
        auto collection = Collection(collectionName);
        document.collection = &collection;
        for(int i=0;i<fields.size;i++) {
            document.fields.push_back(*Field_fromCField(&fields.ptr[i]));
        }
        return client_instance->set_document(document);
    }

    const char* call_function(const char* name, const char* bytes) {
        if (client_instance == NULL) {
            exit(-1);
        }
        std::string sBytes = bytes;
        std::vector<char> vBytes(sBytes.length());
        for(int i=0; i<sBytes.length();i++) {
            vBytes[i] = sBytes[i];
        }
        auto input = DataUnit(vBytes);
        auto unit = client_instance->call_function(name, input);
        static std::string sUnit;
        for(int i=0;i<unit.bytes.size();i++) {
            sUnit += unit.bytes[i];
        }
        return sUnit.c_str();
    }

#ifdef __cplusplus
}
#endif
