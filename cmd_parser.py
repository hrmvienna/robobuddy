import commands

DEFAULT_CMDFILE = "./commands.txt"

def parse_cmd(cmd):
    return commands.Command(cmd)

def parse_cmds(filename = DEFAULT_CMDFILE):
    cmd_list = []
    if (filename == None):
        filename = DEFAULT_CMDFILE

    # read cmd list from file
    with open(filename) as f:
        cmds = f.readlines()
        f.close()
        # parse cmds
        for cmd in cmds:
            if cmd.startswith('#'):
                continue
            if not cmd.rstrip():
                # empty string
                continue
            cmd_list.append(parse_cmd(cmd))

    return cmd_list
