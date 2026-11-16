# Specifies the name of the exe file
APP := test

# Add here other sources if you have (i.e. APP_CFLAGS += my_conv.c)
APP_SRCS := $(wildcard src/*.c)

# Specifies c compiler's flags
APP_CFLAGS += -O3 -Iinc

# Define the available cores for this application
APP_CFLAGS += -DNB_CORES=8


# Prepare the lab (simulator memory configuration, see initialization.sh)
INIT_OUTPUT := $(shell bash $(CURDIR)/initialization.sh 2>&1)
ifneq ($(INIT_OUTPUT),)
$(info $(INIT_OUTPUT))
endif

# The lab-local linker script chips/pulp/link.ld (64kB of L1) is picked up instead of the PULP SDK one,
# since the linker looks for -Tchips/pulp/link.ld in the current directory first.

include $(RULES_DIR)/pmsis_rules.mk
