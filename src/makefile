# Concentrator makefile
# 2017-02-18 
# Nayo Wang

APP_MAJOR_VERSION = 1
APP_MINOR_VERSION = 0
APP_REVISION_VERSION = 0
APP_SUB_VERSION = 0
APP_VERSION_STR = $(APP_MAJOR_VERSION).$(APP_MINOR_VERSION).$(APP_REVISION_VERSION).$(APP_SUB_VERSION)

MACRO =

RELEASE_SOFT_TIME = $(strip $(shell date +%Y-%m-%d)\ $(shell date +%H:%M:%S))
MACRO += -DRELEASE_APP_TIME=\"$(RELEASE_SOFT_TIME)\"

THREADS_CNT=7
MACRO += -DTHREADS_COUNT=$(THREADS_CNT) -DTIMER_COUNT=$(THREADS_CNT) -DAPP_VERSION=\"$(APP_VERSION_STR)\"


# binary
BIN = ./gascon
OBJS_DIR = objs/
INC_PATH = include/
INC_PATH2 = /usr/local/tslib/include/


COMPILER_DIR ?= /opt/linux-devkit/sysroots/x86_64-arago-linux/usr/bin/
#COMPILER_DIR ?= /opt/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi/bin/ 
CROSS_COMPILE ?= arm-linux-gnueabihf-
#CROSS_COMPILE ?= arm-fsl-linux-gnueabi- 
#CC = $(COMPILER_DIR)$(CROSS_COMPILE)gcc 
CC = $(COMPILER_DIR)$(CROSS_COMPILE)gcc
#CC = arm-fsl-linux-gnueabi-gcc

# compiler flag
CFLAGS := -Werror -Wall -I$(INC_PATH) -O3 -I$(INC_PATH2)
CFLAGS := -I$(INC_PATH) -Os
CFLAGS += $(MACRO)

# libray
LIBS = -lpthread -lm -lsqlite3

# objects
OBJECTS =$(addprefix $(OBJS_DIR), $(patsubst %.c, %.o, $(wildcard *.c)))

# functions
cmd_print_create = @echo "  Create $@"

# regulations & depencies

.PHONY: all
all: $(BIN)
$(BIN): $(OBJECTS)
	@$(CC) -o $(BIN) $(OBJECTS) $(CFLAGS) $(LIBS)
	@echo "  Linking OK"
	$(call cmd_print_create)

# attention
$(OBJECTS):$(OBJS_DIR)%.o: %.c
	@mkdir -p $(OBJS_DIR)
	@$(CC) $(CFLAGS) -o $@ -c $<
	$(call cmd_print_create)

.PHONY: clean
clean:
	@-rm $(OBJECTS) $(BIN)
	@echo "  Remove objects and app"

.PHONY: test
test:
	@echo "  Test"

.PHONY: scp
scp:
	$(shell scp gascon root@192.168.0.211:./gascon.bak)
	@echo "gascon -> root@192.168.0.211:gascon.bak"

.PHONY: install
install:
	$(shell cp gascon ~/nfsroot)

#@echo "gascon is sent to ~/nfs/"
	@echo "gascon is sent to ~/nfsroot"
