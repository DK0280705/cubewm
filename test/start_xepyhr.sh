#!/usr/bin/bash

unset XDG_SEAT
Xephyr -br -ac +COMPOSITE +RANDR +xinerama -ac -br -screen 1280x720 :1