#include <M5Stack.h>

#include "BLEDevice.h"
//#include "BLEScan.h"

#define DEVICE_NAME         "M5StickC"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("1ed42829-cd7a-44e6-87db-24fed6422bc4");
// The characteristic of the remote service we are interested in.
static BLEUUID charUUID("503423d1-ad1f-4ed3-b3df-06e276798fba");

static boolean doConnect = false;
static boolean connected = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;

int buttons = 0;

void printStatus(char const *text)
{
  Serial.println(text);

  M5.Lcd.fillRect(0, 0, 320, 40, M5.Lcd.color565(64, 64, 80));
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(4);
  M5.Lcd.setCursor(4, 4);
  M5.Lcd.printf(text);
}

void printScanningStatus()
{
  printStatus("Scanning...");  
}

void drawButtons()
{
  for (int i = 0; i < 3; i++) {
    int w = 320 / 3;
    int x = w * i;
    int h = 48;
    int y = 240 - h - 2;
    int color = ((buttons >> i) & 1) ? M5.Lcd.color565(0, 255, 0) : M5.Lcd.color565(64, 64, 64);
    M5.Lcd.drawRect(x, y, w, h, WHITE);
    M5.Lcd.fillRect(x + 2, y + 2, w - 4, h - 4, color);
  }
}

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify)
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    for (int i = 0; i < length; i++) {
      Serial.printf(" %02x", pData[i]);
    }
    if (length == 1) {
      buttons = pData[0];
      drawButtons();
    }
    Serial.println();
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient)
  {
  }

  void onDisconnect(BLEClient *pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient *pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    if (!pRemoteService) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (!pRemoteCharacteristic) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic.
    if(pRemoteCharacteristic->canRead()) {
      std::string value = pRemoteCharacteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    printStatus("OK, Connected");
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.getName() == DEVICE_NAME) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      return;
    }
  }
};

void startScan()
{
  printScanningStatus();
  BLEDevice::getScan()->start(0);
}

void setup()
{
  setCpuFrequencyMhz(80);
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  drawButtons();
  printStatus("Starting...");

  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  startScan();
//  pBLEScan->start(0);
}


// This is the Arduino main loop function.
void loop()
{

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
#if 0
    String newValue = "Time since boot: " + String(millis()/1000);
    Serial.println("Setting new characteristic value to \"" + newValue + "\"");
    
    // Set the characteristic's value to be the array of bytes that is actually a string.
    pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());
#endif
  } else {//if (doScan) {
    startScan();
  }
  
  delay(100); // Delay a second between loops.
}
