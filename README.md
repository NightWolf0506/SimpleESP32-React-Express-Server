# 📷 ESP32-CAM Image Capture System

This project provides a complete **local image capture pipeline** using an **ESP32-CAM module** and an **Express.js backend server**. It enables:

- Capturing images on button press or timed intervals via ESP32-CAM
- Sending images to a local Express server (`/uploadImage`)
- IP registration and device status tracking
- Seamless communication between hardware and frontend via Express

---

## 🔌 ESP32 Firmware (`/esp32` folder)

The `/esp32` directory contains the **Arduino-based firmware** that runs on the ESP32-CAM module. This firmware is responsible for:

- 📸 Capturing images upon button press or via stream trigger  
- 🌐 Sending captured images to the Express server at `/uploadImage` as Base64 payloads  
- 🟢 Sending the device's IP address to the server on boot for connection tracking  
- 💡 Providing flash LED feedback using GPIO 0 during capture  
- 🔁 Supporting both manual and timed capture modes  

### ✨ Key Features of the Firmware

- Captures high-quality images in **VGA resolution**
- Sends images over HTTP POST to `http://<server-ip>:5000/uploadImage`
- Includes **reset and status logic** using a push-button
- Sends ESP32’s local IP to the backend via `/registerESPIP` (optional endpoint)
- Uses **WiFiManager** for flexible Wi-Fi provisioning (if included)
- Compatible with **ESPAsyncWebServer** for efficient network handling

### 🧩 Main Components

- `setup()` initializes camera, Wi-Fi, and status indicators  
- `loop()` waits for button input or timed interval to trigger image capture  
- `captureAndSendImage()` encodes and sends image as Base64 to server  
- `blinkLED()` gives visual feedback during operations  
- `sendESPIP()` notifies the backend server of the module’s IP address  

> 📦 **Tip:** Use the same network SSID and ensure the Express server IP is reachable from the ESP32 device. Avoid using `localhost` on the ESP32 — use the actual **local IP** of your PC running the Express server.

---

## 🖥️ Express Backend Server (`/` root folder)

This is a simple **Express.js backend server** that acts as a **communication bridge** between:

- 🖥️ **Frontend application** (React or similar)
- 📷 **ESP32-CAM module** (connected to the same local Wi-Fi)

The server handles status checks, image uploads, and command relays locally using `localhost` without requiring external network access.

### 🚀 Features

- 🔄 Communicates with ESP32-CAM over local Wi-Fi  
- 📥 Handles image capture and upload requests  
- 📡 Checks hardware connection status (e.g., ESP32 availability)  
- 🌐 Acts as a middleware between frontend and ESP32 hardware  

### 🔧 Setup Instructions

1. **Install dependencies:**

   ```bash
   npm install
   ```

2. **Run the server:**

   ```bash
   node index.cjs
   ```

   Or use:

   ```bash
   npm start
   ```

3. Server runs at:

   ```
   http://localhost:5000
   ```

### ⚙️ Environment & Local Setup

- Ensure your **ESP32-CAM** is connected to the **same Wi-Fi** as your computer.
- The ESP32 module sends data (like images or status) to this local server using its IP.
- The frontend communicates with this server over `localhost:5000`.

> 💡 You can hardcode or dynamically discover the ESP32 IP using a utility script or the `/getESPIP` route.

### 📡 Endpoints Overview

| Method | Endpoint         | Description                              |
|--------|------------------|------------------------------------------|
| GET    | `/getESPIP`      | Returns current IP of ESP32 module       |
| POST   | `/uploadImage`   | Accepts base64 or binary image data      |
| GET    | `/status`        | Returns basic ESP32 connection status    |
| GET    | `/images/:name`  | Serves saved image by filename           |

### 📁 Image Handling

Captured images are stored in the `/uploads` folder and can be served statically or accessed via the frontend.

### 🧪 Testing

- Use tools like **Postman** or your frontend app to hit endpoints.
- Ensure ESP32-CAM firmware sends data to the correct **server IP and port**.

---

## 📜 License

MIT License
