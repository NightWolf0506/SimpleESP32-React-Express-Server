#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include "esp_camera.h"
#include <WiFiManager.h>

//======================================== 

#include "esp_timer.h"
#include "img_converters.h" // Required for frame2jpg
#include "Arduino.h"
#include <HTTPClient.h>
#include <ESPAsyncWebServer.h>


#define JPEG_QUALITY 12
//======================================== CAMERA_MODEL_AI_THINKER GPIO.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define TRIGGER_PIN 12  // GPIO 4

//======================================== 
#include <Preferences.h>
Preferences preferences;
bool shouldSaveConfig = false;
// LED Flash PIN (GPIO 4)
#define FLASH_LED_PIN 4

//======================================== Enter your WiFi ssid and password.


//======================================== Variables for Timer/Millis.
unsigned long previousMillis = 0; 
const int Interval = 20000;
int timeout=120;
int buttonState=0; 
unsigned long buttonPressTime = 0;
bool buttonHeld = false;//--> Capture and Send a photo every 20 seconds.
//======================================== 

// Variable to set capture photo with LED Flash.
// Set to "false", then the Flash LED will not light up when capturing a photo.
// Set to "true", then the Flash LED lights up when capturing a photo.
bool LED_Flash_ON = true;

// Initialize WiFiClientSecure.
WiFiClientSecure client;
AsyncWebServer server(80);
bool isStreaming = false;


void saveConfigCallback () {
  Serial.println("⚙️  Should save config");
  shouldSaveConfig = true;

}void SendCapturedPhotosToExpress() {
  Serial.println();
  Serial.println("-----------");
  Serial.println("📸 Capturing and sending photo to Express server...");

  camera_fb_t * fb = NULL;
  fb=esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed");
    return;
  }

  Serial.printf("📦 Raw image size: %u bytes\n", fb->len);
  Serial.printf("Free heap before sending: %u bytes\n", ESP.getFreeHeap());

  // Convert to JPEG if needed
  size_t jpg_size = 0;
  uint8_t* jpg_buf = nullptr;

  if (fb->format != PIXFORMAT_JPEG) {
    jpg_buf = convertToJPEG(fb, &jpg_size);
    if (!jpg_buf) {
      Serial.println("❌ JPEG conversion failed. Aborting...");
      esp_camera_fb_return(fb);
      return;
    }
  } else {
    jpg_buf = fb->buf;
    jpg_size = fb->len;
  }

  String filename = "esp32_" + String(millis()) + ".jpg";

  WiFiClient client;
  HTTPClient http;

  String endpoint = "http://" + String(serverUrl) + "/uploadBinary";
  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/octet-stream");
  http.addHeader("X-Filename", filename);

  int httpResponseCode = http.POST(jpg_buf, jpg_size);

  if (httpResponseCode > 0) {
    Serial.println("✅ Image sent to server successfully!");
    String response = http.getString();
    Serial.println("📨 Server response: " + response);
  } else {
    Serial.println("❌ Failed to send image. HTTP error: " + String(httpResponseCode));
  }

  http.end();

  // Free JPEG buffer if we allocated it
  if (fb->format != PIXFORMAT_JPEG && jpg_buf != nullptr) {
    free(jpg_buf);
  }

  esp_camera_fb_return(fb);


  


 
}


