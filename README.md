# Android ↔ Arch Clipboard Sync (Wayland / Hyprland)

A lightweight clipboard-sync daemon for **Wayland (Hyprland)** that mirrors the *“Apple Ecosystem”* behavior — copy on one device, paste on another.

This project exposes the local clipboard through a **secure WebSocket endpoint (TLS)** and pushes updates in real-time to subscribed clients (e.g., an Android app).

## Supported today

- **Hyprland** (via `wlr-data-control-unstable-v1`)
- **Wayland clipboard (regular + primary selection)**
- **Secure WebSocket transport**
- **UTF-8 handling**
- **Broadcast to multiple subscribers**

> This is not a polling hack — clipboard events are captured directly from wlr-roots and streamed out.


## Why this exists

Android + Linux doesn’t have an ecosystem clipboard. KDE Connect is bloat and irrelevant under Hyprland.

This daemon gives you:
- **zero-friction copy/paste across devices**
- **no desktop environment lock-in**
- **just Wayland + a client**


## Architecture
```
Wayland (wlr-data-control)
        │
        ▼
Clipboard listener (C++)
        │
Normalize / encode text
        │
        ▼
uWebSockets SSL server
        │
Broadcast to all clients
```

The Android side is just a WebSocket consumer — receive text, insert into system clipboard.

## Requirements

- Wayland compositor using **wlr-data-control** (tested on Hyprland)
- C++20 toolchain
- OpenSSL
- uWebSockets
- pkg-config
- `wlr-protocols`

## Build

```bash
make
```
Artifacts:
- Binary: `main`
- Clipboard implementation: `Clipboard.cpp`
- Protocol stubs generated under `dep/`

You'll need your own TLS cert/key (place under `misc/` or adjust the paths).

## Run

```bash
./main
```

The server starts on wss://0.0.0.0:9002 (edit in source if needed).

On a client (example JavaScript):

```js
const ws = new WebSocket("wss://your-ip:9002", { rejectUnauthorized: false });
ws.onmessage = (e) => navigator.clipboard.writeText(e.data);
```

## Security

- TLS-encrypted transport
- All connections subscribe to a private topic
- No clipboard history is stored

This is meant for **personal LAN use.**


## Limitations

- Wayland only
- Hard-wired to Hyprland (wlroots)
- No Windows/macOS support (obviously)
- Text-only (no MIME negotiation yet)
- No bidirectional clipboard (coming later)


## Roadmap

- Client authentication
- Android native client (foreground service)
- Bidirectional clipboard sync
- MIME type propagation
- Desktop tray status
