#ifndef C_API_CONNECTOR_H
#define C_API_CONNECTOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    void createClient(const char *uri);
    void runClient();
    void authorize(unsigned provider, const char *credentials);
    bool authorized();
    const char* get_field(const char* collectionName, const char* documentName, const char* path);
    bool set_field(const char* collectionName, const char* documentName, const char* path, const char* value);
    const char* get_document(const char* collectionName, const char* documentName);
    bool set_document(const char* collectionName, const char* documentName, const char* content);
    const char* call_function(const char* name, const char* bytes);

#ifdef __cplusplus
}
#endif

#endif