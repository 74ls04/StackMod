#!/usr/bin/env python
# coding: Latin-1

from StackModIO import StackModIO
import pygame
import math
import os
import time
global joystick

os.environ["SDL_VIDEODRIVER"] = "dummy"

axisR2 = 4                          # Joystick axis to read for up / down position
axisL2 = 6
axisLeftRight = 2                        # Left joystick
axisUpDown = 5                       # Right joystick

last_left = 0
last_right = 0


def ps4_init():
    global joystick
    print 'Waiting for joystick... (press CTRL+C to abort)'
    pygame.init()

    while True:
        try:
            try:
                pygame.joystick.init()
                # Attempt to setup the joystick
                if pygame.joystick.get_count() < 1:
                    pygame.joystick.quit()
                    time.sleep(0.1)
                else:
                    # We have a joystick, attempt to initialise it!
                    joystick = pygame.joystick.Joystick(0)
                    break
            except pygame.error:
                # Failed to connect to the joystick, toggle the LED
                print 'Failed to connect to the joystick'
                pygame.joystick.quit()
                time.sleep(0.1)
        except KeyboardInterrupt:
            # CTRL+C exit, give up
            print '\nUser aborted'
    print 'Joystick found'
    joystick.init()


def translate(value, leftMin, leftMax, rightMin, rightMax):
    # Figure out how 'wide' each range is
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin

    # Convert the left range into a 0-1 range (float)
    valueScaled = float(value - leftMin) / float(leftSpan)

    # Convert the 0-1 range into a value in the right range.
    return rightMin + (valueScaled * rightSpan)


arduino_1 = StackModIO(address=0x45)


def steering(x, y):
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

    left = translate(left, -1, 1, -255, 255)
    right = translate(right, -1, 1, -255, 255)
    return int(left), int(right)


def exponential_filter(val, slope):
    # https://www.chiefdelphi.com/forums/showthread.php?t=88065
    offset = .23
    if val > 0:
        return offset + (1 - offset) * (slope * math.pow(val, 3) + (1 - slope) * val)
    elif val == 0:
        return 0
    else:
        return -offset + (1 - offset) * (slope * math.pow(val, 3) + (1 - slope) * val)

# ps4_init()

# test_speed = .5

while True:
    test_speed = float(raw_input('Val: '))
    test_speed = exponential_filter(test_speed, 1)
    arduino_1.set_motor(1,  int(translate(test_speed, -1, 1, -255, 255)))
    arduino_1.set_motor(2,  int(translate(test_speed, -1, 1, -255, 255)))
    print("{} | {}".format(test_speed, int(translate(test_speed, -1, 1, -255, 255))))
    # time.sleep(1)

#
# while True:
#     # EVENT PROCESSING STEP
#     for event in pygame.event.get():  # User did something
#         # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
#         if event.type == pygame.JOYBUTTONDOWN:
#             print("Joystick button pressed.")
#         elif event.type == pygame.JOYAXISMOTION:
#             x_axis = -joystick.get_axis(axisLeftRight)
#             y_axis = -joystick.get_axis(axisUpDown)
#             (left, right) = steering(y_axis, x_axis)
#
#             if left != last_left and abs(left - last_left) > 10:
#                 arduino_1.set_motor(1, left)
#                 last_left = left
#                 # print left
#
#             if right != last_right and abs(right - last_right) > 10:
#                 arduino_1.set_motor(2, right)
#                 last_right = right
#                 # print right
#
#             #print("{} | {}".format(left, right))
#             #
#             #
