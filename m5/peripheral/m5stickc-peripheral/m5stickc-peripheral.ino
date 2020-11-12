
#include <M5StickC.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME         "M5StickC"
#define SERVICE_UUID        "1ed42829-cd7a-44e6-87db-24fed6422bc4"
#define CHARACTERISTIC_UUID "503423d1-ad1f-4ed3-b3df-06e276798fba"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int start_advertise_timer = 0;
int buttons = 0;
int lcd_dimmer_counter = 0;
const int min_lcd_brightness = 10;
const int max_lcd_brightness = 50;

void activateLcdBrightness()
{
  lcd_dimmer_counter = 10 * 10; // 10sec
//  M5.Lcd.setBrightness(max_lcd_brightness);
}

void printStatus(char const *text)
{
  M5.Lcd.fillRect(0, 0, 320, 10, M5.Lcd.color565(64, 64, 80));
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(1, 1);
  M5.Lcd.printf(text);

  activateLcdBrightness();
}

void printWatingStatus()
{
  printStatus("Waiting...");  
}

void printConnectedStatus()
{
  printStatus("OK, Connected");  
}

void drawButtons()
{
  int i = 0;
  int w = 80;
  int x = 0;
  int h = 80;
  int y = 80;
  int color = ((buttons >> i) & 1) ? M5.Lcd.color565(0, 255, 0) : M5.Lcd.color565(64, 64, 64);
  M5.Lcd.drawRect(x, y, w, h, WHITE);
  M5.Lcd.fillRect(x + 2, y + 2, w - 4, h - 4, color);
}

void startAdvertise()
{
  printWatingStatus();
  start_advertise_timer = 5;  
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };
  
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      for (int i = 0; i < value.length(); i++) {
        Serial.printf("0x%02x", value[i]);
      }
      Serial.println();
    }
  }
};

void setup()
{
  setCpuFrequencyMhz(80);
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  printStatus("Starting...");
  drawButtons();

//  Serial.begin(115200);
//  Serial.println("Starting BLE Remote Controller...");
  
  // Create the BLE Device
  BLEDevice::init(DEVICE_NAME);
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ   |
        BLECharacteristic::PROPERTY_WRITE  |
        BLECharacteristic::PROPERTY_NOTIFY |
        BLECharacteristic::PROPERTY_INDICATE
        );
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();
  
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  startAdvertise();

//  Serial.println("Ready.");
}

void loop()
{
  if (deviceConnected) {
    if (!oldDeviceConnected) {
      oldDeviceConnected = true;
      printConnectedStatus();
    }
    M5.update();
    uint8_t value = 0;
    if (M5.BtnA.isPressed()) value |= 1;
    if (M5.BtnB.isPressed()) value |= 2;
    if (buttons != value) {
      buttons = value;
      pCharacteristic->setValue((uint8_t *)&value, 1);
      pCharacteristic->notify();
      drawButtons();
      activateLcdBrightness();
    }
  } else {
    if (oldDeviceConnected) {
      oldDeviceConnected = false;
      startAdvertise();
    } else if (start_advertise_timer > 0) {
      start_advertise_timer--;
      if (start_advertise_timer == 0) {
        pServer->startAdvertising(); // restart advertising
      }
    }
  }

  delay(100);
}