void SendFoodPhotosToExpress() {
  Serial.println();
  Serial.println("-----------");
  Serial.println("📸 Capturing and sending photo to Express server...");

  camera_fb_t * fb = NULL;
  fb=esp_camera_fb_get();
  if (!fb) {
    Serial.println("❌ Camera capture failed");
    return;
  }

  Serial.printf("📦 Raw image size: %u bytes\n", fb->len);
  Serial.printf("Free heap before sending: %u bytes\n", ESP.getFreeHeap());

  // Convert to JPEG if needed
  size_t jpg_size = 0;
  uint8_t* jpg_buf = nullptr;

  if (fb->format != PIXFORMAT_JPEG) {
    jpg_buf = convertToJPEG(fb, &jpg_size);
    if (!jpg_buf) {
      Serial.println("❌ JPEG conversion failed. Aborting...");
      esp_camera_fb_return(fb);
      return;
    }
  } else {
    jpg_buf = fb->buf;
    jpg_size = fb->len;
  }

  String filename = "esp32_" + String(millis()) + ".jpg";

  WiFiClient client;
  HTTPClient http;

  String endpoint = "http://" + String(serverUrl) + "/uploadFoodImage";
  http.begin(client, endpoint);
  http.addHeader("Content-Type", "application/octet-stream");
  http.addHeader("X-Filename", filename);

  int httpResponseCode = http.POST(jpg_buf, jpg_size);

  if (httpResponseCode > 0) {
    Serial.println("✅ Image sent to server successfully!");
    String response = http.getString();
    Serial.println("📨 Server response: " + response);
  } else {
    Serial.println("❌ Failed to send image. HTTP error: " + String(httpResponseCode));
  }

  http.end();

  // Free JPEG buffer if we allocated it
  if (fb->format != PIXFORMAT_JPEG && jpg_buf != nullptr) {
    free(jpg_buf);
  }

  esp_camera_fb_return(fb);
  delay(2500);
  for (int i = 0; i < 4; i++) {
    digitalWrite(FLASH_LED_PIN, HIGH);  // Turn LED on
    delay(100);  // Short delay (adjust to make it blink faster)
    digitalWrite(FLASH_LED_PIN, LOW);   // Turn LED off
    delay(100);  // Short delay (adjust to make it blink faster)
}

  

  Serial.println("-----------");
}


void sendIPToServer() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        Serial.print("IP?: ");
      Serial.print(serverUrl);
      String serverUrlAct = String("http://") + serverUrl + "/registerESP";
// Change to actual server IP
        String postData = "{\"ip\":\"" + WiFi.localIP().toString() + "\"}";
        
        Serial.print("Sending data: ");
        Serial.println(postData);

        http.begin(client, serverUrlAct);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Connection", "close"); // Ensure connection closes properly

        int httpResponseCode = http.POST(postData);
        
        if (httpResponseCode > 0) {
            Serial.println("✅ IP sent successfully!");
            Serial.print("Server response: ");
            Serial.println(http.getString());
        } else {
            Serial.print("⚠️ Error sending IP: ");
            Serial.println(httpResponseCode);
        }

        http.end();
    } else {
        Serial.println("❌ WiFi not connected. Cannot send IP.");
    }
}


void startStreaming() {
  
        // isStreaming = true;
        Serial.println("✅ Starting camera stream...");
        SendCapturedPhotosToExpress();
        delay(1000);
        
        

}

// Function to stop streaming





uint8_t* convertToJPEG(camera_fb_t* fb, size_t* jpg_size) {
    uint8_t *jpeg_buf = NULL;
    if (!frame2jpg(fb, 80, &jpeg_buf, jpg_size)) { // 80 = JPEG Quality
        Serial.println("JPEG conversion failed!");
        return NULL;
    }
    return jpeg_buf;
}

