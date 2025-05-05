# **ESP32-CAM Backend Server**

This is a simple **Express.js backend server** that acts as a **communication bridge** between:

- 🖥️ **Frontend application** (React or similar)
- 📷 **ESP32-CAM module** (connected to the same local Wi-Fi)

The server handles status checks, image uploads, and command relays locally using `localhost` without requiring external network access.

---

## 🚀 **Features**

- 🔄 Communicates with ESP32-CAM over local Wi-Fi
- 📥 Handles image capture and upload requests
- 📡 Checks hardware connection status (e.g., ESP32 availability)
- 🌐 Acts as a middleware between frontend and ESP32 hardware

---



## 🔧 **Setup Instructions**

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

3. The server will run on:

   ```
   http://localhost:5000
   ```

---

## ⚙️ **Environment & Local Setup**

- Ensure your **ESP32-CAM** is connected to the **same Wi-Fi network** as your computer.
- The ESP32 module sends data (like images or status) to this local server using its IP.
- The frontend communicates with this server over `localhost:5000`.

> 💡 You can hardcode or dynamically discover the ESP32 IP using a utility script or `/getESPIP` route.

---

## 📡 **Endpoints Overview**

| Method | Endpoint        | Description                             |
|--------|------------------|-----------------------------------------|
| GET    | `/getESPIP`      | Returns current IP of ESP32 module      |
| POST   | `/uploadImage`   | Accepts base64 or binary image data     |
| GET    | `/status`        | Returns basic ESP32 connection status   |
| GET    | `/images/:name`  | Serves saved image by filename          |

---

## 📁 **Image Handling**

Captured images are stored in the `/uploads` folder and can be served statically or accessed via the frontend.

---

## 🧪 **Testing**

- Use tools like **Postman** or your frontend app to hit endpoints.
- Ensure ESP32-CAM firmware sends data to the correct **server IP and port**.

---

## 📜 **License**

MIT License
