#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "NimBLEDevice.h"
#include <Tone32.h>

#define BLE_DIVECE_NAME "ZoneAnomaly"

#define ARROW_TRIGGER_PIN 14
#define ARROW_CHANNEL 2

#define BUZZER_PIN 12
#define BUZZER_CHANNEL 0
#define BUZZER_BASE_FREQ 1024

#define LAST_RSSI_SIZE 5

unsigned long rssi = 0;
unsigned long last_rssi[LAST_RSSI_SIZE] = {0};
unsigned long last_rssi_index = 0;

NimBLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
		if (strcmp(advertisedDevice->getName().c_str(), BLE_DIVECE_NAME) == 0 ) {
			rssi = (100 + advertisedDevice->getRSSI()) * 10;

			if (rssi >= 1024) {
				rssi = 1024;
			}

			ledcWrite(ARROW_CHANNEL, rssi);

			Serial.printf(
				"Advertised Device: %s, Address: %s, RSSI: %d \n", 
				advertisedDevice->getName().c_str(),
				advertisedDevice->getAddress().toString().c_str(),
				rssi/10
			);
		}
    }
};

void setup() {
	ledcAttachPin(ARROW_TRIGGER_PIN, ARROW_CHANNEL);
    ledcSetup(ARROW_CHANNEL, 2000, 8);

  	Serial.begin(115200);
  	Serial.println("Scanning...");

   	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
 	NimBLEDevice::setScanFilterMode(CONFIG_BTDM_SCAN_DUPL_TYPE_DEVICE);

	NimBLEDevice::setScanDuplicateCacheSize(200);

	NimBLEDevice::init("");

	pBLEScan = NimBLEDevice::getScan(); 
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
	pBLEScan->setActiveScan(true);
	pBLEScan->setInterval(100);
	pBLEScan->setWindow(37);
	pBLEScan->setMaxResults(0);
}

void loop() {
	if(pBLEScan->isScanning() == false) {
		pBLEScan->start(0, nullptr, false);
	}

	int freq = BUZZER_BASE_FREQ - rssi;

	tone(BUZZER_PIN, NOTE_A1, freq, BUZZER_CHANNEL);
  	noTone(BUZZER_PIN, BUZZER_CHANNEL);
	delay(freq);
   
	if (last_rssi_index > LAST_RSSI_SIZE - 1) {
		bool reset_rssi = true;

		for (int i = 0; i < LAST_RSSI_SIZE; i++) {
			if (last_rssi[i] != rssi) {
				reset_rssi = false;
				break;
			}
		}

		if (reset_rssi) {
			rssi = 0;
			ledcWrite(ARROW_CHANNEL, rssi);
		}
		last_rssi_index = 0;
	} else {
		last_rssi[last_rssi_index++] = rssi;
	}
}