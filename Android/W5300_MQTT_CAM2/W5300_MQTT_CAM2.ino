/*
  Telnet client

 This sketch connects to a a telnet server (http://www.google.com)
 using an Arduino Wiznet Ethernet shield.  You'll need a telnet server
 to test this with.
 Processing's ChatServer example (part of the network library) works well,
 running on port 10002. It can be found as part of the examples
 in the Processing application, available at
 http://processing.org/

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13

 created 14 Sep 2010
 modified 9 Apr 2012
 by Tom Igoe
 */

#include <Ethernet.h>
#include <Wire.h>
// MQTT [[ 
#include <PubSubClient.h>
// ]]
#include <ArduinoJson.h>
#include "HardwareSerial.h"

const char* device_unique_id = "ArdCam1_" __DATE__ "_" __TIME__;

//#define SERIAL_OUTPUT
#ifdef SERIAL_OUTPUT
#define PRINT(x)   Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)   
#define PRINTLN(x) 
#endif

#define SYNC  0x96

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x77, 0xF7
};
IPAddress ip(192, 168, 0, 77);
//IPAddress ip(10, 5, 15, 109);

// Enter the IP address of the server you're connecting to:
//IPAddress server(192, 168, 100, 2);
IPAddress server(10, 21, 70, 16);
//IPAddress server(44, 195, 202, 69);
IPAddress myDns(192, 168, 0, 1);
uint16_t port = 5000;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 23 is default for telnet;
// if you're using Processing's ChatServer, use port 10002):
#define max_transfer 1024
#define max_buffer  (100 * max_transfer)
byte img_buf[max_buffer];
EthernetClient client;

//[ MQTT
IPAddress mqtt_server(10, 21, 70, 16);
EthernetClient mqttClient;
PubSubClient mqtt_client(mqttClient);
bool capture_requested = 0;
//]

//===================================================================================================
// HTTP POST 

int count = 0;
char c = 0;

void client_write_large(byte *bptr, size_t len) {
  size_t sent = 0;
  for (; sent + max_transfer < len; ) {
    client.write(bptr, max_transfer);
    sent += max_transfer;
    bptr += max_transfer;
    //PRINTLN(max_transfer);
  }
  client.write(bptr, len - sent);

  PRINT("Sent: "); PRINTLN(len);
}

void httpPostForm(byte *imageData, uint32_t imageSize) {
  String textData = "OpenMVCam1"; // Replace with your text data

  // Prepare the POST request body
  String requestBody = "";
  // Append the text data
  requestBody += "--ArduinoBoundary_OpenMVCam1\r\n";
  requestBody += "Content-Disposition: form-data; name=\"text\"\r\n\r\n";
  requestBody += textData;
  // Append the image data
  requestBody += "\r\n--ArduinoBoundary_OpenMVCam1\r\n";
  requestBody += "Content-Disposition: form-data; name=\"image\"; filename=\"image.jpg\"\r\n\r\n";

  //requestBody.append(imageData, imageSize);

  // Append the closing boundary
  String requestBodyEnd = "";
  requestBodyEnd += "\r\n--ArduinoBoundary_OpenMVCam1--\r\n";

  // Prepare the POST request headers
  String requestHeaders = "POST /upload HTTP/1.1\r\n";
  requestHeaders += "Host: 10.21.70.16:5000\r\n";
  requestHeaders += "Content-Type: multipart/form-data; boundary=ArduinoBoundary_OpenMVCam1\r\n";
  requestHeaders += "Connection: close\r\n";
  requestHeaders += "Content-Length: " + String(requestBody.length()+imageSize+requestBodyEnd.length()) + "\r\n\r\n";

  // Send the POST request headers
  client.print(requestHeaders);

  // Send the POST request body
  client.print(requestBody);
  client_write_large(imageData, imageSize);
  client.print(requestBodyEnd);
  client.flush();

  delay(100);
}

void http_postData(byte *buf, uint32_t length) {
  count = 0;

  // if the server's disconnected, reconnect the client:
  while (!client.connected()) {
    PRINTLN();
    PRINTLN("disconnected. Reconnecting...");
    if (client.connect(server, port)) {
      PRINTLN("connected");
      break;
    } else {
      // if you didn't get a connection to the server:
      if (++count > 5) {  // Retry 5 times.
        PRINTLN("HTTP Post failed. Give up.");
        return;
      }      
      PRINTLN("connection failed");
      delay(1000);
    }
  }
  delay(200);
  
  httpPostForm(buf, length);

  int len = client.available();

  String msg = "HTTP Response: ";
  msg += len;
  //mqtt_client.publish("W5300-MQTT", msg.c_str());

  if (len > 0) {
    byte buffer[500];
    if (len > 500) len = 500;
    int recvlen = 0;
        
    recvlen = client.read(buffer+recvlen, len);
    if (recvlen < len) {
      delay(10);
      recvlen += client.read(buffer+recvlen, len-recvlen);
    }

    String msg = "HTTP Received: ";
    msg += recvlen;
    //mqtt_client.publish("W5300-MQTT", msg.c_str());

    //Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    //PRINTLN("");
    //byteCount = byteCount + len;

    byte prev_char = 0;
    String response = "";
    int index = 0;
    for (index=0; index<len; ++index) {
      if (buffer[index] == '\n') {
        if (prev_char == '\n')
          break;
      } else if (buffer[index] == '\r')
        continue;
      prev_char = buffer[index];
    }

    for (; index<len; ++index)
      response += (char)buffer[index];
    response.trim();

    PRINTLN("Contents: " + response);
    mqtt_client.publish("W5300-MQTT", response.c_str());
  }

  client.stop();
}

