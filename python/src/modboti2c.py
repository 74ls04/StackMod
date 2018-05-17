
from __future__ import division
import logging
import time
import math
import sys


I2C_MASTER_ADDRESS  = 0x70
START               = '{'
END                 = '}'
MOTOR               = 0x32

class Modboti2c(object):
    """Modbot Protocol Python Implementation"""

    def __init__(self, address, i2c=None, **kwargs):
        # Setup I2C interface for the device.
        if i2c is None:
            import Adafruit_GPIO.I2C as I2C
            i2c = I2C
        self._device = i2c.get_i2c_device(address, **kwargs)
        self.address = address

    def set_motor(self, motor, value):
        packet_start = [START, self.address, I2C_MASTER_ADDRESS, "$", 'M', 'T', 'R', str(motor)]
        val = list(str(abs(value)).zfill(3))
        if value < 0:
            packet_start.append('-')
        else:
            packet_start.append('+')

        packet_start = packet_start + val
        packet_start.append(END)
        print packet_start
        try:
            self._device.writeList(MOTOR, packet_start)
        except:
            e = sys.exc_info()[0]
            print("Send Error: %s" % e)

