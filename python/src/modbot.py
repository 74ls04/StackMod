#!/usr/bin/env python

import os
import logging
import RPi.GPIO as GPIO
import time
import pygame
import math
from StackModIO import StackModIO
import signal
import sys

logging.basicConfig(level=os.environ.get("LOGLEVEL", "INFO"),
                    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s")

os.environ["SDL_VIDEODRIVER"] = "dummy"

#from tfmini import TFmini
# port.flushInput()
# port.flushOutput()
#lidar = TFmini('/dev/ttyAMA0')

controller = None

stackmodio = StackModIO(address=0x45)

# PS4 Controller variables
LEFT_ANALOG_X = 0
LEFT_ANALOG_Y = 1
RIGHT_ANALOG_X = 2
RIGHT_ANALOG_Y = 5
L2_ANALOG = 3
R2_ANALOG = 4

DPAD_UP = (0, 1)
DPAD_DOWN = (0, -1)
DPAD_LEFT = (-1, 0)
DPAD_RIGHT = (1, 0)
BUTTON_CROSS = 1
BUTTON_CIRCLE = 2
BUTTON_SQUARE = 0
BUTTON_TRIANGLE = 3
BUTTON_L1 = 4
BUTTON_L2 = 6
BUTTON_L3 = 10
BUTTON_R1 = 5
BUTTON_R2 = 7
BUTTON_R3 = 11
BUTTON_SHARE = 8
BUTTON_OPTIONS = 9
BUTTON_TRACKPAD = 13
BUTTON_PS = 12

MOTION_Y = 9
MOTION_X = 10
MOTION_Z = 11
ORIENTATION_ROLL = 6
ORIENTATION_YAW = 8
ORIENTATION_PITCH = 7


# PS4 Controller variables
# exponential_deadband = .23
# exponential_sensitivity = 1
x = 0
y = 0

left_motors_avg = 0
last_left_motors_pwm = 0
right_motors_avg = 0
last_right_motors_pwm = 0


def init():
    global controller
    pygame.init()

    try:
        pygame.joystick.init()
        if pygame.joystick.get_count() < 1:  # Attempt to setup the joystick
            logging.error("No controllers found")
            cleanup()
        else:
            controller = pygame.joystick.Joystick(0)  # We have a joystick, attempt to initialise it!
    except pygame.error:
        logging.error("Failed to connect to the joystick")  # Failed to connect to the joystick
        cleanup()

    logging.info("Joystick found")
    controller.init()


def joystick_drive(x, y):
    """

    :param x:
    :param y:
    :return:
    """
    global last_left_motors_pwm
    global last_right_motors_pwm
    global left_motors_avg
    global right_motors_avg

    # x_filtered = stackmodio.exponential_filter(exponential_deadband, x, exponential_sensitivity)
    # y_filtered = stackmodio.exponential_filter(exponential_deadband, y, exponential_sensitivity)

    # (left_motors_pwm, right_motors_pwm) = stackmodio.joystick_to_diff(x, y, -1, 1, -255, 255)

    (left_motors_pwm, right_motors_pwm) = stackmodio.mix_x_y(x, y)

    if (abs(left_motors_pwm - last_left_motors_pwm) > 2) or (abs(right_motors_pwm - last_right_motors_pwm) > 2) \
            or left_motors_pwm == 0 or right_motors_pwm == 0:

        logging.info("{} {}".format(left_motors_pwm, right_motors_pwm))

        if last_left_motors_pwm != left_motors_pwm:
            last_left_motors_pwm = left_motors_pwm
            stackmodio.set_motor(1, left_motors_pwm)

        if last_right_motors_pwm != right_motors_pwm:
            last_right_motors_pwm = right_motors_pwm
            stackmodio.set_motor(2, right_motors_pwm)
        # left_motors_avg = int(stackmodio.exponential_moving_average(left_motors_avg, left_motors_pwm))
        # right_motors_avg = int(stackmodio.exponential_moving_average(right_motors_avg, right_motors_pwm))


        # if abs(left_motors_pwm - last_left_motors_pwm) > 10 or left_motors_pwm == 0:
        #     if last_left_motors_pwm != left_motors_pwm:
                # logging.debug("Left: %d" % left_motors_pwm)
                # last_left_motors_pwm = left_motors_pwm
                # stackmodio.set_motor(1, left_motors_pwm)

        # if abs(right_motors_pwm - last_right_motors_pwm) > 10 or right_motors_pwm == 0:
        #     if last_right_motors_pwm != right_motors_pwm:
        #         logging.debug("Right: %d" % right_motors_pwm)
        #         last_right_motors_pwm = right_motors_pwm
        #         stackmodio.set_motor(2, right_motors_pwm)


def cleanup():
    pygame.joystick.quit()
    logging.error("Exiting")
    sys.exit(1)


def operate():
    global x
    global y

    while True:
        # time.sleep(.1)
        for event in pygame.event.get():
            if event.type == pygame.JOYBUTTONDOWN:
                # if event.button == BUTTON_L1:
                #     pass
                pass
            elif event.type == pygame.JOYBUTTONUP:
                # if event.button == BUTTON_L1:
                #     pass
                pass
            elif event.type == pygame.JOYAXISMOTION:
                # print event.axis
                if (event.axis == RIGHT_ANALOG_Y or event.axis == RIGHT_ANALOG_X) and event.value != 0:
                    raw_x_axis_value,  raw_y_axis_value = -controller.get_axis(RIGHT_ANALOG_X), \
                                                          controller.get_axis(RIGHT_ANALOG_Y)


                    # raw_y_axis_value = controller.get_axis(RIGHT_ANALOG_Y) if controller.get_axis(RIGHT_ANALOG_Y) == 0 \
                    #     else controller.get_axis(RIGHT_ANALOG_Y)

                    # .0039
                    # Apply 14% dead zone on analog sticks by calcuating the magnitude of the <x,y> vector
                    magnitude = math.sqrt(raw_x_axis_value ** 2 + raw_y_axis_value ** 2)
                    deadzone = 0.10
                    if magnitude < deadzone:
                        raw_x_axis_value = 0
                        raw_y_axis_value = 0
                    # else:
                    #     raw_x_axis_value = (raw_x_axis_value / magnitude) * ((magnitude - deadzone) / (1 - deadzone))
                    #     raw_y_axis_value = (raw_y_axis_value / magnitude) * ((magnitude - deadzone) / (1 - deadzone))

                    # raw_x_axis_value = round(stackmodio.exponential_filter(.23, raw_x_axis_value, 1), 6)
                    # raw_y_axis_value = round(stackmodio.exponential_filter(.23, raw_y_axis_value, 1), 6)

                    max_jitter = .019
                    # print("{} {}".format(raw_x_axis_value - x, raw_y_axis_value - y))
                    if abs(raw_x_axis_value - x) > max_jitter or raw_x_axis_value == 0:
                        x = raw_x_axis_value

                    if abs(raw_y_axis_value - y) > max_jitter or raw_y_axis_value == 0:
                        y = raw_y_axis_value

                    joystick_drive(x, y)

                    # print("{} {}".format(x, y))


def signal_handler(signal, frame):
    cleanup()


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    init()
    operate()
    # (d, s, q) = lidar.read()
    # print('Distance: {:5}'.format(d))
    # print d
    # time.sleep(.5)