//===================================================================================================
// MQTT [[ 

void callback(char* topic, byte* payload, unsigned int length) {
  PRINT(">>>>>>>>>>> Message arrived [");
  PRINT(topic);
  PRINT("] ");
  String cmd = "";
  for (int i=0;i<length;i++) {
    cmd += (char)payload[i];
    PRINT((char)payload[i]);
  }
  PRINTLN();

  if (cmd == "cmd:capture") {
    capture_requested = 1;
    PRINTLN("Capture requested!!!");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    PRINT("MQTT: Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(device_unique_id)) {
      PRINTLN("MQTT: connected");
      mqtt_client.publish("W5300-MQTT", "Ready");
      // ... and resubscribe
      mqtt_client.subscribe("MQTT-W5300");
    } else {
      PRINT("MQTT: failed, rc=");
      PRINT(mqtt_client.state());
      PRINTLN(" MQTT: try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}
// ]]

//===================================================================================================

void setup() {
    // Open serial communications and wait for port to open:
  Serial3.setRx(PC11);
  Serial3.setTx(PC10);  
  delay(50);
  
  // Open serial communications and wait for port to open:
#ifdef SERIAL_OUTPUT
  Serial.begin(9600);
#else
  Serial.setRx(0);
  Serial.setTx(1);  
  //Serial.begin(1000000);
  Serial.begin(500000);
  //Serial.begin(38400);
  //Serial.begin(19200);
  delay(50);
#endif

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
   // start the Ethernet connection:
  PRINTLN("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    PRINTLN("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      PRINTLN("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      PRINTLN("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    PRINT("  DHCP assigned IP ");
    PRINTLN(Ethernet.localIP());
  }
  
  // give the Ethernet shield a second to initialize:
  delay(1000);

  // MQTT [[ 
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  // ]]

  // give the Ethernet shield a second to initialize:
  delay(500);

}

unsigned long prevmillis = 0;

void loop() {

  //*// MQTT [[ 
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();
  // ]]
  //*/

  //if (millis() - prevmillis > 5000)
  if (capture_requested) {
    capture_requested = 0;

    // Flush serial buffer.
    while (Serial.available())
      Serial.read();

    // Send SYNC code
    Serial.write(SYNC);

    uint32_t length = serial_read_length();
    if (length > 0) {
      //String response = "Length: ";
      //response += length;
      //mqtt_client.publish("W5300-MQTT", response.c_str());
      
      uint32_t received = serial_read_data(img_buf, length);
      if (received != length) {
        // Time-out error!!!!
        String response = "Time-out: ";  // + received;
        response += received;
        mqtt_client.publish("W5300-MQTT", response.c_str());
      } else {
        // Send it to the server
        String response = "Image received: ";  // + length;
        //mqtt_client.publish("W5300-MQTT", response.c_str());
        http_postData(img_buf, length);
      }
    } else {
        mqtt_client.publish("W5300-MQTT", "No data");
    }
  }
}

uint32_t serial_read_length() {
  uint32_t length = 0;
  byte recv[4];
  int index = 0;

  prevmillis = millis();
  //Loop with 1sec timeout.
  while (millis()-prevmillis < 1000) {
    if (Serial.available()) {
      // Read the most recent byte
      recv[index++] = Serial.read();

      if (index >= 4) {
        // Big endian
        length = (recv[0] << 24) | (recv[1] << 16) | (recv[2] << 8) | recv[3];
        break;
      }
    }
  }

  return length;
}

uint32_t serial_read_data(byte *buf, uint32_t length) {
  int index = 0;

  uint32_t maxsize = 1024;
  uint32_t recvlen = 0;
  uint32_t remain = length;

  prevmillis = millis();
  while (remain > 0 && (millis()-prevmillis < 2000)) {
    if (remain > maxsize) {
      recvlen = serial_read_data0(buf, maxsize);
    } else {
      recvlen = serial_read_data0(buf, remain);
    }
    if (recvlen == -1) {
      return length - remain;
    }
    buf += recvlen;
    remain -= recvlen;

    /*
    String response = "RecvLen: ";
    response += recvlen;
    mqtt_client.publish("W5300-MQTT", response.c_str());
    */
  }

  return length - remain;
}

uint32_t serial_read_data0(byte *buf, uint32_t length) {
  int index = 0;

  prevmillis = millis();
  //Loop with 2sec timeout.
  while (millis()-prevmillis < 1000) {
    if (Serial.available()) {
      // Read the most recent byte
      buf[index++] = Serial.read();

      if (index >= length)
        return index;
    }
  }

  return -1;
}
