SRC  = $(wildcard src/*.c)
INC  = inc

# Subfolders to compile and include from asf/sam/drivers
SAM_DRIVERS = pio pmc rstc wdt dacc adc udp matrix usart tc twi hsmci uart pwm gpbr efc
SRC += $(wildcard $(SAM_DRIVERS:%=asf/sam/drivers/%/*.c))
INC += $(SAM_DRIVERS:%=asf/sam/drivers/%)

# Subfolders to compile and include from asf/common/services
COMMON_SERVICES  = clock gpio sleepmgr ioport delay serial
COMMON_SERVICES += usb usb/udc usb/class/cdc usb/class/cdc/device
COMMON_SERVICES += storage/ctrl_access
SRC += $(wildcard $(COMMON_SERVICES:%=asf/common/services/%/sam4s/*.c))
SRC += $(wildcard $(COMMON_SERVICES:%=asf/common/services/%/sam/*.c))
SRC += $(wildcard $(COMMON_SERVICES:%=asf/common/services/%/*.c))
INC += $(COMMON_SERVICES:%=asf/common/services/%)

# Subfolders to compile and include from asf/common/components
COMMON_COMPONENTS = memory/sd_mmc
SRC += $(wildcard $(COMMON_COMPONENTS:%=asf/common/components/%/*.c))
INC += $(COMMON_COMPONENTS:%=asf/common/components/%)


# Subfolders to compile and include from asf/sam/utils
SRC += asf/sam/utils/syscalls/gcc/syscalls.c
SRC += asf/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c
SRC += asf/sam/utils/cmsis/sam4s/source/templates/gcc/startup_sam4s.c
INC += asf/sam/utils
INC += asf/sam/utils/header_files
INC += asf/sam/utils/preprocessor
INC += asf/sam/utils/cmsis/sam4s/include

# Subfolders to compile and include from asf/common/utils
SRC += $(wildcard asf/common/utils/interrupt/*.c)
SRC += $(wildcard asf/common/utils/stdio/*.c)
SRC += $(wildcard asf/common/utils/membag/*.c)
INC += asf/common/utils/membag
INC += asf/common/utils

# FAT Library
SRC += asf/thirdparty/fatfs/fatfs-port-r0.09/diskio.c
SRC += asf/thirdparty/fatfs/fatfs-port-r0.09/sam/fattime_rtc.c
SRC += asf/thirdparty/fatfs/fatfs-r0.09/src/ff.c
SRC += asf/thirdparty/fatfs/fatfs-r0.09/src/option/ccsbcs.c
INC += asf/thirdparty/fatfs/fatfs-port-r0.09/sam
INC += asf/thirdparty/fatfs/fatfs-r0.09/src 



# All other ASF header paths
INC += asf/common/boards
INC += asf/thirdparty/CMSIS/Include

# Specific source exclusions, because ASF is inconsistent
EXCLUDE  = asf/sam/drivers/wdt/wdt_sam4l.c
EXCLUDE += asf/sam/drivers/adc/adc2.c
EXCLUDE += asf/common/services/usb/udc/udc_dfu_small.c
SRC := $(filter-out $(EXCLUDE),$(SRC))
