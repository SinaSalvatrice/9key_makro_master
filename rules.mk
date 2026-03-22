MCU = RP2040
BOOTLOADER = rp2040
PLATFORM = rp2040
PYTHON = python
CONSOLE_ENABLE = yes
# No ChibiOS or TMK dependencies for RP2040
STRICT = no
EXTRAFLAGS += -Wno-error
