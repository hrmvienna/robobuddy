# commands
CMD_CHECK_STATUS = 'c'
CMD_FIND_LINE    = 'a'
CMD_FIND_STATION = 's'
CMD_CAL_BLACK    = 'q'
CMD_CAL_WHITE    = 'w'
CMD_CAL          = 't'
CMD_TURN_LEFT    = 'f'
CMD_TURN_RIGHT   = 'g'
CMD_NEXT_STATION = 'h'
CMD_TURN_AROUND  = 'j'
CMD_READ_SENSOR_L  = 'o'
CMD_READ_SENSOR_R  = 'p'
CMD_READ_ROAD    = 'i'
CMD_NOP          = 'n'
CMD_SPEED_0      = '0'
CMD_SPEED_1      = '1'
CMD_SPEED_2      = '2'

# command strings
CMD_CHECK_STATUS_STR = "CHECK_STATUS"
CMD_FIND_LINE_STR    = "FIND_LINE"
CMD_FIND_STATION_STR = "FIND_STATION"
CMD_CAL_BLACK_STR    = "CALIBRATE_WHITE"
CMD_CAL_WHITE_STR    = "CALIBRATE_BLACK"
CMD_CAL_STR          = "CALIBRATE"
CMD_TURN_LEFT_STR    = "TURN_LEFT"
CMD_TURN_RIGHT_STR   = "TURN_RIGHT"
CMD_NEXT_STATION_STR = "NEXT_STATION"
CMD_TURN_AROUND_STR  = "TURN_AROUND"
CMD_READ_SENSOR_L_STR  = "READ_SENSOR_L"
CMD_READ_SENSOR_R_STR  = "READ_SENSOR_R"
CMD_READ_ROAD_STR    = "READ_ROAD"
CMD_SPEED_0_STR      = "SPEED_0"
CMD_SPEED_1_STR      = "SPEED_1"
CMD_SPEED_2_STR      = "SPEED_2"
CMD_NOP_STR          = "NOP"

CMD_VALID_CMDS = {  CMD_CHECK_STATUS_STR : CMD_CHECK_STATUS,
                    CMD_FIND_LINE_STR    : CMD_FIND_LINE,
                    CMD_FIND_STATION_STR : CMD_FIND_STATION,
                    CMD_CAL_BLACK_STR    : CMD_CAL_BLACK,
                    CMD_CAL_WHITE_STR    : CMD_CAL_WHITE,
                    CMD_CAL_STR          : CMD_CAL,
                    CMD_TURN_LEFT_STR    : CMD_TURN_LEFT,
                    CMD_TURN_RIGHT_STR   : CMD_TURN_RIGHT,
                    CMD_NEXT_STATION_STR : CMD_NEXT_STATION,
                    CMD_TURN_AROUND_STR  : CMD_TURN_AROUND,
                    CMD_READ_SENSOR_L_STR  : CMD_READ_SENSOR_L,
                    CMD_READ_SENSOR_R_STR  : CMD_READ_SENSOR_R,
                    CMD_READ_ROAD_STR    : CMD_READ_ROAD,
                    CMD_SPEED_0_STR      : CMD_SPEED_0,
                    CMD_SPEED_1_STR      : CMD_SPEED_1,
                    CMD_SPEED_2_STR      : CMD_SPEED_2,
                    CMD_NOP_STR          : CMD_NOP}

class Command:
    def __init__(self, cmd_str):
        #remove all whitespace characters, string to uppercase and replace multiple spaces with one
        cmd = cmd_str.rstrip().upper()
        if Command.isValidIf(cmd):
            split_cmd = cmd.split()
            self.cmd_str = cmd
            self.isif = True
            self.cmd = CMD_VALID_CMDS.get(split_cmd[1])
            self.then = split_cmd[3]
            self.otherwise = split_cmd[5]
        elif Command.isValid(cmd):
            self.cmd_str = cmd
            self.isif = False
            self.cmd = CMD_VALID_CMDS.get(cmd)
        else:
            raise ValueError("Invalid command: '" + cmd + "'")
    def __repr__(self):
        return "Command(%s)" % (self.cmd_str)
    def __str__(self):
        return self.cmd_str
    def getCmd(self):
        return self.cmd
    def isIf(self):
        return self.isif
    def getThen(self):
        if self.isif: 
            return self.then
        else:
            return None
    def getElse(self):
        if self.isif: 
            return self.otherwise
        else:
            return None
    @staticmethod
    def isValid(cmd_str):
        if cmd_str in CMD_VALID_CMDS.keys():
            return True
        else:
            return False
    @staticmethod
    def isValidIf(cmd_str):
        # syntax: IF cmd THEN cmd ELSE cmd
        split_cmd = cmd_str.split()
        if len(split_cmd) != 6:
            return False;
        if split_cmd[0] != "IF":
            return False;
        if split_cmd[2] != "THEN":
            return False;
        if split_cmd[4] != "ELSE":
            return False;
        if split_cmd[1] not in CMD_VALID_CMDS.keys():
            return False;
        if split_cmd[3] not in CMD_VALID_CMDS.keys():
            return False;
        if split_cmd[5] not in CMD_VALID_CMDS.keys():
            return False;
        else:
            return True;
