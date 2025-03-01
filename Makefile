TARGET_OUT = image.elf
all : $(TARGET_OUT)

#SUBMODULE if we are including this in another project.
SUBMODULE?=NO

#Pick from *52, *80, *115, 160, *173, *189, 231, 320 or 346 or (for now)
#Peripheral clocks of >115 will NOT boot without a full power-down and up. (Don't know why).  * = peripheral clock at processor clock.
MAIN_MHZ?=346

ESPTOOL:=~/esp/ESP8266_RTOS_SDK/components/esptool_py/esptool/esptool.py
GCC_FOLDER:=~/esp/xtensa-lx106-elf
PREFIX:=$(GCC_FOLDER)/bin/xtensa-lx106-elf-
SIZE:=$(PREFIX)size
GCC:=$(PREFIX)gcc

#-mno-serialize-volatile will prevent extra memw things from being generated.

ifneq (YES, $(SUBMODULE))
	SRCPREFIX:=
	CFLAGS:=$(CFLAGS) -flto
	SRCS:=$(SRCS) main.c
else
	SRCPREFIX:=nosdk8266/
endif

LDFLAGS:=-T $(SRCPREFIX)ld/linkerscript.ld -T $(SRCPREFIX)ld/addresses.ld
FOLDERPREFIX:=$(GCC_FOLDER)/bin
PORT:=/dev/ttyS12

#Adding the -g flag makes our assembly easier to read and does not increase size of final executable.
CFLAGS:=$(CFLAGS) -Ofast -I$(SRCPREFIX)include -DMAIN_MHZ=$(MAIN_MHZ) -mno-serialize-volatile -mlongcalls -g
SRCS:=$(SRCS) $(SRCPREFIX)src/delay.S $(SRCPREFIX)src/nosdk8266.c

$(TARGET_OUT) : $(SRCS)
	$(GCC) $(CFLAGS) $(SRCS) $(LDFLAGS) -o $@
	nm -S -n $(TARGET_OUT) > image.map
	$(SIZE) $@
	$(PREFIX)objdump -S $@ > image.lst
	PATH=$(FOLDERPREFIX):$$PATH;$(ESPTOOL) elf2image $(TARGET_OUT) -o $(TARGET_OUT)-0x00000.bin

burn : $(FW_FILE_1) $(FW_FILE_2) $(TARGET_OUT)
	($(ESPTOOL) --after soft_reset --no-stub --port $(PORT) write_flash 0x00000 $(TARGET_OUT)-0x00000.bin -fm dout)||(true)

clean :
	rm -rf $(TARGET_OUT) image.map image.lst $(TARGET_OUT)-0x10000.bin $(TARGET_OUT)-0x00000.bin

