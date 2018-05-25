import pygame
import os
import time
os.environ["SDL_VIDEODRIVER"] = "dummy"

pygame.init()
j = pygame.joystick.Joystick(0)
j.init()

d = {}

try:
    while True:
        # time.sleep(.05)
        events = pygame.event.get()
        for event in events:
            if event.type == pygame.JOYAXISMOTION:
                if event.axis in d:
                    if d[event.axis] != round(event.value, 1):
                        # print round(j.get_axis(11), 1)
                        if event.axis in [6, 7, 8, 11]:
                            continue
                        else:
                            print(event.axis, round(event.value, 1))
                d[event.axis] = round(event.value, 1)
                # print(event.dict, event.joy, event.axis, event.value)
            elif event.type == pygame.JOYBALLMOTION:
                print(event.dict, event.joy, event.ball, event.rel)
            # elif event.type == pygame.JOYBUTTONDOWN:
            #     print(event.dict, event.joy, event.button, 'pressed')
            # elif event.type == pygame.JOYBUTTONUP:
            #     print(event.dict, event.joy, event.button, 'released')
            # elif event.type == pygame.JOYHATMOTION:
                # print(event.dict, event.joy, event.hat, event.value)
                # print(event.value)

except KeyboardInterrupt:
    print("EXITING NOW")
    j.quit()