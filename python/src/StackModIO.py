
from __future__ import division
import logging
import time
import math
import sys

log = logging.getLogger(__name__)

I2C_MASTER_ADDRESS  = 0x70
START               = '{'
END                 = '}'
MOTOR               = 0x32

exponential_deadband = .25
exponential_sensitivity = 1


class StackModIO(object):
    """Modbot Protocol Python Implementation"""
    smoothing_size = 10
    x_array = [0]*smoothing_size
    x_index = 0
    x_total = 0
    x_average = 0
    y_array = [0]*smoothing_size
    y_index = 0
    y_total = 0
    y_average = 0

    def __init__(self, address, i2c=None, **kwargs):
        # Setup I2C interface for the device.
        if i2c is None:
            import Adafruit_GPIO.I2C as I2C
            i2c = I2C
        self._device = i2c.get_i2c_device(address, **kwargs)
        self.address = address

    def joystick_to_diff(self, x, y, minJoystick, maxJoystick, minSpeed, maxSpeed):
        # If x and y are 0, then there is not much to calculate...
        if x == 0 and y == 0:
            return (0, 0)

        # First Compute the angle in deg
        # First hypotenuse
        z = math.sqrt(x * x + y * y)

        # angle in radians
        rad = math.acos(math.fabs(x) / z)

        # and in degrees
        angle = rad * 180 / math.pi

        # Now angle indicates the measure of turn
        # Along a straight line, with an angle o, the turn co-efficient is same
        # this applies for angles between 0-90, with angle 0 the coeff is -1
        # with angle 45, the co-efficient is 0 and with angle 90, it is 1

        tcoeff = -1 + (angle / 90) * 2
        turn = tcoeff * math.fabs(math.fabs(y) - math.fabs(x))
        turn = round(turn * 100, 0) / 100

        # And max of y or x is the movement
        mov = max(math.fabs(y), math.fabs(x))

        # First and third quadrant
        if (x >= 0 and y >= 0) or (x < 0 and y < 0):
            rawLeft = mov
            rawRight = turn
        else:
            rawRight = mov
            rawLeft = turn

        # Reverse polarity
        if y < 0:
            rawLeft = 0 - rawLeft
            rawRight = 0 - rawRight

        # minJoystick, maxJoystick, minSpeed, maxSpeed
        # Map the values onto the defined rang
        rightOut = self.map(rawRight, minJoystick, maxJoystick, minSpeed, maxSpeed)
        leftOut = self.map(rawLeft, minJoystick, maxJoystick, minSpeed, maxSpeed)

        return int(rightOut), int(leftOut)

    @staticmethod
    def map(v, in_min, in_max, out_min, out_max):
        # Check that the value is at least in_min
        if v < in_min:
            v = in_min
        # Check that the value is at most in_max
        if v > in_max:
            v = in_max
        return (v - in_min) * (out_max - out_min) // (in_max - in_min) + out_min


    @staticmethod
    def map_values(value, left_min, left_max, right_min, right_max):
        """
        Maps values from one range to another

        :param value:
        :param left_min:
        :param left_max:
        :param right_min:
        :param right_max:
        :return:
        """

        # Figure out how 'wide' each range is
        left_span = left_max - left_min
        right_span = right_max - right_min
        value_scaled = float(value - left_min) / float(left_span)  # Convert the left range into a 0-1 range (float)
        return right_min + (value_scaled * right_span)  # Convert the 0-1 range into a value in the right range.

    def mix_x_y(self, in_x, in_y):
        """
        Converts X,Y values to L,R differential stearing values


        :param in_x:
        :param in_y:
        :return:
        """
        x = in_x
        y = in_y
        # self.x_total = self.x_total - self.x_array[self.x_index]
        # self.x_array[self.x_index] = in_x
        # self.x_total = self.x_total + self.x_array[self.x_index]
        # self.x_index = self.x_index + 1
        #
        # if self.x_index >= self.smoothing_size:
        #     self.x_index = 0
        #
        # x = self.x_total / self.smoothing_size
        #
        # self.y_total = self.y_total - self.y_array[self.y_index]
        # self.y_array[self.y_index] = in_y
        # self.y_total = self.y_total + self.y_array[self.y_index]
        # self.y_index = self.y_index + 1
        #
        # if self.y_index >= self.smoothing_size:
        #     self.y_index = 0
        #
        # y = self.y_total / self.smoothing_size
        x = self.exponential_filter(.23, in_x, 1)
        y = self.exponential_filter(.23, in_y, 1)
        #
        # convert to polar
        r = math.hypot(x, y)
        t = math.atan2(y, x)

        # rotate by 45 degrees
        t += math.pi / 4

        # back to cartesian
        left = r * math.cos(t)
        right = r * math.sin(t)

        # rescale the new coords
        left = left * math.sqrt(2)
        right = right * math.sqrt(2)

        # clamp to -1/+1
        left = max(-1, min(left, 1))
        right = max(-1, min(right, 1))

        # if (left < 0 and right > 0) or (left > 0 and right < 0):
        # left = self.exponential_filter(.23, left, 1)
        # right = self.exponential_filter(.23, right, 1)
        
        # else:
        #     left = self.exponential_filter(exponential_deadband, left, exponential_sensitivity)
        #     right = self.exponential_filter(exponential_deadband, right, exponential_sensitivity)
        # x = -x
        # v = (1 - abs(x)) * (y/1) + y
        # w = (1 - abs(y)) * (x/1) + x
        # left = (v - w) / 2
        # right = (v + w) / 2
        return int(self.map_values(left, -1, 1, -255, 255)), int(self.map_values(right, -1, 1, -255, 255))

    @staticmethod
    def exponential_filter(deadband, value, sensitivity):
        """
        Exponential response for a joystick input in the range -1 - 1
        A sensitivity of 0 gives a linear response and a sensitivity of 1 gives a steep exponential curve
        The inverse deadband or slope is the minimum value at which in input causes the motor to move

        f(x)=.2+(1-.2)*(x)
        f(x)=-.2+(1-.2)*(x)

        f(x)=.2+(1-.2)*(x^3)
        f(x)=-.2+(1-.2)*(x^3)

        Credit to this thread: https://www.chiefdelphi.com/forums/showthread.php?t=88065

        :param deadband:
        :param value:
        :param sensitivity:
        :return:
        """
        if value > 0:
            return deadband + (1 - deadband) * (sensitivity * math.pow(value, 4) + (1 - sensitivity) * value)
        elif value == 0:
            return 0
        else:
            return -deadband + (1 - deadband) * (sensitivity * math.pow(value, 4) + (1 - sensitivity) * value)

    @staticmethod
    def exponential_moving_average(curr_sum, new_value):
        """
        https://stackoverflow.com/questions/10990618/calculate-rolling-moving-average-in-c#10990656
        :param curr_sum:
        :param new_value:
        :return:
        """
        # alpha = .3
        # (1 - .3) = .7
        return 0 if new_value == 0 else (.7 * new_value) + .7 * curr_sum

    @staticmethod
    def _calculate_checksum(packet):
        """

        :param packet:
        :return:
        """
        checksum = 0
        for c in packet:
            if (c != START) and (c != END):
                try:
                    checksum += c - 32
                except TypeError:
                    checksum += ord(c) - 32
        return (checksum % 95) + 32

    def set_motor(self, motor, value):
        """

        :param motor:
        :param value:
        :return:
        """
        packet = [START, self.address, I2C_MASTER_ADDRESS, '$', 'M', 'T', 'R', str(motor)]
        val = list(str(abs(value)).zfill(3))
        if value < 0:
            packet.append('-')
        else:
            packet.append('+')

        packet = packet + val
        packet.append(END)
        packet.append(self._calculate_checksum(packet))
        log.debug(packet)
        try:
            self._device.writeList(MOTOR, packet)
        except:
            e = sys.exc_info()[0]
            log.error("Send Error: %s" % e)

