
########################### VARIABLES ############################

TARGET      = Test
COMPILER    = gcc
C_RELEASE   = -c -O3 -Wall
C_DEBUG     = -ggdb -c -Wall -D_KLS_MEMORY_DEBUG
#C_FLAGS     = $(C_RELEASE)
C_FLAGS     = $(C_DEBUG)
LD_FLAGS    =
SOURCES     =
OBJECTS     =
OBJECTS_ADD =

############################ FOLDERS #############################

FLD_SRC  = ./SRC/

##################################################################

C_FLAGS += -I$(FLD_SRC)

#C_FLAGS += -D_KLS_MALLOC_HEAP=1024*1024*20
#C_FLAGS += -D_KLS_SYS_OS_SETUP=KLS_SYS_OS_...
#C_FLAGS += -D_KLS_SYS_ENDIAN_SETUP=KLS_SYS_ENDIAN_...
#C_FLAGS += -D_KLS_SYS_BITNESS_SETUP=32
#C_FLAGS += -D_KLS_SYS_PLATFORM_SETUP=KLS_SYS_PLATFORM_...

##################################################################

LD_FLAGS += -lm
LD_FLAGS += -lc
LD_FLAGS += -lpthread
LD_FLAGS += -lrt
LD_FLAGS += -lX11

##################################################################

SOURCES += main.c
SOURCES += $(FLD_SRC)KLS_lib.c

##################################################################

OBJECTS += $(SOURCES:.c=.o)

##################################################################

all: $(SOURCES) $(TARGET)

##################################################################

$(TARGET): $(OBJECTS) $(OBJECTS_ADD)
	$(COMPILER) $(OBJECTS) $(OBJECTS_ADD) -o $(TARGET) $(LD_FLAGS)

.c.o:
	$(COMPILER) $(C_FLAGS) $< -o $@

##################################################################

rmo:
	rm $(OBJECTS)

clean:
	rm $(OBJECTS) $(TARGET)

