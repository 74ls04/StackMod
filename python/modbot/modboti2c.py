
from __future__ import division
import logging
import time
import math

I2C_MASTER_ADDRESS  = 0x70
START               = 0x7B
END                 = 0x7D
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
        self._device.writeList(MOTOR, [START, self.address, "$", 'M', 'T', 'R', motor, value, '}'])


arduino_1 = Modboti2c(address=0x65)
arduino_1.set_motor(66, 100)
