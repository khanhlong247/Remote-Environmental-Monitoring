#include "esp_camera.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// Định nghĩa model của ESP32-CAM
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Cấu hình cảm biến DHT
#define DHTPIN 14  // Chân GPIO của ESP32-CAM mà cảm biến DHT được kết nối
#define DHTTYPE DHT11 // Loại cảm biến DHT sử dụng (có thể thay bằng DHT22)
DHT dht(DHTPIN, DHTTYPE); // Khởi tạo đối tượng DHT

// Thông tin mạng Wi-Fi
const char* ssid = "your_SSID";       // Tên Wi-Fi
const char* password = "your_PASSWORD";  // Mật khẩu Wi-Fi

// Khởi tạo server trên ESP32
AsyncWebServer server(80);

// Biến lưu dữ liệu cảm biến
float temperature = 0.0;
float humidity = 0.0;

// Hàm để xử lý yêu cầu sensor-data
void handleSensorData(AsyncWebServerRequest *request) {
    // Tạo chuỗi JSON chứa nhiệt độ và độ ẩm
    String json = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
    request->send(200, "application/json", json); // Gửi phản hồi với dữ liệu JSON
}

// Hàm để xử lý yêu cầu capture (ảnh từ camera)
void handleCapture(AsyncWebServerRequest *request) {
    // Chụp ảnh từ camera
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) { // Nếu không thể chụp ảnh
        request->send(500, "text/plain", "Camera capture failed");
        return;
    }
    // Gửi ảnh dưới dạng file JPEG
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/jpeg", fb->buf, fb->len);
    response->addHeader("Content-Disposition", "inline; filename=capture.jpg");
    request->send(response);
    esp_camera_fb_return(fb); // Trả lại buffer sau khi gửi xong
}

void setup() {
    Serial.begin(115200);

    // Cấu hình camera
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
    config.xclk_freq_hz = 20000000; // Tần số XCLK cho camera (20 MHz)
    config.pixel_format = PIXFORMAT_JPEG; // Định dạng pixel của ảnh là JPEG

    // Thiết lập kích thước khung hình và chất lượng ảnh
    config.frame_size = FRAMESIZE_SVGA; // Kích thước khung hình SVGA (800x600)
    config.jpeg_quality = 10;           // Chất lượng ảnh JPEG (giá trị thấp hơn sẽ tạo ảnh có kích thước nhỏ hơn)
    config.fb_count = 1;                // Số lượng frame buffer

    // Khởi tạo camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err); // Nếu khởi tạo camera thất bại
        return;
    }

    // Khởi tạo cảm biến DHT
    dht.begin();

    // Kết nối tới Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { // Chờ đến khi kết nối thành công
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi"); // Thông báo đã kết nối thành công

    // Khởi tạo các endpoint cho server
    server.on("/sensor-data", HTTP_GET, handleSensorData); // Endpoint cho dữ liệu cảm biến
    server.on("/capture", HTTP_GET, handleCapture);       // Endpoint cho ảnh từ camera
    
    server.begin(); // Bắt đầu server
}

void loop() {
    // Đọc nhiệt độ và độ ẩm từ cảm biến DHT
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) { // Kiểm tra xem dữ liệu có hợp lệ không
        Serial.println("Failed to read from DHT sensor!"); // Nếu không đọc được dữ liệu từ cảm biến
    } else {
        Serial.println("Temperature: " + String(temperature) + " °C");
        Serial.println("Humidity: " + String(humidity) + " %");
    }

    delay(1000); // Đợi 1 giây trước khi đọc lại dữ liệu cảm biến
}
