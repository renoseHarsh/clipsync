#include "Clipboard.hpp"
#include <thread>
#include <uWebSockets/App.h>

struct PerSocketData {};

int main() {
    uWS::SSLApp app =
        uWS::SSLApp({
                        .key_file_name = "misc/key.pem",
                        .cert_file_name = "misc/cert.pem",
                    })
            .ws<PerSocketData>(
                "/*", {.compression = uWS::SHARED_COMPRESSOR,
                       .open = [](auto *ws) { ws->subscribe("clipboard"); },
                       .message =
                           [](auto *ws, std::string_view message,
                              uWS::OpCode opCode) {
                               printf("%.*s\n",
                                      static_cast<int>(message.length()),
                                      message.data());
                           }})
            .listen(9001, [](auto *listen_socket) {
                if (listen_socket) {
                    printf("Listening on port %d\n", 9001);
                }
            });

    Clipboard clipboard(&app);

    std::thread monitorThread([&clipboard]() { clipboard.monitor(); });

    app.run();
}
