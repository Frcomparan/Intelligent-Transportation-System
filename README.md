# Tangram Challenge Robot

## Overview

This project is a robotic system designed to solve a Tangram-like challenge. It utilizes computer vision (YOLO object detection) to identify Tangram pieces and predefined patterns, and a robotic arm to physically rearrange these pieces. The system integrates a Python-based server for high-level logic and communication, an ESP32 microcontroller responsible for direct robot control (movement and arm manipulation), and an ESP32 CAM for live image capturing.

## Features

*   **Robotic Arm Control:** Precise multi-servo robotic arm for picking and placing Tangram pieces.
*   **Line Following Navigation:** The robot can autonomously follow a designated line on the surface for accurate movement between points.
*   **Image Capture:** Utilizes an ESP32 CAM to capture real-time images of Tangram pieces and base patterns.
*   **YOLO-Based Object Recognition:** Employs You Only Look Once (YOLO) models to:
    *   Identify different Tangram shapes.
    *   Recognize predefined base patterns.
*   **Automated Tangram Challenge Routine:** Implements a full sequence for:
    *   Scanning different locations (bases) to identify a target pattern.
    *   Picking up individual Tangram pieces from a starting configuration.
    *   Transporting and placing the pieces onto the identified target base.
*   **HTTP Communication Protocol:** The Python server communicates with the ESP32 robot controller via HTTP requests to send commands and receive status.

## Hardware Components

The following hardware components are essential for building and running this project:

*   **ESP32 Microcontroller:** One ESP32 board for controlling the robot's motors, servos, and line-following sensors. (Specific pin configurations can be found in `server/server.ino`).
*   **ESP32 CAM:** A separate ESP32-based camera module for capturing images.
*   **Robot Chassis:** A mobile robot platform equipped with:
    *   Motors for driving the wheels.
    *   Line following sensors (IR or similar).
*   **Robotic Arm:** A multi-degree-of-freedom robotic arm with servo motors and a gripper mechanism.
*   **Power Supply:** Appropriate power sources (batteries or AC adapters) for the ESP32s, motors, and servos.
*   **Jumper Wires and Breadboard:** For making connections between components (if not using a custom PCB).

## Software and Setup

### 1. Python Environment (Host Computer)

*   **Python:** Ensure you have Python 3.x installed.
*   **Dependencies:** The project relies on several Python libraries. Install them using the `requirements.txt` file:
    ```bash
    pip install -r requirements.txt
    ```
    Key libraries include:
    *   `opencv-python` (for image processing)
    *   `requests` (for HTTP communication)
    *   `ultralytics` (for YOLO object detection)
    *   (Verify `requirements.txt` for a complete list)

### 2. ESP32 Firmware (Robot Controller)

*   **Arduino IDE:** Use the Arduino IDE to flash the firmware to the ESP32 microcontroller.
*   **Libraries:** Ensure you have the following libraries installed in your Arduino IDE:
    *   `WiFi`
    *   `ESPAsyncWebServer`
    *   `ESP32Servo`
    (These typically come with the ESP32 board support package or can be installed via the Library Manager).
*   **Firmware:** The firmware code is located at `server/server.ino`.
*   **Wi-Fi Credentials:** **Crucially, you must update the Wi-Fi SSID and password** within the `server/server.ino` file before uploading:
    ```cpp
    const char *ssid = "YOUR_WIFI_SSID";     // Change to your Wi-Fi network name
    const char *password = "YOUR_WIFI_PASSWORD"; // Change to your Wi-Fi password
    ```
*   **Flashing:** Upload the modified `server/server.ino` sketch to the ESP32 board connected to your robot.

### 3. ESP32 CAM (Camera Module)

*   The ESP32 CAM should be running firmware that starts a video stream accessible via its IP address (e.g., the "CameraWebServer" example sketch from the Arduino IDE, or similar).
*   Note its IP address once it connects to your Wi-Fi network.

### 4. YOLO Models

*   This project uses pre-trained YOLO models for object detection. These models are expected to be in a directory named `yolo_recognizer/` at the root of the project.
*   The Python script `CarritoServer.py` refers to:
    *   `./yolo_recognizer/only_real.pt` (for live Tangram piece recognition)
    *   `./yolo_recognizer/only_images_v4.pt` (for Tangram pattern recognition on the floor)
*   **Ensure these model files (`.pt`) are present in the `yolo_recognizer/` directory.** (These models are not included in this repository and need to be obtained or trained separately).

### 5. IP Address Configuration

*   The Python script `CarritoServer.py` needs to know the IP addresses of both the ESP32 robot controller and the ESP32 CAM.
*   Update these IP addresses at the beginning of `CarritoServer.py`:
    ```python
    # IP de la ESP32 CAM (Robot Controller ESP32)
    ip = "http://YOUR_ROBOT_ESP32_IP"
    # Example: ip = "http://192.168.1.100"

    # IP de la ESP32 CAM (Camera ESP32)
    stream_url = 'http://YOUR_ESP32_CAM_IP'
    # Example: stream_url = 'http://192.168.1.101'
    ```
    *Note: The comments in the original script might be slightly confusing regarding which IP is which. `ip` is for the ESP32 controlling the robot/arm, and `stream_url` is for the ESP32 CAM.*

