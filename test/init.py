import os

def start_xephyr():
    os.system("unset XDG_SEAT")
    os.system("Xephyr -br -ac +extension COMPOSITE -noreset -screen 1280x720 :1")

def start_cubewm():
    pass