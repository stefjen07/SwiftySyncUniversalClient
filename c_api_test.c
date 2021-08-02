#include "c_api_connector.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Client starting...\n");
    createClient("ws://localhost:8888");
    runClient();
    printf("Client started\n");
    while(!authorized()) {
        authorize(2, "");
        sleep(5);
    }
    printf("Authorized\n");
    printf("Function returned %s\n", call_function("nothing", "nothing"));
    set_document("users", "stefjen07", "owner");
    return 0;
}