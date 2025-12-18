import asyncio
import ssl

import websockets


async def run():
    uri = "wss://localhost:9001"

    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_NONE

    async with websockets.connect(uri, ssl=ssl_context) as websocket:
        await websocket.send("Hello from Python!")
        while True:
            msg = await websocket.recv()
            print(f"Received: {msg}")


if __name__ == "__main__":
    asyncio.run(run())
