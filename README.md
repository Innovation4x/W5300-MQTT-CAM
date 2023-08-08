# W5300-MQTT-CAM: Vision-based IoT with MQTT & HTTP Access

1. **Title (max 60 characters):**
   W5300-MQTT-Cam: Vision-based IoT with MQTT & HTTP Access

2. **Summary (max 140 characters):**
   An IoT solution utilizing W5300-TOE-Shield for capturing images, accessible via MQTT & HTTP.

3. **Hardware Components:**
   - WIZnet W5300-TOE-Shield for ethernet connection on STM32 Nucleo-144 boards.
   - OpenMV camera module on the STM32 Nucleo-144 board.

4. **Software Components:**
   - Flask web service for receiving captured images using the POST method.
   - MQTT broker for connecting W5300-TOE-Shield and other MQTT clients.
   - Android app for controlling camera capture and displaying the captured image.

5. **Tags (max 5):**
   - W5300
   - MQTT
   - OpenMV
   - IoT
   - Vision Applications

6. **Descriptions (As detailed as possible):**

   **Introduction**
   The W5300-MQTT-Cam project aims to bridge the realm of vision with IoT devices. By leveraging the capabilities of the W5300-TOE-Shield, the project not only captures images but also allows any internet-connected device to access them via MQTT and HTTP. This adds a new dimension to how IoT devices can be used for vision applications.

   **Components and Configuration**
   - **WIZnet W5300-TOE-Shield**: The core of the system that provides high-speed internet access to STM32 Nucleo-144 boards. It facilitates the communication with other devices.
   - **OpenMV Camera Module**: This is attached to the STM32 Nucleo-144 board and captures images via a serial port connection.
   - **Flask Web Service**: It serves as the interface for receiving the captured image through the POST method. The image becomes accessible via an HTTP URL, making it retrievable from various devices.
   - **MQTT Broker**: It forms the link between W5300-TOE-Shield and other MQTT clients, making the whole system highly interoperable.
   - **Android App**: This is the user interface, allowing users to send a camera capture command to the STM32 Nucleo-144 board through MQTT. It also displays the captured image on the screen.

   **Implementation and Operation**
   The project is designed with scalability and reusability in mind. It starts with the activation of the camera module via the Android app, sending a capture command to the STM32 board. Upon capturing, the image is sent to the Flask web service and is available for viewing via HTTP.

   **Creativity and Reusability**
   This project showcases creativity in combining vision with IoT. The application's design makes it reusable for various purposes, from security monitoring to remote visualization. 

   **Use Case & Applications**


   **Conclusion**
   The W5300-MQTT-Cam project opens up new horizons in the IoT landscape. It offers a practical and innovative way to leverage image capturing with IoT devices, making it a compelling solution for various applications.


