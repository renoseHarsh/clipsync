#pragma once
#include "wlr-data-control-unstable-v1-client-protocol.h"
#include <string>
#include <uWebSockets/App.h>
#include <wayland-client-protocol.h>

class Clipboard {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_seat *seat;
    struct zwlr_data_control_manager_v1 *manager;

    static const struct wl_registry_listener registry_listener;
    static const struct zwlr_data_control_device_v1_listener device_listener;
    static const zwlr_data_control_offer_v1_listener offer_listener;
    uWS::SSLApp *app;

  public:
    Clipboard(uWS::SSLApp *app);
    void monitor();
    std::string latest_mime = "";

    static void handle_global(void *data, struct wl_registry *registry,
                              uint32_t name, const char *interface,
                              uint32_t version);

    static void on_data_offer(void *data, zwlr_data_control_device_v1 *device,
                              zwlr_data_control_offer_v1 *id);

    static void on_selection(void *data, zwlr_data_control_device_v1 *device,
                             zwlr_data_control_offer_v1 *id);

    static void on_primary_selection(void *data,
                                     zwlr_data_control_device_v1 *device,
                                     zwlr_data_control_offer_v1 *id);

    static void on_offer_mime_type(
        void *data,
        struct zwlr_data_control_offer_v1 *zwlr_data_control_offer_v1,
        const char *mime_type);
};
