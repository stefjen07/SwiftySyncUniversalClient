#include "c_api_connector.h"
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

int main() {
    printf("Client starting...\n");
    createClient("ws://localhost:8888");
    runClient();
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
    set_document("users", "stefjen07", "owner");
    return 0;
}