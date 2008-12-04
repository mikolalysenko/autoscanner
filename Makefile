#i#############################################################################
#
# Makefile
#
###############################################################################

SOURCES  = volume.cpp view.cpp misc.cpp photohull.cpp

DEPENDS  = $(SOURCES:.cpp=.d)
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET	 = main
VOLTARGET= volmain


###############################################################################

OPTFLAGS = -O3

CC      = g++
CFLAGS  = -I/opt/local/include -Wall -Wno-format $(OPTFLAGS)
LDFLAGS = -lcv -lcvaux -lhighgui -lm


###############################################################################

all: $(TARGET)

vol: $(VOLTARGET)

$(TARGET): $(OBJECTS) main.o
	$(CC) $(CFLAGS) main.o $(OBJECTS) $(LDFLAGS) -o $@

$(VOLTARGET): $(OBJECTS) vol_main.o video_loader.o volume_cuts.o
	$(CC) $(CFLAGS) vol_main.o video_loader.o volume_cuts.o $(OBJECTS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(TARGET) $(VOLTARGET)

.PHONY: all clean

###############################################################################

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

%.d: %.cpp
	$(CC) $(CFLAGS) -MM $< > $@

###############################################################################

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif

###############################################################################
