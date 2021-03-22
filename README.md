# PlayMP3TriggeredByEddystone

Playing a MP3 audio file when M5stack receive the Eddystone beacon with a specific namespace.

## Getting Started
### Prerequisites
- [M5stack Basic](https://m5stack.com/)
- micro SD card
- [Arduino IDE](https://www.arduino.cc/en/software) 
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)
   

### Installing
Clone this git repository into your Arduino source directory.
```bash
cd ~/Arduino
git clone https://github.com/yohama/PlayMP3TriggeredByEddystone.git
```

### Usage
1. Write an appropriate MP3 audio file to the micro SD card.
1. Insert the micro SD card into the M5stack.
1. Open ~/Arduino/PlayMP3TriggedByEddystone/PlayMP3TriggedByEddystone.ino by Arduino IDE.
1. Change line at "#define BEACON_NAMESPACE\ {...}" to an appropriate namespace of the Eddystone beacon.
1. Change line at "#define MP3AUDIO_FILEPATH "..."" to an appropriate path of MP3 audio file in micro SD card.
1. Click "Upload" button on Arduino IDE.

## Author
yohama 

## License
The source code is licensed the GNU General Public License version 3.