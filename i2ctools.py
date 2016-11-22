import smbus, time, sys
import functools
import config

# TODO
msg_err = '\033[91m'
msg_suc = '\033[92m'
msg_msg = '\033[94m'
msg_end = '\033[0m'

# TODO
class I2CTool:
    def __init__(self, address=config.I2C_ADDRESS):
        self.address = address
        # for RPI version 1, use "bus = smbus.SMBus(0)"
        self.bus = smbus.SMBus(1)

    # TODO
    def write(self, data):
        self.bus.write_byte(self.address, ord(data))
        return

    # TODO 'c' check status
    def read(self, cmd='c'):
        data = self.bus.read_i2c_block_data(self.address, ord(cmd), config.I2C_DATA_LEN)
        def f(x, y): return (x<<8)|y
        return functools.reduce(f, data, 0)

    # TODO
    def waitCmdExec(self):
        time.sleep(config.I2C_RESPONSE_TIME)

    @staticmethod
    def evalResult(msg, i2c_ret_code):
        if i2c_ret_code == config.I2C_OK:
            sys.stdout.write(msg_suc)
        elif i2c_ret_code == config.I2C_ERR:
            sys.stdout.write(msg_err)
        else:
            sys.stdout.write(msg_msg)
        print "Result:", msg + msg_end;