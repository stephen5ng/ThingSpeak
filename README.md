# ThingSpeak

platformio.ini should look something like this:

[env:mayfly]
monitor_speed = 115200
platform = atmelavr
board = mayfly
framework = arduino
lib_ignore = RTCZero
lib_ldf_mode = deep+
build_flags =
    -DSDI12_EXTERNAL_PCINT
lib_deps =
    EnviroDIY_ModularSensors
    mathworks/ThingSpeak@^2.0.0
