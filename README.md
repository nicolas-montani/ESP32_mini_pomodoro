# 🍅 ESP32 Poodoro Timer

<div align="center">

**An intelligent Pomodoro timer with personality, presence detection, and playful features!**

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange.svg)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

*Work smarter, not harder - with a timer that keeps you honest!*

</div>

---

## 📖 Table of Contents

- [Features](#-features)
- [Hardware Requirements](#-hardware-requirements)
- [Pin Configuration](#-pin-configuration)
- [Installation](#-installation)
- [Usage](#-usage)
- [Features in Detail](#-features-in-detail)
- [Configuration](#-configuration)
- [Project Structure](#-project-structure)
- [Contributing](#-contributing)
- [License](#-license)

---

## ✨ Features

### 🎯 Core Features
- **⏱️ Classic Pomodoro Timer** - Configurable work sessions (10-90 min) and breaks (5-20 min)
- **👀 RoboEyes Display** - Animated eyes that express emotions based on timer state
- **🔊 Audio Feedback** - Musical tones and Mario theme for motivation
- **💡 Visual Feedback** - Synchronized LED lights (green for success, red for alerts)
- **⚙️ Customizable Durations** - Adjust work and break times on-the-fly

### 🤖 Smart Features
- **📡 Ultrasonic Presence Detection** - Automatically pauses when you leave your desk
- **🤝 Auto-Resume** - Resumes timer when you return (with celebration!)
- **📊 Session Tracking** - Counts completed Pomodoros
- **🎨 Smooth Scrolling Display** - Professional OLED interface

### 🎮 Fun Features
- **🍽️ Mensa Menu Integration** - Check cafeteria menu by shaking the device
- **🎰 Gambling Mode** - Double-shake to play a quick game between work sessions
- **🎵 Mario Theme** - Victory music when you win the gamble
- **😊 Emotional Feedback** - RoboEyes show happiness, sadness, surprise, and more

---

## 🛠️ Hardware Requirements

### Required Components

| Component | Specification | Quantity |
|-----------|--------------|----------|
| **Microcontroller** | ESP32 Dev Module | 1 |
| **Display** | SSD1306 OLED (128x64, I2C) | 1 |
| **Distance Sensor** | HC-SR04 Ultrasonic Sensor | 1 |
| **Vibration Sensor** | KY-002 Shock Sensor | 1 |
| **Buzzer** | Passive Piezo Buzzer | 1 |
| **LEDs** | 5mm LEDs (1 Red, 1 Green) | 2 |
| **Buttons** | Tactile Push Buttons | 2 |
| **Resistors** | 220-330Ω (for LEDs) | 2 |
| **Resistors** | 10kΩ (for buttons) | 2 |

### Optional
- Breadboard or custom PCB
- Enclosure/case
- USB cable for power

---

## 📍 Pin Configuration

### Default Pin Mapping

```
ESP32 Pin    | Component              | Function
-------------|------------------------|---------------------------
GPIO 21      | SSD1306 OLED (SDA)    | I2C Data
GPIO 22      | SSD1306 OLED (SCL)    | I2C Clock
GPIO 5       | Button 1              | Start/Pause/Settings
GPIO 4       | Button 2              | Mode Toggle/Next/Reset
GPIO 13      | Passive Buzzer        | Audio Output
GPIO 33      | Green LED             | Success Indicator
GPIO 32      | Red LED               | Alert Indicator
GPIO 19      | HC-SR04 (TRIG)        | Ultrasonic Trigger
GPIO 18      | HC-SR04 (ECHO)        | Ultrasonic Echo
GPIO 14      | KY-002 Sensor         | Vibration Detection
```

> 💡 **Tip:** All pins can be reconfigured in the respective header files (see [Configuration](#-configuration))

---

## 🚀 Installation

### Prerequisites

1. **PlatformIO IDE** (VSCode extension recommended)
   ```bash
   # Install VSCode extension
   # Search for "PlatformIO IDE" in VSCode extensions
   ```

2. **Git** (to clone the repository)
   ```bash
   # macOS
   brew install git

   # Ubuntu/Debian
   sudo apt-get install git
   ```

### Steps

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/ESP32_poodoro.git
   cd ESP32_poodoro
   ```

2. **Open in PlatformIO**
   ```bash
   # Open the project folder in VSCode with PlatformIO
   code .
   ```

3. **Configure WiFi** (optional, for Mensa menu feature)

   Edit `include/config.h`:
   ```cpp
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourPassword"
   ```

4. **Build the project**
   ```bash
   pio run
   ```

5. **Upload to ESP32**
   ```bash
   pio run --target upload
   ```

6. **Monitor serial output** (optional)
   ```bash
   pio device monitor
   ```

---

## 🎮 Usage

### Basic Operations

#### Starting a Session

1. **Power on** the device
2. The **idle screen** shows:
   - Current mode (WORK/BREAK)
   - Completed Pomodoros count
   - Scrolling motivational banner
3. **Press Button 1** to start the timer

#### During a Session

- **Button 1:** Pause/Resume timer
- **Button 2:** Reset current session
- **Leave your desk:** Timer automatically pauses (ultrasonic sensor)
- **Return to desk:** Timer automatically resumes with celebration!

#### Settings Mode

1. **Press both buttons** simultaneously while idle
2. **Button 1:** Decrease time (-5 min)
3. **Button 2:** Increase time (+5 min)
4. **Both buttons:** Confirm and save

#### Mode Switching (Idle Only)

- **Button 2:** Toggle between WORK and BREAK modes

### Advanced Features

#### 🍽️ Mensa Menu Mode

1. **Shake the device** once
2. Browse menu with Button 1 (previous) / Button 2 (next)
3. **Press both buttons** to exit

#### 🎰 Gambling Mode

1. Enter **Mensa Menu Mode** first
2. **Shake again** to activate gambling
3. **Button 1:** Bet on RED ❤️
4. **Button 2:** Bet on BLACK ♠️
5. Win = Mario theme! 🎵 / Lose = Better luck next time! 😢

---

## 🔍 Features in Detail

### 🤖 Ultrasonic Presence Detection

The HC-SR04 sensor continuously monitors your presence:

- **Initial Calibration:** Measures distance when you start working
- **Monitoring:** Checks every 1 second during work sessions
- **Threshold:** 25cm deviation triggers "user left" event
- **Auto-Pause:** Timer pauses when you leave
- **Auto-Resume:** Timer resumes with happy animation when you return

**RoboEyes Expressions:**
- 😊 Happy when you're present
- 😢 Sad when you leave
- 🎉 Excited when you return

### 💡 LED Feedback System

LEDs provide synchronized visual feedback:

| Event | LED Behavior |
|-------|--------------|
| Happy sounds | 🟢 Green blinks with melody |
| Sad sounds | 🔴 Red blinks with melody |
| Startup | 🟢🔴 Alternating green/red |
| WiFi connected | 🟢 3-second alternating pattern |
| Timer running | Off (no distraction) |

### 🎵 Audio Feedback

Rich audio feedback with multiple jingles:

- **Happy Tones:** Success, completion, return
- **Sad Tones:** Leave, error, failure
- **Startup Jingle:** Power-on sequence
- **Mario Theme:** Gambling victory! 🎮

### 📊 Display System

128x64 OLED display with three main screens:

1. **Idle Screen**
   - Mode indicator (WORK/BREAK)
   - Pomodoro count
   - Scrolling motivational message

2. **Running Screen**
   - Large countdown timer (MM:SS)
   - Current state (WORK/BREAK/PAUSED)
   - Session progress

3. **Settings Screen**
   - Adjustable duration
   - Live preview

---

## ⚙️ Configuration

### Customizing Pin Assignments

Each module has its own configuration in header files:

#### **Buttons & WiFi** - `include/config.h`
```cpp
#define BUTTON1_PIN 5
#define BUTTON2_PIN 4
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
```

#### **Display** - `include/monitor.h`
```cpp
#define SDA_PIN 21
#define SCL_PIN 22
```

#### **Buzzer** - `include/buzzer.h`
```cpp
#define BUZZER_PIN 13
```

#### **LEDs** - `include/lights.h`
```cpp
#define GREEN_LED_PIN 33
#define RED_LED_PIN 32
```

#### **Ultrasonic Sensor** - `include/ultrasound.h`
```cpp
#define TRIG_PIN 19
#define ECHO_PIN 18
```

#### **Vibration Sensor** - `include/shaking.h`
```cpp
#define SHAKING_PIN 14
```

### Customizing Durations

Modify default durations in `include/pomodoro.h`:

```cpp
#define WORK_DURATION 1500           // 25 minutes (in seconds)
#define SHORT_BREAK_DURATION 300     // 5 minutes
#define LONG_BREAK_DURATION 900      // 15 minutes
#define POMODOROS_UNTIL_LONG_BREAK 4 // Long break after 4 sessions
```

Or adjust on-the-fly using the settings mode!

---

## 📁 Project Structure

```
ESP32_poodoro/
├── include/
│   ├── config.h              # Central configuration (WiFi, buttons)
│   ├── pomodoro.h            # Timer logic declarations
│   ├── monitor.h             # Display management
│   ├── ultrasound.h          # Presence detection
│   ├── buzzer.h              # Audio feedback
│   ├── lights.h              # LED control
│   ├── shaking.h             # Vibration sensor
│   ├── gambling.h            # Gambling mode
│   └── request.h             # WiFi & API requests
├── src/
│   ├── main.cpp              # Main application logic
│   ├── pomodoro.cpp          # Timer implementation
│   ├── monitor.cpp           # Display rendering
│   ├── ultrasound.cpp        # Distance measurement
│   ├── buzzer.cpp            # Sound generation
│   ├── lights.cpp            # LED patterns
│   ├── shaking.cpp           # Vibration detection
│   ├── gambling.cpp          # Game logic
│   └── request.cpp           # Network requests
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

### Architecture Highlights

- **Modular Design:** Each feature in its own module
- **Consistent Naming:** `module_function_name()` convention
- **Default Parameters:** Easy initialization with sensible defaults
- **Interrupt-Driven:** Shaking sensor uses hardware interrupts
- **Non-Blocking:** Smooth operation without delays

---

## 🎨 Customization Ideas

### Easy Modifications

1. **Change LED Colors:**
   - Swap red/green LEDs for different colors
   - Modify light patterns in `lights.cpp`

2. **Add More Sounds:**
   - Define new note sequences in `buzzer.cpp`
   - Create custom melodies

3. **Custom Messages:**
   - Edit banner messages in `monitor.cpp`
   - Add motivational quotes

4. **Adjust Sensitivity:**
   - Modify ultrasound range threshold (25cm default)
   - Change shake detection cooldown (2s default)

### Advanced Modifications

1. **Add MQTT Integration:**
   - Report completed Pomodoros to home automation
   - Remote control via MQTT commands

2. **Add Buttons for Quick Start:**
   - Add preset duration buttons (15min, 25min, 45min)

3. **Battery Power:**
   - Add LiPo battery support
   - Implement sleep mode for power saving

4. **Web Interface:**
   - ESP32 web server for configuration
   - View statistics in browser

---

## 🐛 Troubleshooting

### LEDs Don't Light Up

- ✅ Check GPIO 32 and 33 are used (not 34/35 - those are input-only!)
- ✅ Verify resistor values (220-330Ω)
- ✅ Check LED polarity (long leg = anode = +)
- ✅ Test LEDs directly with 3.3V

### Display Shows Nothing

- ✅ Verify I2C address (0x3C is most common)
- ✅ Check SDA/SCL connections (GPIO 21/22)
- ✅ Try different I2C pull-up resistors
- ✅ Test display with I2C scanner sketch

### Ultrasonic Sensor Not Working

- ✅ Verify 5V power supply to sensor
- ✅ Check TRIG (GPIO 19) and ECHO (GPIO 18) connections
- ✅ Ensure no obstacles blocking sensor
- ✅ Check serial monitor for distance readings

### Build Errors

- ✅ Update PlatformIO: `pio upgrade`
- ✅ Clean build: `pio run --target clean`
- ✅ Check library dependencies in `platformio.ini`

### WiFi Not Connecting

- ✅ Verify SSID and password in `config.h`
- ✅ Check 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- ✅ Ensure WiFi is in range
- ✅ Check serial monitor for connection messages

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/AmazingFeature`)
3. **Commit** your changes (`git commit -m 'Add some AmazingFeature'`)
4. **Push** to the branch (`git push origin feature/AmazingFeature`)
5. **Open** a Pull Request

### Areas for Contribution

- 🎨 UI/UX improvements
- 🔊 Additional sound effects
- 📊 Statistics tracking
- 🌐 Multi-language support
- 📱 Mobile app integration
- 🔋 Power optimization
- 📝 Documentation improvements

---

## 📊 Dependencies

This project uses the following libraries:

- **Adafruit GFX Library** - Graphics core
- **Adafruit SSD1306** - OLED display driver
- **U8g2** - Additional graphics support
- **FluxGarage RoboEyes** - Animated eyes
- **ArduinoJson** - JSON parsing for API
- **WiFi** - ESP32 WiFi support
- **HTTPClient** - HTTP requests

All dependencies are automatically managed by PlatformIO.

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- **Pomodoro Technique** by Francesco Cirillo
- **FluxGarage** for the amazing RoboEyes library
- **Adafruit** for excellent hardware libraries
- **PlatformIO** for the best embedded development platform
- **ESP32 Community** for continuous support and inspiration

---

## 📮 Contact & Support

- **Issues:** [GitHub Issues](https://github.com/yourusername/ESP32_poodoro/issues)
- **Discussions:** [GitHub Discussions](https://github.com/yourusername/ESP32_poodoro/discussions)
- **Email:** your.email@example.com

---

<div align="center">

**Made with ❤️ and ☕ using the Pomodoro Technique**

*"Work smart, stay focused, achieve more!"*

⭐ **Star this repo if you found it helpful!** ⭐

</div>
