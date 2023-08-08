# Basic UART communications between OpenMV and Arduino Uno.

# 1) Wire up your OpenMV Cam to your Arduino Uno like this:
#
# OpenMV Cam Ground Pin   ----> Arduino Ground
# OpenMV Cam UART3_TX(P4) ----> Arduino Uno UART_RX(0)
# OpenMV Cam UART3_RX(P5) ----> Arduino Uno UART_TX(1)

# 2) Uncomment and upload the following sketch to Arduino:
#
# void setup() {
#   // put your setup code here, to run once:
#   Serial.begin(19200);
# }
#
# void loop() {
#   // put your main code here, to run repeatedly:
#   if (Serial.available()) {
#     // Read the most recent byte
#     byte byteRead = Serial.read();
#     // ECHO the value that was read
#     Serial.write(byteRead);
#   }
# }

# 3) Run the following script in OpenMV IDE:

import sensor, image, time, struct
from pyb import UART

sensor.reset()
sensor.set_pixformat(sensor.RGB565)
sensor.set_framesize(sensor.QVGA)
sensor.skip_frames(time = 2000)

# UART 3, and baudrate.
#uart = UART(3, 1000000)
uart = UART(3, 500000)
#uart = UART(3, 38400)

while(True):

    if uart.any():

        ret = uart.read(1)
        print(ret)
        if ret[0] == 0x96:  # SYNC

            # Capture camera
            img = sensor.snapshot()
            img.to_jpeg()
            length = img.size()
            lenstr = struct.pack('>I', length)
            imgbuf = img.bytearray()

            print(length)
            print(lenstr)

            # Send image size in 4 bytes
            uart.write(lenstr)
            time.sleep_ms(100)

            # Send the image
            start = 0
            maxsize = 1024
            while length > 0:
                if length <= maxsize:
                    sent = uart.write(imgbuf[start:start+length])
                    #print("Len: %d, Sent: %d" % (length, sent))
                    start += sent
                    length -= sent
                else:
                    sent = uart.write(imgbuf[start:start+maxsize])
                    #print("Len: %d, Sent: %d" % (length, sent))
                    start += sent
                    length -= sent
                #time.sleep_ms(1)

    #time.sleep_ms(1000)
