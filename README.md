# Smart Sorting and Counting Machine

An ESP32-CAM based automation system that detects, classifies, sorts, and counts objects in real time using computer vision and embedded systems.

## Features
- Real-time object counting using an IR sensor
- Color-based object classification using ESP32-CAM
- Automated sorting using a servo motor
- Live count display on OLED
- Edge Impulse machine learning integration
- Wireless monitoring through ESP32 Wi-Fi capabilities

## Hardware Used
- ESP32-CAM
- ESP32
- IR Sensor
- Servo Motor
- OLED Display
- DC Motors
- Conveyor Belt
- FTDI Programmer

## Working
Objects moving on the conveyor belt are detected by an IR sensor and counted. The ESP32-CAM captures images of the objects and classifies them using a trained Edge Impulse model. Based on the classification result, a servo motor directs each object into the appropriate bin while the count is displayed in real time.


