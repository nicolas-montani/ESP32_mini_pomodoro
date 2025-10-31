---
description: Build, upload, and monitor the PlatformIO project
---

Build the project, upload it to the ESP32, and start the serial monitor using PlatformIO CLI.

Run these commands in sequence:
1. `pio run` - Build the project
2. `pio run --target upload` - Upload to the device
3. `pio device monitor` - Start serial monitor

Make sure to check for build errors before uploading. If the build fails, do not proceed with upload.
