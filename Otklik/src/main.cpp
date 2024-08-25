#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include <NimBLEDevice.h>
#include <Tone32.h>
#include <LittleFS.h>

#define DIVECE_NAME "ZoneArtefact"
#define SETTINGS_UUID "0x0666"

#define LED1_PIN  13
#define LED2_PIN  14
#define LED3_PIN  15

#define BUZZER_PIN 12
#define BUZZER_CHANNEL 0
#define BUZZER_BASE_FREQ 1024

#define LAST_RSSI_SIZE 5

unsigned int rssi = 0;
unsigned int last_rssi[LAST_RSSI_SIZE] = {0};
unsigned int last_rssi_index = 0;
unsigned int low_signal_filter = 0;

unsigned long previousMillis = 0; 
unsigned long interval = 0;

NimBLEScan* pBLEScan;

void saveFile(String filename, String data);
void readFile(String filename, String &data);

class MyAdvertisedDeviceCallbacks: public NimBLEAdvertisedDeviceCallbacks {
    void onResult(NimBLEAdvertisedDevice* advertisedDevice) {
	
		if (strcmp(advertisedDevice->getServiceDataUUID().toString().c_str(), SETTINGS_UUID) == 0 ) {
			int new_low_signal_filter = atoi(advertisedDevice->getServiceData().c_str());

			if (new_low_signal_filter != low_signal_filter) {
				low_signal_filter = new_low_signal_filter;
				saveFile("/settings.txt", String(low_signal_filter));
				Serial.printf("Saved new low signal filter: %d\n", low_signal_filter);
			}
		}
		if (strcmp(advertisedDevice->getName().c_str(), DIVECE_NAME) == 0 ) {
			rssi = (100 + advertisedDevice->getRSSI()) * 10;

			if (rssi >= 1024) {
				rssi = 1024;
			} else if (rssi <= low_signal_filter) {
				rssi = 0;
			}

			Serial.printf(
				"Advertised Device: %s, Address: %s, RSSI: %d \n", 
				advertisedDevice->getName().c_str(),
				advertisedDevice->getAddress().toString().c_str(),
				rssi/10
			);

			interval = 1024 - rssi - 100;
		}
    }
};

void saveFile(String filename, String data) {
	File file = LittleFS.open(filename, "w");
	if (file) {
      file.println(data); 
      file.close();
    }
}

void readFile(String filename, String &data) {
	File file = LittleFS.open(filename, "r");
	while (file.available()) {
		data = file.readStringUntil('\n');
    }
	file.close();
}

void setup() {
	String data;

	pinMode(LED1_PIN, OUTPUT);
	pinMode(LED2_PIN, OUTPUT);
	pinMode(LED3_PIN, OUTPUT);

  	Serial.begin(115200);
	LittleFS.begin();
  	Serial.println("Scanning...");

	readFile("/settings.txt", data);

	if (data != "") {	
		low_signal_filter = data.toInt();
		Serial.printf("Loaded low signal filter: %s\n", data);
	}

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

	int freq = BUZZER_BASE_FREQ - rssi - 100;

	if (rssi > 0) {
		tone(BUZZER_PIN, NOTE_A1, freq, BUZZER_CHANNEL);
		noTone(BUZZER_PIN, BUZZER_CHANNEL);
		delay(freq);

		unsigned long currentMillis = millis();

		if (currentMillis - previousMillis >= interval) {
			previousMillis = currentMillis;

			if (digitalRead(LED1_PIN) == LOW) {
				digitalWrite(LED1_PIN, HIGH);
				digitalWrite(LED2_PIN, HIGH);
				digitalWrite(LED3_PIN, HIGH);

				digitalWrite(LED1_PIN, LOW);
				digitalWrite(LED2_PIN, LOW);
				digitalWrite(LED3_PIN, LOW);

				delay(100);

				digitalWrite(LED1_PIN, HIGH);
				digitalWrite(LED2_PIN, HIGH);
				digitalWrite(LED3_PIN, HIGH);
			} else {
				digitalWrite(LED1_PIN, LOW);
				digitalWrite(LED2_PIN, LOW);
				digitalWrite(LED3_PIN, LOW);
			}
		}
	}
   
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

			digitalWrite(LED1_PIN, LOW);
			digitalWrite(LED2_PIN, LOW);
			digitalWrite(LED3_PIN, LOW);
		}
		last_rssi_index = 0;
	} else {
		last_rssi[last_rssi_index++] = rssi;
	}
}