______________________________________________________________ VOID SETUP()
void setup() {
  // put your setup code here, to run once:
  
  // Disable brownout detector.
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.println();
  delay(1000);


  pinMode(FLASH_LED_PIN, OUTPUT);
  
  // Setting the ESP32 WiFi to station mode.
  Serial.println();
  Serial.println("Setting the ESP32 WiFi to station mode.");
  pinMode(TRIGGER_PIN,INPUT_PULLUP);
  // WiFi.mode(WIFI_STA);

  //---------------------------------------- The process of connecting ESP32 CAM with WiFi Hotspot / WiFi Router.
  Serial.println();
  Serial.print("Connecting to : !!!!");
  // Serial.println(ssid);
  preferences.begin("esp32", true);  // Read-only mode
  String savedAPIKey = preferences.getString("apiServer", "default string");  // default if not set
  preferences.end();

    WiFiManagerParameter custom_text_box(
    "EnterAPIServer",               // id
    "Enter your API Server Key",    // placeholder
    savedAPIKey.c_str(),            // default value
    50                              // max length
  );
  // WiFi.begin(ssid, password);
 WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_text_box);

  // AutoConnect tries saved credentials, else opens AP
  if (!wifiManager.autoConnect("ESP32-Setup", "password")) {
    Serial.println("❌ Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ WiFi connected!");
  Serial.println(WiFi.localIP());

  // Save custom parameter if flagged
  if (shouldSaveConfig) {
    Serial.println("💾 Saving custom parameters...");

    preferences.begin("esp32", false);  // write mode
    preferences.putString("apiServer", custom_text_box.getValue());
    preferences.end();

    Serial.println("✅ Custom parameter saved:");
    Serial.println(custom_text_box.getValue());
  }

  // For debug: print final loaded API key
  preferences.begin("esp32", true);
  Serial.print("📦 Loaded API Key: ");
  Serial.println(preferences.getString("apiServer"));
  serverUrl = preferences.getString("apiServer");
  preferences.end();

  Serial.println(custom_text_box.getValue());
  
  // The process timeout of connecting ESP32 CAM with WiFi Hotspot / WiFi Router is 20 seconds.
  // If within 20 seconds the ESP32 CAM has not been successfully connected to WiFi, the ESP32 CAM will restart.
  // I made this condition because on my ESP32-CAM, there are times when it seems like it can't connect to WiFi, so it needs to be restarted to be able to connect to WiFi.
  int connecting_process_timed_out = 20; //--> 20 = 20 seconds.
  connecting_process_timed_out = connecting_process_timed_out * 2;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(250);
    digitalWrite(FLASH_LED_PIN, LOW);
    delay(250);
    if(connecting_process_timed_out > 0) connecting_process_timed_out--;
    if(connecting_process_timed_out == 0) {
      Serial.println();
      Serial.print("Failed to connect to ");
      Serial.println(ssid);
      Serial.println("Restarting the ESP32 CAM.");
      delay(1000);
      ESP.restart();
    }
  }

  digitalWrite(FLASH_LED_PIN, LOW);
  
  Serial.println();
  Serial.print("Successfully connected to ");
  Serial.println(ssid);
  Serial.print("Connected! IP Address: ");
    Serial.println(WiFi.localIP());

    sendIPToServer();
    
  //Serial.print("ESP32-CAM IP Address: ");
  //Serial.println(WiFi.localIP());
  //---------------------------------------- 

  //---------------------------------------- Set the camera ESP32 CAM.
  Serial.println();
  Serial.println("Set the camera ESP32 CAM...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 16000000;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
 config.pixel_format = PIXFORMAT_RGB565;  
  
  // init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA; 
    config.grab_mode = CAMERA_GRAB_LATEST;
   //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 8;  //0-63 lower number means higher quality
    config.fb_count = 2;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    Serial.println();
    Serial.println("Restarting the ESP32 CAM.");
    delay(1000);
    // ESP.restart();
  }

  sensor_t * s = esp_camera_sensor_get();

  // Selectable camera resolution details :
  // -UXGA   = 1600 x 1200 pixels
  // -SXGA   = 1280 x 1024 pixels
  // -XGA    = 1024 x 768  pixels
  // -SVGA   = 800 x 600   pixels
  // -VGA    = 640 x 480   pixels
  // -CIF    = 352 x 288   pixels
  // -QVGA   = 320 x 240   pixels
  // -HQVGA  = 240 x 160   pixels
  // -QQVGA  = 160 x 120   pixels
  s->set_framesize(s, FRAMESIZE_VGA);  //--> UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA

  Serial.println("Setting the camera successfully.");
  Serial.println();

 

    server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
        startStreaming();

        request->send(200, "text/plain", "Streaming started. View at http://ESP_IP_ADDRESS/stream");
    });

    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "ESP32 is active");
    });

    server.begin();
delay(1000);
    SendCapturedPhotosToExpress();
    delay(1000);

  

}
//________________________________________________________________________________ 
void loop() {
  buttonState = digitalRead(TRIGGER_PIN);

  if (buttonState == LOW) {
     // Button is pressed
    if (!buttonHeld) {
      buttonPressTime = millis();  // Start the timer
      buttonHeld = true;
    }

    if (millis() - buttonPressTime >= 9000) {  // Held for 9+ seconds
      Serial.println("Button held for 9 seconds! Resetting WiFi settings...");
      for (int i = 0; i < 2; i++) {
        digitalWrite(FLASH_LED_PIN, HIGH);
        delay(600);
        digitalWrite(FLASH_LED_PIN, LOW);
        delay(600);
      }

      WiFiManager wfm;
      wfm.resetSettings();
      delay(500);
      ESP.restart();  // Restart ESP32 to apply changes
    }
  } else {
    // Button is released
    if (buttonHeld) {
      unsigned long pressDuration = millis() - buttonPressTime;

      if (pressDuration < 2000) {  // Considered a short tap
        Serial.println("ESP32 Click detected! Capturing image...");
        delay(800);
        SendFoodPhotosToExpress();
        delay(300);
      }

      // Reset state after release
      buttonHeld = false;
      buttonPressTime = 0;
    }
  }

  delay(100);  // debounce
}
