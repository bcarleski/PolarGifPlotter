#include <BTstackLib.h>

#define BLE_DEVICE_NAME "Dynamic_Sand_Arduino"
#define BLE_SERVICE_UUID "45aa5c8f-c47e-42f6-af4a-66544b8aff17"
#define BLE_STATUS_UUID "52eb19a4-6421-4910-a8ca-7ff75ef2f56b"
#define BLE_COMMAND_UUID "66af95bc-3dd1-4343-b4b5-ad328b33fda7"
#define BLE_MAX_RADIUS_UUID "fb65af7d-494f-4a45-8872-6e6ffbf0703c"
#define BLE_RADIUS_STEP_SIZE_UUID "fd18cf65-85d7-4730-ad77-4cc3fabaab99"
#define BLE_AZIMUTH_STEP_SIZE_UUID "4dc9c338-0004-4c05-bd26-bb13e55c3bb9"
#define BLE_MARBLE_SIZE_UUID "60af168a-b702-4d0b-8c1b-f35c7a436781"
#define BLE_DRAWING_UUID "fa95bee6-46f9-4898-913a-0575019d3d33"
#define BLE_STEP_UUID "54a63a69-90ce-4b14-a103-46152bb1da68"
#define BLE_RADIUS_UUID "7fcd311a-fafa-47ee-80b8-618616697a59"
#define BLE_AZIMUTH_UUID "eb654acc-3430-45e3-8dc9-22c9fe982518"
#define BLE_STATE_UUID "ec314ea1-7426-47fb-825c-8fbd8b02f7fe"
#define BLE_STRING_SIZE 512

static char characteristic_data = 'H';

void deviceConnectedCallback(BLEStatus status, BLEDevice *device) {
  (void) device;
  switch (status) {
    case BLE_STATUS_OK:
      Serial.println("Device connected!");
      break;
    default:
      break;
  }
}

void deviceDisconnectedCallback(BLEDevice * device) {
  hci_con_handle_t handle = device->getHandle();
  Serial.print("Disconnected BLE Device - ");
  Serial.println(handle);
}

/** 
  * In BTstack, the Read Callback is first called to query the size of the Characteristic Value, before it is called to
  * provide the data. Both times, the size has to be returned. The data is only stored in the provided buffer, if the
  * buffer arguement is not NULL. If more than one dynamic Characteristics is used, the value handle is used to distinguish them.
  */
uint16_t gattReadCallback(uint16_t value_handle, uint8_t * buffer, uint16_t buffer_size) {
  (void) value_handle;
  (void) buffer_size;
  if (buffer) {
    Serial.print("gattReadCallback, value: ");
    Serial.println(characteristic_data, HEX);
    buffer[0] = characteristic_data;
  }
  return 1;
}

/**
  * When the remove device writes a Characteristic Value, the Write callback is called. The buffer arguments points to the data of size size/ If more than one dynamic Characteristics is used, the value handle is used to distinguish them.
  */
int gattWriteCallback(uint16_t value_handle, uint8_t *buffer, uint16_t size) {
  (void) value_handle;
  (void) size;
  characteristic_data = buffer[0];
  Serial.print("gattWriteCallback , value ");
  Serial.println(characteristic_data, HEX);
  return 0;
}
