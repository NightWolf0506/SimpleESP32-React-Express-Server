const express = require("express");
const cors = require("cors");
const axios = require("axios"); // For sending HTTP requests to ESP32

const app = express();
app.use(cors());
app.use(express.json());

let esp32IP = null; // Store ESP32 IP

// Receive ESP32 IP
app.post("/registerESP", (req, res) => {
    const { ip } = req.body;
    if (ip) {
        esp32IP = ip;
        console.log(`ESP32-CAM registered with IP: ${esp32IP}`);
        res.json({ success: true, message: "ESP32 IP registered", ip: esp32IP });
    } else {
        res.status(400).json({ success: false, message: "Invalid IP" });
    }
});

// Get ESP32 IP
app.get("/getESPIP", (req, res) => {
    if (esp32IP) {
        res.json({ success: true, ip: esp32IP });
    } else {
        res.json({ success: false, message: "ESP32 IP not registered" });
    }
});

// Start ESP32-CAM Streaming
app.post("/startStream", async (req, res) => {
    if (!esp32IP) {
        return res.status(400).json({ success: false, message: "ESP32 IP not registered" });
    }
    try {
        await axios.get(`http://${esp32IP}/start`); // ESP32 should handle "/start"
        console.log(`Stream started on ${esp32IP}`);
        res.json({ success: true, message: "Streaming started" });
    } catch (error) {
        console.error("Error starting stream:", error.message);
        res.status(500).json({ success: false, message: "Failed to start streaming" });
    }
});

// Stop ESP32-CAM Streaming
app.post("/stopStream", async (req, res) => {
    if (!esp32IP) {
        return res.status(400).json({ success: false, message: "ESP32 IP not registered" });
    }
    try {
        await axios.get(`http://${esp32IP}/stop`); // ESP32 should handle "/stop"
        console.log(`Stream stopped on ${esp32IP}`);
        res.json({ success: true, message: "Streaming stopped" });
    } catch (error) {
        console.error("Error stopping stream:", error.message);
        res.status(500).json({ success: false, message: "Failed to stop streaming" });
    }
});

const PORT = 5000;
app.listen(PORT, "0.0.0.0", () => {
    console.log(`Server running on http://0.0.0.0:${PORT}`);
});
