#!/usr/bin/env python
# coding: Latin-1

from modboti2c import Modboti2c
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
    # pygame.display.init()
    # pygame.display.set_mode((1, 1))
    while True:
        try:
            try:
                pygame.joystick.init()
                # Attempt to setup the joystick
                if pygame.joystick.get_count() < 1:
                    # No joystick attached, toggle the LED
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
            sys.exit()
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


arduino_1 = Modboti2c(address=0x45)


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


ps4_init()

while True:
    # EVENT PROCESSING STEP
    time.sleep(.2)
    for event in pygame.event.get():  # User did something
        # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
        if event.type == pygame.JOYBUTTONDOWN:
            print("Joystick button pressed.")
        elif event.type == pygame.JOYAXISMOTION:
            x_axis = -joystick.get_axis(axisLeftRight)
            y_axis = -joystick.get_axis(axisUpDown)
            (left, right) = steering(y_axis, x_axis)

            if left != last_left and abs(left - last_left) > 10:
                arduino_1.set_motor(1, left)
                last_left = left
                # print left

            if right != last_right and abs(right - last_right) > 10:
                arduino_1.set_motor(2, right)
                last_right = right
                # print right

            #print("{} | {}".format(left, right))
            #
            #
