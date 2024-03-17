from blatann.uuid import Uuid128

PERIPHERAL_NAME = "Smart_Guitar_Amp"

LBS_SERVICE_UUID = Uuid128("00001523-1212-efde-1523-785feabcd123")
BUTTON_CHAR_UUID = LBS_SERVICE_UUID.new_uuid_from_base(0x1524)
LED_CHAR_UUID = LBS_SERVICE_UUID.new_uuid_from_base(0x1525)
