import sys, argparse, time
from multiprocessing.pool import ThreadPool
import config, cmd_parser
import i2ctools as I2C

# config
cmd_delay = 0.2

# Thread pool
pool = None

# Debugging stuff
start_time = time.time()
cmd_cnt = 0
cmd_cnt_cor = 0

# TODO
def command_handler(i2ctool, command):
    print "Command handler:", command
    if command.getCmd() == 'n': #NOP
        return config.I2C_OK;

    i2ctool.write(command.getCmd())
    i2ctool.waitCmdExec()
    ret = i2ctool.read()

    # TODO loop as long as busy
    while ret == config.I2C_BUSY:
        i2ctool.waitCmdExec()
        ret = i2ctool.read()

    return ret
#TODO
def execute_command(pool, i2ctool, command):
    cmd_result = pool.apply_async(command_handler, (i2ctool, command))
    res = cmd_result.get()
    msg = "val: " + str(res) + " hex: " + str(hex(res))
    i2ctool.evalResult(msg, res)

    global cmd_cnt_cor, cmd_cnt
    cmd_cnt += 1
    if res != config.I2C_ERR:
        cmd_cnt_cor += 1

    time.sleep(cmd_delay)
    
    return res
# TODO
def main():
    # parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-c', '--cmds', help='commandlist filename')
    parser.add_argument('-d', '--delay', help='delay between commands [s]')
    parser.add_argument("-l", help='start in live modus', action="store_true", default=False)
    args = parser.parse_args()

    if args.l == False:
        global cmd_delay
        try:
            commandlist = cmd_parser.parse_cmds(args.cmds)
            print commandlist, cmd_delay
        except ValueError as e:
            print e
        except IOError as e:
            print "I/O error({0}): {1}".format(e.errno, e.strerror) + ":", args.cmds

        if args.delay != None:
            cmd_delay = args.delay

        try:
            i2ctool = I2C.I2CTool()
            pool = ThreadPool(processes=1)

            for command in commandlist:
                if command.isif:
                    res = execute_command(pool, i2ctool, command)
                    if res == config.I2C_TRUE:
                        execute_command(pool, i2ctool, cmd_parser.parse_cmd(command.getThen()))
                    elif res == config.I2C_FALSE:
                        execute_command(pool, i2ctool, cmd_parser.parse_cmd(command.getElse()))
                else:
                    res = execute_command(pool, i2ctool, command)

                if res == config.I2C_ERR:
                    print "ERROR: command", command
                    print "exit programm"
                    break;
        finally:
            exit_program()

    else:
        print "To exit enter 'quit'"
        i2ctool = I2C.I2CTool()
        pool = ThreadPool(processes=1)

        while True:
            try:
                print "Enter command: ",
                line = sys.stdin.readline()
                if line.startswith("quit"):
                    exit_program()
                command = cmd_parser.parse_cmd(line)
                if command.isif:
                    res = execute_command(pool, i2ctool, command)
                    if res == config.I2C_TRUE:
                        execute_command(pool, i2ctool, cmd_parser.parse_cmd(command.getThen()))
                    elif res == config.I2C_FALSE:
                        execute_command(pool, i2ctool, cmd_parser.parse_cmd(command.getElse()))
                else:
                    execute_command(pool, i2ctool, command)
            except ValueError as e:
                print e

# Helper functions
# TODO
def exit_program(code=0):
    #TODO: kill threads
    if pool != None:
        pool.close()
        pool.terminate()

    global cmd_cnt_cor, cmd_cnt, start_time

    print "===================="
    print "cor. exec. commands:", str(cmd_cnt_cor) + "/" + str(cmd_cnt)
    print "runtime:", ("%.3f" % (time.time() - start_time)), "s"
    sys.exit(code)

# TODO
if __name__ == "__main__":
    main()
