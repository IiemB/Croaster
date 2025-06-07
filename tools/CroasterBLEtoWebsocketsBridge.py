
import asyncio
import json
import logging
import time

from websockets.asyncio.server import serve
from websockets.exceptions import ConnectionClosedOK
from bleak import BleakScanner, BleakClient

DATA_UUID = "d56d0059-ad65-43f3-b971-431d48f89a69"

logger = logging.getLogger("BLEtoWebsocketsBridge")
logging.basicConfig(level=logging.INFO, format="%(asctime)s\t%(levelname)s\t%(message)s")
ble_queue = asyncio.Queue()
first_time = True

# see https://github.com/hbldh/bleak/issues/801
async def ble_callback(sender, data: bytearray):
    global ble_queue
    while not ble_queue.empty():
        await ble_queue.get()
    ble_data = json.loads(data)
    rec_time = time.time()
    await ble_queue.put((rec_time, ble_data))


# Find the Croaster by name, return None if not found
async def find_croaster():
    devices = await BleakScanner.discover()
    for d in devices:
        logging.debug(str(d))
        if d.name is not None and "[C4C4]" in d.name:
            logging.info("Found device " + d.name )
            return d
    return None

async def request_handler(websocket):
    global ble_queue, first_time
    try:
        while True:
            message = await websocket.recv()
            decoded_message = json.loads(message)
            if decoded_message["command"] == "getArtisanData":
                # something like: {"command":"getArtisanData","id":6120,"roasterID":0}
                response_id = decoded_message["id"]
                # data = { "BT": 137, "ET": 117}
                # response = {"id": response_id, "data": data }
                # if first_time:
                #     first_time = False
                #     # discard first sample for better syncroniization.
                #     rec_time, ble_data = await ble_queue.get()

                start_time = time.time()
                rec_time, ble_data = await ble_queue.get()

                stop_time = time.time()
                artisan_response = ble_data
                artisan_response["id"] = response_id
                response_json = json.dumps(artisan_response)
                et = artisan_response["data"]["ET"]
                bt = artisan_response["data"]["BT"]
                elapsed_time_ms = (stop_time - start_time) * 1000
                data_age_ms = (stop_time - rec_time) * 1000
                logging.info(f"Response for {response_id}: ET: {et:.2f}C, BT {bt:.2f}C (delay: {elapsed_time_ms:.0f}ms, age: {data_age_ms:.0f}ms)")
                await websocket.send(response_json)
            else:
                logging.warning(f"Unknown command {decoded_message['command']}")
    except ConnectionClosedOK:
        logging.info("Artisan disconnected")



async def main():
    logging.info("Bridge started")
    global croaster_client
    croaster_adress = await find_croaster()
    if croaster_adress is None:
        logging.error("Croaster not found")
        exit()
    croaster_client = BleakClient(croaster_adress)
    await croaster_client.connect()
    if not croaster_client.is_connected:
        exit()
    await croaster_client.start_notify(DATA_UUID, ble_callback)

    async with serve(request_handler, "localhost", 8765) as server:
        await server.serve_forever()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Shutting down")