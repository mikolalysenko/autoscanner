#i#############################################################################
#
# Makefile
#
###############################################################################

SOURCES  = video_loader.cpp volume.cpp view.cpp misc.cpp photohull.cpp \
    config.cpp kutulakis.cpp consistency.cpp

DEPENDS  = $(SOURCES:.cpp=.d)
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET	 = main
VOLTARGET= volmain


###############################################################################

OPTFLAGS = -ggdb

CC      = g++
CFLAGS  = -I/opt/local/include -Wall -Wno-format $(OPTFLAGS)
LDFLAGS = -lcv -lcvaux -lhighgui -lm


###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS) main.o
	$(CC) $(CFLAGS) main.o $(OBJECTS) $(LDFLAGS) -o $@

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
