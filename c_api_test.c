#include "c_api_connector.h"
#include <stdio.h>

int main() {
    printf("Client started\n");
    createClient("ws://localhost:8888");
    call_function("empty", "nothing");
    set_document("users", "stefjen07", "owner");
    return 0;
}