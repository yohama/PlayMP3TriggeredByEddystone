/**
 * @file    PlayMP3TriggedByEddystone.ino
 * @brief   Playing a MP3 audio file when M5stack receive the Eddystone 
 *          beacon with a specific namespace.
 * @author  yohama
 * @date    Mar.20, 2021
 * 
 * Copyright (C) 2021  yohama
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string>

#include <M5Stack.h>
#include <BLEDevice.h>

#include "AudioFileSourceSD.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

/*
 * @def   EDDYSTONE_BEACON
 * @brief The Eddystone Service UUID
 */
#define EDDYSTONE_BEACON BLEUUID((uint16_t)0xFEAA)
/*
 * @def   BEACON_NAMESPACE
 * @brief Target namespace of the beacon
 */
#define BEACON_NAMESPACE \
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
/*
 * @def   MP3AUDIO_FILEPATH
 * @brief Path of MP3 autdio file
 */
#define MP3AUDIO_FILEPATH "/change_to_appropriate_filename.mp3"
/*
 * @def   BLESCAN_DURATION
 * @brief Scanning duration time per an iteration [sec]
 */
#define BLESCAN_DURATION 120

//! Stored target namespace of the beacon.
static uint8_t NS[] = BEACON_NAMESPACE; 
//! Allocated a pointer for BLE Scanning.
BLEScan* pBLEScan;
//! Allocated a boolean flag for detecting the target namespace.
bool detected = false;

/*
 * @class playMP3
 * @brief Playing MP3 audio file using the ESP8266Audio library.
 */
class playMP3 {
  private:
    /*
     * @var     _mp3
     * @brief   Wrap libmad MP3 library to play audio
     */
    AudioGeneratorMP3* _mp3;
    /*
     * @var     _id3
     * @brief   ID3 filter that extracts any ID3 fields and sends to CB function
     */
    AudioFileSourceID3* _id3;
    /*
     * @var     _out
     * @brief   Base class for an I2S output port
     */
    AudioOutputI2S* _out;
  public:
    /*
     * @brief           Constructor of playMP3 class
     *                  All the required variable are initialized.
     * @fn              playMP3::playMP3
     * @param[in] path  Path of playing MP3 file on SD card 
     */
    playMP3(char* path): 
      _mp3(new AudioGeneratorMP3()),
      _id3(new AudioFileSourceID3(new AudioFileSourceSD(path))),
      _out(new AudioOutputI2S(0, 1)) {}
    /*
     * @brief           Set parameters to instance of the AudioOutputI2S.
     * @fn              playMP3::setParameters
     * @param[in] gain  Volume related augument (0.0-4.0, 0.5 is default)
     */
    void setParameters(float gain=0.5) {
      _out->SetGain(gain);
      _out->SetOutputModeMono(true);
    }
    /*
     * @brief           Playing MP3 audio.
     * @fn              playMP3::play
     */    
    void play(void) {
      _mp3->begin(_id3, _out);
      while (_mp3->isRunning()) {
        if (!_mp3->loop()) {
          _mp3->stop();
        }
      } 
    }
};

/*
 * @class myBLEAdvertisedDeviceCB
 * @brief Call back methods when M5Stack received BLE beacons.
 *        If the received beacon is the Eddystone beacon with a specific 
 *        namespace, a global boolean variable changes to true.
 */
class myBLEAdvertisedDeviceCB: public BLEAdvertisedDeviceCallbacks {
  private:
    /*
     * @var     _ns
     * @brief   Target namespace
     */
    uint8_t* _ns;
    /*
     * @var     _size_ns
     * @brief   Size of the target namespace
     */
    uint8_t _size_ns;
  public:
    /*
     * @brief             Constructor of myBLEAdvertisedDeviceCByMP3 class
     *                    All the required variable are initialized.
     * @fn                myBLEAdvertisedDeviceCB::myBLEAdvertisedDeviceCB
     * @param[in] ns      Target namespace 
     * @param[in] size_ns Size of the target namespace
     */
    myBLEAdvertisedDeviceCB(uint8_t* ns, uint8_t size_ns=10):
      _ns(ns), _size_ns(size_ns) {}
    /*
     * @brief             Callback on receiving BLE beacons
     * @fn                myBLEAdvertisedDeviceCB::onResult
     * @param[in] advertisedDevice
     *                    Sender information of the revecied beacon
     */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      uint8_t cServiceData[100];
      std::string strServiceData = advertisedDevice.getServiceData();
      strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);
      if (advertisedDevice.getServiceDataUUID().equals(EDDYSTONE_BEACON)) {
        if (memcmp(cServiceData+2, _ns, _size_ns) == 0) {
          advertisedDevice.getScan()->stop();
          detected = true;
          M5.Lcd.printf("NS:");
          if (_size_ns + 2 < strServiceData.length()){
            for (int i=0; i<_size_ns; i++) {
              M5.Lcd.printf("%02X",cServiceData[i+2]);
            }
          }
          M5.Lcd.printf(" Device:%s\n", advertisedDevice.toString().c_str());
        }
      }
    }
};

/*
 * @brief   M5stack setup routine
 * @fn      setup
 */
void setup()
{
  M5.begin();
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new myBLEAdvertisedDeviceCB(NS));
  pBLEScan->setActiveScan(true);
}

/*
 * @brief   M5stack loop routine
 * @fn      loop
 */
void loop()
{
  playMP3 audio(MP3AUDIO_FILEPATH);
  audio.setParameters();
  detected = false;
  M5.Lcd.clear(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Scanning Eddystone Beacon...\n");
  pBLEScan->start(BLESCAN_DURATION, false);
  if (detected) {
    audio.play();
  }
}
