
from __future__ import division
import logging
import time
import math
import sys

I2C_MASTER_ADDRESS  = 0x70
START               = '{'
END                 = '}'
MOTOR               = 0x32

class StackModIO(object):
    """Modbot Protocol Python Implementation"""

    def __init__(self, address, i2c=None, **kwargs):
        # Setup I2C interface for the device.
        if i2c is None:
            import Adafruit_GPIO.I2C as I2C
            i2c = I2C
        self._device = i2c.get_i2c_device(address, **kwargs)
        self.address = address

    @staticmethod
    def calculate_checksum(packet):
        checksum = 0
        for c in packet:
            if (c != START) and (c != END):
                try:
                    checksum += c - 32
                except TypeError:
                    checksum += ord(c) - 32
        return (checksum % 95) + 32

    def set_motor(self, motor, value):
        packet = [START, self.address, I2C_MASTER_ADDRESS, '$', 'M', 'T', 'R', str(motor)]
        val = list(str(abs(value)).zfill(3))
        if value < 0:
            packet.append('-')
        else:
            packet.append('+')

        packet = packet + val
        packet.append(END)
        packet.append(self.calculate_checksum(packet))
        # print packet
        try:
            self._device.writeList(MOTOR, packet)
        except:
            e = sys.exc_info()[0]
            print("Send Error: %s" % e)

