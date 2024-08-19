// Địa chỉ IP của ESP32 (thay thế bằng địa chỉ IP thực của ESP32 của bạn)
const esp32Ip = "esp32_ip_address";

// Hàm lấy dữ liệu cảm biến từ ESP32
async function getSensorData() {
    try {
        const response = await fetch(`${esp32Ip}/sensor-data`);
        const data = await response.json();
        document.getElementById("temperature").innerText = data.temperature;
        document.getElementById("humidity").innerText = data.humidity;
    } catch (error) {
        console.error("Error fetching sensor data:", error);
    }
}

// Hàm lấy hình ảnh từ camera của ESP32
async function getCameraImage() {
    try {
        const response = await fetch(`${esp32Ip}/capture`);
        const blob = await response.blob();
        const imageUrl = URL.createObjectURL(blob);
        document.getElementById("cameraImage").src = imageUrl;
    } catch (error) {
        console.error("Error fetching camera image:", error);
    }
}

// Hàm cập nhật dữ liệu định kỳ
function updateData() {
    getSensorData();
    getCameraImage();
}

// Cập nhật dữ liệu mỗi 30 giây
setInterval(updateData, 30000);

// Cập nhật dữ liệu ngay khi trang được tải
updateData();