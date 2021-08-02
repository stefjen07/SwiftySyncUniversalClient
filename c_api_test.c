#include "c_api_connector.h"
#include "c_api_storage.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main() {
    printf("Client starting...\n");
    create_client("ws://localhost:8888");
    run_client();
    printf("Client started\n");
    while(!authorized()) {
        authorize(2, "");
#ifdef _WIN32
        Sleep(5000);
#else
        usleep(5000);
#endif
    }
    printf("Authorized\n");
    printf("Function returned %s\n", call_function("nothing", "nothing"));
    struct CFieldArray fields;
    fields.size = 1;
    fields.ptr = malloc(sizeof(struct CField) * fields.size);
    for(int i=0;i<fields.size;i++) {
        fields.ptr[i] = *CField_new(cft_string, "hi");
        fields.ptr[i].str_value = "hello";
    }
    set_document("users", "stefjen07", fields);
    struct CFieldArray doc = get_document("users", "stefjen07");
    printf("%s\n", get_array_child(doc, "hi")->str_value);
    return 0;
}