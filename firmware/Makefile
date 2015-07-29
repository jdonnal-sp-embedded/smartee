# Makefile for Atmel SAM4S using cmsis and GNU toolchain.

# The variables $(SRC) and $(INC) are defined in path.mk.
include path.mk

# Object file location and linker script
OBJ = $(SRC:%.c=obj/%.o)
LD_SCRIPT = asf/sam/utils/linker_scripts/sam4s/sam4s4/gcc/%.ld

# Compiler and linker flags. Here be dragons.
CFLAGS  = -mlittle-endian -mthumb -mcpu=cortex-m4
CFLAGS += -g -O0 $(INC:%=-I%) -std=gnu99 -Wall #change to gnu99 for inline assembly
CFLAGS += -DARM_MATH_CM4 -D'__SAM4S4B__' -D'BOARD=USER_BOARD'
LFLAGS  = $(CFLAGS) -T$(@:bin/%.elf=$(LD_SCRIPT))
LFLAGS += -Wl,--entry=Reset_Handler -Wl,--gc-sections

# Targets
.PHONY: all clean gdb
.SECONDARY: $(OBJ)
all: bin/flash.bin bin/flash.elf
clean:
	-rm -rf obj bin
gdb: bin/flash.elf
	@arm-none-eabi-gdb
bin/%.bin: bin/%.elf
	arm-none-eabi-objcopy -O binary $< $@
bin/%.hex: bin/%.elf
	arm-none-eabi-objcopy -O ihex $< $@
bin/%.elf: $(OBJ)
	@mkdir -p $(dir $@)
	$(info LD $@)
	@arm-none-eabi-gcc $(LFLAGS) -o $@ $(OBJ)
obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(info CC $<)
	@arm-none-eabi-gcc $(CFLAGS) -c $< -o $@
