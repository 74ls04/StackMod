#!/usr/bin/env python

import os
import RPi.GPIO as GPIO
import time
import pygame
import math
from StackModIO import StackModIO
import signal
import sys

os.environ["SDL_VIDEODRIVER"] = "dummy"
#from tfmini import TFmini
# port.flushInput()
# port.flushOutput()
#lidar = TFmini('/dev/ttyAMA0')

global joystick

modbot = StackModIO(address=0x45)
# PS4 Controller variables
axisR2 = 4                          # Joystick axis to read for up / down position
axisL2 = 6
axisLeftRight = 2                        # Left joystick
axisUpDown = 5                       # Right joystick

last_left = 0
last_right = 0

average_size = 10
left_counter = 0
left_total = 0
left_average = 0
left_vals = [0] * average_size
right_counter = 0
right_total = 0
right_average = 0
right_vals = [0] * average_size


def translate(value, leftMin, leftMax, rightMin, rightMax):
    # Figure out how 'wide' each range is
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin
    valueScaled = float(value - leftMin) / float(leftSpan) # Convert the left range into a 0-1 range (float)
    return rightMin + (valueScaled * rightSpan) # Convert the 0-1 range into a value in the right range.


def ps4_init():
    global joystick
    print 'Waiting for joystick... (press CTRL+C to abort)'
    pygame.init()

    while True:
        try:
            try:
                pygame.joystick.init()
                if pygame.joystick.get_count() < 1: # Attempt to setup the joystick
                    pygame.joystick.quit()
                    time.sleep(0.1)
                else:
                    joystick = pygame.joystick.Joystick(0)  # We have a joystick, attempt to initialise it!
                    break
            except pygame.error:
                print 'Failed to connect to the joystick' # Failed to connect to the joystick
                pygame.joystick.quit()
                time.sleep(0.1)
        except KeyboardInterrupt:
            # CTRL+C exit, give up
            print '\nUser aborted'
    print 'Joystick found'
    joystick.init()


def differential_steering(x, y):
    # convert to polar
    r = math.hypot(x, y)
    t = math.atan2(y, x)

    t += math.pi / 4  # rotate by 45 degrees

    # back to cartesian
    left = r * math.cos(t)
    right = r * math.sin(t)

    # rescale the new coords
    left = left * math.sqrt(2)
    right = right * math.sqrt(2)

    # clamp to -1/+1
    left = max(-1, min(left, 1))
    right = max(-1, min(right, 1))

    return left, right


def exponential_filter(val, slope):
    # https://www.chiefdelphi.com/forums/showthread.php?t=88065
    offset = .23
    if val > 0:
        return offset + (1 - offset) * (slope * math.pow(val, 3) + (1 - slope) * val)
    elif val == 0:
        return 0
    else:
        return -offset + (1 - offset) * (slope * math.pow(val, 3) + (1 - slope) * val)


def signal_handler(signal, frame):
    print('Exiting Modbot')
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

ps4_init()

while True:
    for event in pygame.event.get():  # User did something
        # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
        if event.type == pygame.JOYBUTTONDOWN:
            print("Joystick button pressed.")
        elif event.type == pygame.JOYAXISMOTION:
            x_axis = exponential_filter(-joystick.get_axis(axisLeftRight), 1)
            y_axis = exponential_filter(-joystick.get_axis(axisUpDown), 1)

            (left, right) = differential_steering(y_axis, x_axis)
            # Scale to -255 - 255 range
            left = int(translate(left, -1, 1, -255, 255))
            right = int(translate(right, -1, 1, -255, 255))
            # Left
            left_total = left_total - left_vals[left_counter]
            left_vals[left_counter] = left
            left_total = left_total + left_vals[left_counter]
            left_counter += 1

            if left_counter >= average_size:
                left_counter = 0

            left_average = left_total / average_size

            # Right
            right_total = right_total - right_vals[right_counter]
            right_vals[right_counter] = right
            right_total = right_total + right_vals[right_counter]
            right_counter += 1

            if right_counter >= average_size:
                right_counter = 0

            right_average = right_total / average_size

            if abs(left_average - last_left) > 10 or left == 0:
                #if left_average != last_left:
                if left == 0 and last_left != left:
                    # print("Left: %d" % left)
                    last_left = left
                    modbot.set_motor(1, left)

                if left != 0 and last_left != left_average:
                    # print("Left: %d" % left_average)
                    last_left = left_average
                    modbot.set_motor(1, left_average)

                # modbot.set_motor(1, left)
                # print("Left: %d %d" % (left, left_average))

            if abs(right_average - last_right) > 10 or right == 0:
                #if right_average != last_right:
                if right == 0 and last_right != right:
                    # print("Right: %d" % right)
                    last_right = right
                    modbot.set_motor(2, right)

                if right != 0 and last_right != right_average:
                    # print("Right: %d" % right_average)
                    last_right = right_average
                    modbot.set_motor(2, right_average)
            #print("{} | {}".format(left, right))
    # (d, s, q) = lidar.read()
    # print('Distance: {:5}'.format(d))
    # print d
    # time.sleep(.5)
