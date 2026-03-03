/**
 * cam_firmware.ino — AtomS3R-CAM Vision Firmware (Phase 5)
 * 
 * Niezależny firmware dla modułu wizyjnego z kamerą GC0308.
 * Uruchawia kamerę, mockuje detekcję obiektów i działa jako podrzędny (Slave) na magistrali I2C (adres 0x40).
 * 
 * Hardware:
 * - M5Stack AtomS3R-CAM
 * - Sensor: GC0308
 * - I2C Grove port na pinach: SDA=2, SCL=1 (na M5AtomS3R)
 */

#include <Arduino.h>
#include <Wire.h>
#include "esp_camera.h"

// =========================================================
// Konfiguracja sprzętowa (AtomS3R-CAM z GC0308)
// =========================================================
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     39
#define SIOD_GPIO_NUM     12
#define SIOC_GPIO_NUM     11
#define Y9_GPIO_NUM       42
#define Y8_GPIO_NUM       41
#define Y7_GPIO_NUM       40
#define Y6_GPIO_NUM       38
#define Y5_GPIO_NUM       37
#define Y4_GPIO_NUM       36
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       34
#define VSYNC_GPIO_NUM    46
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     48

// =========================================================
// Konfiguracja I2C Slave
// =========================================================
#define I2C_SLAVE_ADDR    0x40
// Ważne: W typowym AtomS3 (bez R) port I2C Grove to G2 (SDA), G1 (SCL).
#define SDA_PIN           2
#define SCL_PIN           1

// =========================================================
// Stan detekcji (Mock)
// =========================================================
uint8_t detectionStatus = 0; // 0 = Clear, 1 = Obstacle
unsigned long lastMockTime = 0;

// =========================================================
// Inicializacja Kamery
// =========================================================
bool initCamera() {
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
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size = FRAMESIZE_QVGA; // 320x240
    config.jpeg_quality = 12;
    config.fb_count = 1;

    // Inicjalizacja
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    // Obniżenie rotacji by pasowała do układu mocowania, jeśli trzeba
    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    
    return true;
}

// =========================================================
// Obsługa żądań protokołu I2C od Mastera
// =========================================================
void onRequest() {
    // M5Stick prosi o status, zwracamy 1 bajt logiki ostrzeżenia
    Wire.write(detectionStatus);
}

void setup() {
    Serial.begin(115200);
    Serial.println("=============================================");
    Serial.println("   AtomS3R-CAM Vision Node  (I2C 0x40)       ");
    Serial.println("=============================================");

    // Start Camera
    if (initCamera()) {
        Serial.println("[CAM] Initialized successfully");
    } else {
        Serial.println("[CAM] Error initializing");
    }

    // Start I2C Slave
    bool i2c_ok = Wire.begin(I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN, 100000);
    if(i2c_ok) {
        Wire.onRequest(onRequest);
        Serial.println("[I2C] Started as Slave on address 0x40");
    } else {
        Serial.println("[I2C] Failed to start Slave");
    }
}

void loop() {
    // Odczyt i zwolnienie klatki (na ten moment symulacja TensorFlow i zwalnianie ramek)
    camera_fb_t * fb = esp_camera_fb_get();
    if (fb) {
        // Tu (w Phase x) trafi zasilenie tensora w TFLite
        esp_camera_fb_return(fb);
    } else {
        Serial.println("[CAM] Error pulling frame");
    }

    // Prosty Mock logiki detekcji co 5 sekund zapala przeszkodę na 2 sekundy
    unsigned long now = millis();
    if (now - lastMockTime > 5000) {
        lastMockTime = now;
        detectionStatus = 1; // Przeszkoda!
        Serial.println("[AI] Mock: OBSTACLE_DETECTED");
    } else if (detectionStatus == 1 && (now - lastMockTime > 2000)) {
        detectionStatus = 0; // Czysto
        Serial.println("[AI] Mock: PATH_CLEAR");
    }

    delay(200); // 5 FPS processing
}
