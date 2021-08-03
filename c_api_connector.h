#ifndef C_API_CONNECTOR_H
#define C_API_CONNECTOR_H

#include <stdbool.h>
#include "c_api_storage.h"

#ifdef __cplusplus
extern "C" {
#endif

    struct CFieldArray {
        struct CField* ptr;
        size_t size;
    };

    struct CFieldArray* CFieldArray_new(size_t size);
    size_t CField_size();
    char* allocate_string(char* ptr, size_t size);

    struct CField* get_array_child(struct CFieldArray array, const char* key);

    void create_client(const char *uri);
    void run_client();
    void authorize(unsigned provider, const char *credentials);
    bool authorized();
    struct CField* get_field(const char* collectionName, const char* documentName, const char* path);
    bool set_field(const char* collectionName, const char* documentName, const char* path, struct CField* value);
    struct CFieldArray get_document(const char* collectionName, const char* documentName);
    bool set_document(const char* collectionName, const char* documentName, struct CFieldArray fields);
    const char* call_function(const char* name, const char* bytes);

#ifdef __cplusplus
}
#endif

#endif