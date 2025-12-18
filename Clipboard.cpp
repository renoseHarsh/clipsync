#include "Clipboard.hpp"
#include "dep/wlr-data-control-unstable-v1-client-protocol.h"
#include <iostream>
#include <sstream>
#include <string>
#include <uWebSockets/App.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

std::string decode_unicode_escapes(const std::string &input);

Clipboard::Clipboard(uWS::SSLApp *app) {
    printf("Initializing Clipboard...\n");
    display = wl_display_connect(nullptr);
    registry = wl_display_get_registry(display);
    this->app = app;
    wl_registry_add_listener(registry, &Clipboard::registry_listener, this);
    wl_display_roundtrip(display);
    zwlr_data_control_device_v1 *device =
        zwlr_data_control_manager_v1_get_data_device(manager, seat);
    zwlr_data_control_device_v1_add_listener(device,
                                             &Clipboard::device_listener, this);
    printf("Clipboard initialized.\n");
}

void Clipboard::handle_global(void *data, struct wl_registry *registry,
                              uint32_t name, const char *interface,
                              uint32_t version) {
    Clipboard *self = static_cast<Clipboard *>(data);
    if (strcmp(interface, "wl_seat") == 0) {
        self->seat = static_cast<wl_seat *>(
            wl_registry_bind(registry, name, &wl_seat_interface, 1));
    }

    if (strcmp(interface, "zwlr_data_control_manager_v1") == 0) {
        self->manager =
            static_cast<zwlr_data_control_manager_v1 *>(wl_registry_bind(
                registry, name, &zwlr_data_control_manager_v1_interface, 1));
    }
}

void Clipboard::on_data_offer(void *data, zwlr_data_control_device_v1 *device,
                              zwlr_data_control_offer_v1 *id) {
    zwlr_data_control_offer_v1_add_listener(id, &Clipboard::offer_listener,
                                            data);
}

void Clipboard::on_offer_mime_type(
    void *data, struct zwlr_data_control_offer_v1 *zwlr_data_control_offer_v1,
    const char *mime_type) {
    Clipboard *self = static_cast<Clipboard *>(data);
    if (self->latest_mime.empty() && strcmp(mime_type, "text/plain") == 0) {
        self->latest_mime = std::string(mime_type);
    }
}

void Clipboard::on_selection(void *data, zwlr_data_control_device_v1 *device,
                             zwlr_data_control_offer_v1 *id) {
    Clipboard *self = static_cast<Clipboard *>(data);

    int pipefd[2];
    pipe(pipefd);

    zwlr_data_control_offer_v1_receive(id, self->latest_mime.c_str(),
                                       pipefd[1]);
    close(pipefd[1]);

    wl_display_flush(self->display);

    char buffer[4096];
    ssize_t n;

    std::string contents;

    while ((n = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        contents.append(buffer, n);
    }
    close(pipefd[0]);
    contents = decode_unicode_escapes(contents);
    self->app->publish("clipboard", contents, uWS::OpCode::TEXT, false);
    printf("%s\n", contents.c_str());
}

void Clipboard::on_primary_selection(void *data,
                                     zwlr_data_control_device_v1 *device,
                                     zwlr_data_control_offer_v1 *id) {}

const struct wl_registry_listener Clipboard::registry_listener = {
    .global = Clipboard::handle_global,
    .global_remove = nullptr,
};

const struct zwlr_data_control_device_v1_listener Clipboard::device_listener = {
    .data_offer = Clipboard::on_data_offer,
    .selection = Clipboard::on_selection,
    .primary_selection = Clipboard::on_primary_selection,
};

const zwlr_data_control_offer_v1_listener Clipboard::offer_listener = {
    .offer = Clipboard::on_offer_mime_type,
};

void Clipboard::monitor() {
    while (wl_display_dispatch(display) != -1) {
    }
}

std::string decode_unicode_escapes(const std::string &input) {
    std::ostringstream result;

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 5 < input.size() && input[i + 1] == 'u') {
            std::string hex = input.substr(i + 2, 4);
            unsigned int code = 0;

            std::istringstream iss(hex);
            iss >> std::hex >> code;

            // Convert Unicode code point to UTF-8
            if (code <= 0x7F) {
                result << static_cast<char>(code);
            } else if (code <= 0x7FF) {
                result << static_cast<char>(0xC0 | ((code >> 6) & 0x1F));
                result << static_cast<char>(0x80 | (code & 0x3F));
            } else {
                result << static_cast<char>(0xE0 | ((code >> 12) & 0x0F));
                result << static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                result << static_cast<char>(0x80 | (code & 0x3F));
            }

            i += 5; // skip over \uXXXX
        } else {
            result << input[i];
        }
    }

    return result.str();
}
