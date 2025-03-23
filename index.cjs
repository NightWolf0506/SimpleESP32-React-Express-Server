const express = require("express");
const cors = require("cors");
const axios = require("axios");
const fs = require("fs");
const path = require("path");
const bodyParser = require("body-parser");

const app = express();

const cors = require('cors');
app.use(cors({
    origin: '*', // Allow all domains or restrict to specific domains
    methods: ['GET', 'POST'],
    allowedHeaders: ['Content-Type'],
}));
app.use(express.json());
app.use(bodyParser.urlencoded({ extended: true }));

let esp32IP = null; // Store ESP32 IP
let latestImage = null; // Store latest uploaded image info

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

// Endpoint to handle the uploaded image from ESP32
app.post("/upload-image", (req, res) => {
    console.log('Received image data:');
    const base64Str = req.body.data;  // Base64 string received from ESP32

    if (!base64Str) {
        return res.status(400).json({ success: false, message: "No image data received" });
    }

    // Remove "data:image/jpeg;base64," if present
    const imageData = base64Str.replace(/^data:image\/jpeg;base64,/, "");

    // Create a file name (e.g., timestamped)
    const fileName = `image_${Date.now()}.jpg`;
    const filePath = path.join(__dirname, 'images', fileName);

    // Save the image to disk
    fs.writeFile(filePath, Buffer.from(imageData, 'base64'), (err) => {
        if (err) {
            console.error('Error saving image:', err);
            return res.status(500).json({ success: false, message: "Error saving image" });
        }

        console.log('Image saved:', fileName);

        // Update the latest image URL
        latestImage = `/images/${fileName}`;

        // Return the image URL
        res.json({ success: true, message: 'Image uploaded successfully', imageUrl: latestImage });
    });
});

// Endpoint to get the latest image
app.get("/latest-image", (req, res) => {
    if (!latestImage) {
        return res.status(404).json({ success: false, message: "No new image available" });
    }
    res.json({ success: true, imageUrl: latestImage });
});

const PORT = 5000;
app.listen(PORT, "0.0.0.0", () => {
    console.log(`Server running on http://0.0.0.0:${PORT}`);
});
