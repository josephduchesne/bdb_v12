#include "bdb_ble.h"
#include "SEGGER_RTT.h"

BLEService        lbs(LBS_UUID_SERVICE);
BLECharacteristic lsbBotStatus(LBS_UUID_BOT_STATUS);
BLECharacteristic lbsBotControl(LBS_UUID_BOT_CONTROL);

DShotPWMOutput *motors = nullptr;

void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value)
{
    // Display the raw request packet
    SEGGER_RTT_printf(0,"CCCD Updated: %d\n", cccd_value);
    //Serial.printBuffer(request->data, request->len);
    if (chr->uuid == lsbBotStatus.uuid) {
        if (chr->notifyEnabled(conn_hdl)) {
            SEGGER_RTT_printf(0,"lsbBotStatus 'Notify' enabled\n");
        } else {
            SEGGER_RTT_printf(0,"lsbBotStatus 'Notify' disabled\n");
        }
    }
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include HRM Service UUID
  Bluefruit.Advertising.addService(lbs);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.setName("BDB");
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setLED(uint8_t on)
{
  // data = 255 -> LED = On
  // data = 0 -> LED = Off
  analogWrite(LED_BUILTIN, on);
  //digitalWrite(LED_BUILTIN, on ? LED_STATE_ON : (1-LED_STATE_ON));
}

void ArcadeToDifferential(int8_t drive, int8_t steer, int8_t &left, int8_t &right) {

    // Channel mixing math from http://home.kendra.com/mauser/joystick.html
    // Get X and Y from the Joystick, do whatever scaling and calibrating you need to do based on your hardware.
    float x = (float)steer;
    float y = (float)drive;

    float v = (100.0 - abs(x)) * (y / 100.0) + y;
    float w = (100.0 - abs(y)) * (x / 100.0) + x;

    right = (int8_t)((v + w) / 2.0);
    left = (int8_t)((v - w) / 2.0);
}

void control_write_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len)
{
  static unsigned long last_led_time = 0;
  //SEGGER_RTT_printf(0, "LED property set to %d, %luus since last\n", data[0], millis()-last_led_time);
  last_led_time = millis();

  (void) conn_hdl;
  (void) chr;
  (void) len; // len should be 1

  // data = 1 -> LED = On
  // data = 0 -> LED = Off
  int8_t throttle_x = (int8_t)data[0];
  int8_t throttle_y = (int8_t)data[1];
  setLED(abs(throttle_y));

  int8_t throttle_left, throttle_right;
  ArcadeToDifferential(throttle_y, throttle_x, throttle_left, throttle_right);

  //SEGGER_RTT_printf(0, "Throttle M1 set to %d %d (%d %d)\n", throttle_x, throttle_y, throttle_left, throttle_right);
  motors->setThrottle(0, throttle_left); 
  motors->setThrottle(1, throttle_left); 
  motors->setThrottle(2, throttle_right); 
  motors->setThrottle(3, throttle_right); 
  motors->display(); // set m1
}


// callback invoked when central connects
uint8_t connection_count = 0;
void connect_callback(uint16_t conn_handle)
{
  (void) conn_handle;
  BLEConnection* conn = Bluefruit.Connection(conn_handle);

  setLED(true);
  lbsBotControl.write16(0);  // set initial state to 0

  connection_count++;
  SEGGER_RTT_printf(0, "Connection count: %d\n", connection_count);

  
  conn->requestPHY();  // 2mbit
  conn->requestDataLengthUpdate();
  conn->requestMtuExchange(247);

  conn->requestConnectionParameter(9); // in unit of 1.25

  // Keep advertising if not reaching max
  if (connection_count < MAX_PRPH_CONNECTION)
  {
    SEGGER_RTT_WriteString(0, "Keep advertising\n");
    Bluefruit.Advertising.start(0);
  }
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  setLED(false);
  lbsBotControl.write16(0);  // disable motors

  motors->setChannel(0, 0, 0);
  motors->setChannel(1, 0, 0); 
  motors->setChannel(2, 0, 0); 
  motors->setChannel(3, 0, 0);
  motors->display(); // disable motors
  

  SEGGER_RTT_printf(0, "Disconnected, reason = 0x%x\n", reason); 

  connection_count--;
}



void ble_setup(DShotPWMOutput *motors_)
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1-LED_STATE_ON); // led off

  motors = motors_;

  Serial.begin(115200);
  //while ( !Serial ) delay(10);   // for nrf52840 with native usb

  SEGGER_RTT_WriteString(0, "BDBv12 Init BLE\n");
  SEGGER_RTT_WriteString(0, "------------------------------\n");

  // Initialize Bluefruit with max concurrent connections as Peripheral = MAX_PRPH_CONNECTION, Central = 0
  SEGGER_RTT_WriteString(0, "Initialise the nRF52 module\n");
  Bluefruit.autoConnLed(false);
  Bluefruit.begin(MAX_PRPH_CONNECTION, 0);
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  

  // Setup the LED-Button service using
  SEGGER_RTT_WriteString(0, "Configuring the Service\n");

  // Note: You must call .begin() on the BLEService before calling .begin() on
  // any characteristic(s) within that service definition.. Calling .begin() on
  // a BLECharacteristic will cause it to be added to the last BLEService that
  // was 'begin()'ed!
  lbs.begin();

  // Configure Button characteristic
  // Properties = Read + Notify
  // Permission = Open to read, cannot write
  // Fixed Len  = 1 (button state)
  lsbBotStatus.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  lsbBotStatus.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  lsbBotStatus.setFixedLen(1);
  lsbBotStatus.setCccdWriteCallback(cccd_callback); 
  lsbBotStatus.begin();
  //lsbBotStatus.notify8(100);

  // Configure the LED characteristic
  // Properties = Read + Write
  // Permission = Open to read, Open to write
  // Fixed Len  = 1 (LED state)
  lbsBotControl.setProperties(CHR_PROPS_READ | CHR_PROPS_WRITE_WO_RESP);
  lbsBotControl.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  lbsBotControl.setFixedLen(2);
  lbsBotControl.begin();
  lbsBotControl.write16(0x0); // Disabled on init

  lbsBotControl.setWriteCallback(control_write_callback);

  // Setup the advertising packet(s)
  SEGGER_RTT_WriteString(0, "Setting up the advertising\n");
  startAdv();
}

void ble_update_status(uint8_t battery) {
  SEGGER_RTT_printf(0, "Notifying battery: %d%%\n", battery); 
  lsbBotStatus.notify8(battery);
}

void ble_loop()
{
    static unsigned long last_debug = 0;
    if (millis()-last_debug>1000) {
        if(connection_count>0) {
            Bluefruit.Connection(0)->requestConnectionParameter(9); // in unit of 1.25
        }
        last_debug=millis();
    }
//   uint8_t newState = (BUTTON_ACTIVE == digitalRead(button));

//   // only notify if button state changes
//   if ( newState != buttonState)
//   {
//     buttonState = newState;
//     lsbBotStatus.write8(buttonState);

//     // notify all connected clients
//     for (uint16_t conn_hdl=0; conn_hdl < MAX_PRPH_CONNECTION; conn_hdl++)
//     {
//       if ( Bluefruit.connected(conn_hdl) && lsbBotStatus.notifyEnabled(conn_hdl) )
//       {
//         lsbBotStatus.notify8(conn_hdl, buttonState);
//       }
//     }
//   }
}
