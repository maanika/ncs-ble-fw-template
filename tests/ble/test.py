
from blatann import BleDevice
from blatann.nrf import nrf_events
from blatann.utils import setup_logger
import time
import struct
from tests.ble import constants

logger = setup_logger(level="INFO")

def on_button_char_notification(characteristic, event_args):
    """
    Callback for when a notification is received from the peripheral's button characteristic.

    :param characteristic: The characteristic the notification was on
    :type characteristic: blatann.gatt.gattc.GattcCharacteristic
    :param event_args: The event arguments
    :type event_args: blatann.event_args.NotificationReceivedEventArgs
    """
    value = struct.unpack("?", event_args.value)[0]
    logger.info("Button pressed: {}".format(value))


def find_target_device(ble_device: BleDevice, name: str):
    """
    Starts the scanner and searches the advertising report for the desired name.
    If found, returns the peer's address that can be connected to

    :param ble_device: The ble device to operate on
    :param name: The device's local name that is advertised
    :return: The peer's address if found, or None if not found
    """
    # Start scanning for the peripheral.
    # Using the `scan_reports` iterable on the waitable will return the scan reports as they're
    # discovered in real-time instead of waiting for the full scan to complete
    for report in ble_device.scanner.start_scan().scan_reports:
        if report.advertise_data.local_name == name:
            return report.peer_address


def main(serial_port):
    # Set the target to the peripheral's advertised name
    target_device_name = constants.PERIPHERAL_NAME

    # Create and open the BLE device
    ble_device = BleDevice(serial_port)
    ble_device.event_logger.suppress(nrf_events.GapEvtAdvReport)
    ble_device.open()

    # Set the scanner to scan for 4 seconds
    ble_device.scanner.set_default_scan_params(timeout_seconds=4)

    logger.info("Scanning for '{}'".format(target_device_name))
    target_address = find_target_device(ble_device, target_device_name)

    if not target_address:
        logger.info("Did not find target peripheral")
        return

    # Initial the connection and wait for it to finish
    logger.info("Found match: connecting to address {}".format(target_address))
    peer = ble_device.connect(target_address).wait()
    if not peer:
        logger.warning("Timed out connecting to device")
        return
    logger.info("Connected, conn_handle: {}".format(peer.conn_handle))

    _, event_args = peer.discover_services().wait(10, exception_on_timeout=False)
    logger.info("Service discover complete! status: {}".format(event_args.status))

    # Log each service found
    for service in peer.database.services:
        logger.info(service)

    # Find the LED characteristic
    led_char = peer.database.find_characteristic(constants.LED_CHAR_UUID)
    if led_char:
        for i in range(10):
            data_to_send = [1]
            if (i%2):
                data_to_send = [0]
            if not led_char.write(data_to_send).wait(5, False):
                logger.error("Failed to write data {}".format(data_to_send))
                break
            time.sleep(1)
    else:
        logger.info("Failed to find LED characteristic")

    # Find the Button characteristic
    button_char = peer.database.find_characteristic(constants.BUTTON_CHAR_UUID)
    if button_char:
        logger.info("Subscribing to the Button characteristic")
        button_char.subscribe(on_button_char_notification).wait(5)
    else:
        logger.warning("Failed tp find Button characteristic")

    time.sleep(10) # Test button presses on hardware during this time

    # Clean up
    logger.info("Disconnecting from peripheral")
    peer.disconnect().wait()
    ble_device.close()

if __name__ == '__main__':
    main("COM16")
