
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
