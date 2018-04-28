from serial import Serial
import binascii

class TFmini(object):
    """
    TFMini - Micro LiDAR Module
    https://www.sparkfun.com/products/14577
    http://www.benewake.com/en/tfmini.html
    """
    NOHEADER = 1
    BADCHECKSUM = 2
    TOO_MANY_TRIES = 3

    def __init__(self, port, retry=25):
        self.serial = Serial()
        self.serial.port = port
        self.serial.baudrate = 115200
        self.serial.timeout = 0.005
        self.serial.open()
        self.retry = retry  # how many times will I retry to find the packet header

        if not self.serial.is_open:
            raise Exception("ERROR: couldn't open port: {}".format(port))

            # do I need this?
        # std_mode = [0x42, 0x57, 0x02, 0x00, 0x00, 0x00, 0x01, 0x06]
        # data = bytearray(std_mode)
        # data = bytes(data)
        # self.serial.write(data)

    def __del__(self):
        self.close()

    def close(self):
        self.serial.close()

    def read(self):
        # Read data stream
        self.serial.flushInput()

        # find header
        a, b = 0, 0
        count = self.retry

        # Find header of [0x59 0x59]
        while True:
            a = b
            b = self.serial.read(1)

            if a == 'Y' and b == 'Y':
                break
            if count == 0:
                return (None, None, self.TOO_MANY_TRIES)
            count -= 1

        raw = self.serial.read(7)
        # print raw.encode('hex')
        pkt = [ord(a), ord(b)] + list(map(ord, raw))
        # print pkt
        try:
            ret = self.process_pkt(pkt)
        except Exception as e:
            print(e)
            ret = (None, None, None)
        return ret

    def process_pkt(self, pkt):
        """
        packet = [0x59, 0x59, distL, distH, strL, strH, reserved, quality, checksum]
        """
        # turn string data into array of bytes
        # pkt = list(map(ord, pkt))
        if len(pkt) != 9:
            raise Exception("ERROR: packet size {} != 9".format(len(pkt)))

        # check header
        if pkt[0] != 0x59 or pkt[1] != 0x59:
            raise Exception("ERROR: bad header in packet")

        # calculate checksum
        # cs = 0
        # for b in pkt[:8]:
        #     cs += b
        cs = sum(pkt[:8])
        cs &= 0xff

        if pkt[8] != cs:
            raise Exception("ERROR: bad checksum in packet")

        dist = pkt[2] + (pkt[3] << 8)
        st = pkt[4] + (pkt[5] << 8)
        q = pkt[7]

        return dist, st, q