## How to Run

1.  **Hardware & Network Setup:**
    *   Ensure all hardware components are correctly assembled and powered on.
    *   Verify that the ESP32 robot controller and the ESP32 CAM are connected to the same Wi-Fi network that the host computer (running `CarritoServer.py`) is on.
2.  **Configuration Checks:**
    *   Confirm that you have updated the Wi-Fi credentials in `server/server.ino` and flashed it to the robot's ESP32.
    *   Double-check that the IP addresses for the robot's ESP32 (`ip`) and the ESP32 CAM (`stream_url`) are correctly set in `CarritoServer.py`.
    *   Make sure the YOLO model files (`.pt`) are in the `yolo_recognizer/` directory.
    *   Ensure the `uploads/` directory exists (the script creates it, but good to be aware).
3.  **Start the Python Server:**
    *   Navigate to the project's root directory in your terminal.
    *   Run the main server script:
        ```bash
        python CarritoServer.py
        ```
4.  **Using the Command-Line Interface:**
    *   Once the server is running, it will prompt you to enter commands. The available commands are:
        *   `bodega`: Initiates the full Tangram challenge routine (photo sequence, pattern recognition, and piece manipulation).
        *   `sade`: Commands the line-following robot to move forward to the next detected stop point.
        *   `sdet`: Commands the line-following robot to move backward to the next detected stop point.
        *   `q`: Quits the server application.
    *   You can also send specific arm movement commands or other custom commands if you modify/extend the `main()` loop in `CarritoServer.py`. (The script currently shows direct calls to `enviar_comando_brazo` and `enviar_comando_seguidor` for more granular control if uncommented or used directly in code).

**Initial Setup Note:** It's recommended to first test individual components:
*   Test ESP32 CAM stream in a browser.
*   Test basic robot movements and arm controls by sending direct HTTP requests (e.g., using `curl` or a tool like Postman) to the ESP32 robot controller's IP address and endpoints (`/seguidor`, `/brazo`) before running the full `CarritoServer.py` script. This can help in debugging connections and basic functionality.

## Project Structure

```
.
├── .gitignore
├── CarritoServer.py        # Main Python server application for high-level control, CV, and communication.
├── commands.txt            # Likely a text file for reference or notes on commands.
├── requirements.txt        # Python dependencies for the host server.
├── server/
│   └── server.ino          # Arduino firmware for the ESP32 robot controller (motors, servos, line sensors).
├── templates/
│   ├── base1.jpg           # Reference images for Tangram base patterns.
│   ├── base2.jpg
│   ├── base3.jpg
│   └── base4.jpg
├── uploads/                # Directory where images captured by the ESP32 CAM are saved by CarritoServer.py.
└── yolo_recognizer/        # (Expected directory) Contains YOLO model files (.pt) for object detection.
```

*   **`CarritoServer.py`**: The core Python script that orchestrates the robot's actions, performs image processing and YOLO detection, and communicates with the ESP32 robot.
*   **`server/server.ino`**: Firmware for the ESP32 microcontroller on the robot. It handles low-level control of motors for movement, servos for the robotic arm, and reads line-following sensors. It also hosts a web server to receive commands from `CarritoServer.py`.
*   **`requirements.txt`**: Lists the Python packages required to run `CarritoServer.py`.
*   **`templates/`**: Contains pre-captured images of different "bases" or Tangram patterns used as references by the YOLO model (`model_en_piso`) for identifying where to move the pieces.
*   **`uploads/`**: This directory is used by `CarritoServer.py` to store images captured during its operation (e.g., photos of the initial setup `bi.jpg` and photos of each base `b1.jpg`-`b4.jpg`).
*   **`yolo_recognizer/`**: This directory (not present in the initial `ls` but inferred from code) is crucial as it must contain the YOLO model weights files (`.pt`) used for recognizing Tangram pieces and patterns.
*   **`commands.txt`**: Its exact use isn't detailed in the code, but it might contain a list of manual commands or notes for development.

## Future Enhancements / To-Do

*   **Web Interface:** Develop a web-based UI for easier control and monitoring, replacing or supplementing the current command-line interface.
*   **Improved Error Handling:** Implement more robust error handling and recovery mechanisms (e.g., if a piece is not picked up correctly, or if a base is not found).
*   **Dynamic IP Discovery:** Instead of hardcoding IP addresses, implement a discovery mechanism (e.g., mDNS) to find the ESP32 robot and CAM on the network.
*   **Calibration Routines:** Add calibration routines for:
    *   Camera calibration.
    *   Robotic arm positions (e.g., for picking up pieces from different starting points).
    *   Line follower sensor thresholds.
*   **Configuration File:** Move hardcoded settings (like IP addresses, YOLO model paths, servo positions) into a separate configuration file (e.g., JSON or YAML).
*   **Advanced Object Detection:** Explore more sophisticated object detection techniques or train more robust YOLO models for varying lighting conditions or piece orientations.
*   **Obstacle Avoidance:** Integrate sensors for basic obstacle avoidance during robot navigation.
*   **Modular Code Structure:** Refactor `CarritoServer.py` into smaller, more modular functions or classes for better readability and maintainability.
*   **Detailed Logging:** Implement more comprehensive logging for debugging and tracking system operations.
