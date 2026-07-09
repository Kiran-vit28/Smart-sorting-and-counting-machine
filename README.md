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

## Setup & Usage

### 1. Assemble the Hardware
Connect the ESP32-CAM, IR sensor, OLED display, servo motor, and conveyor system according to the circuit diagram.

### 2. Train the Model
- Collect images of the objects to be sorted.
- Create and train a classification model using Edge Impulse.
- Export the trained model for ESP32-CAM deployment.

### 3. Upload the Code
- Open the Arduino IDE.
- Install the ESP32 board package and required libraries.
- Upload the provided code to the ESP32 and ESP32-CAM modules.

### 4. Run the System
- Place objects on the conveyor belt.
- The IR sensor detects incoming objects and updates the count.
- The ESP32-CAM captures and classifies each object.
- The servo motor directs the object to the correct bin.
- The OLED displays the total object count in real time.

## Working
Objects moving on the conveyor belt are detected by an IR sensor and counted. The ESP32-CAM captures images of the objects and classifies them using a trained Edge Impulse model. Based on the classification result, a servo motor directs each object into the appropriate bin while the count is displayed in real time.
