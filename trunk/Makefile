#i#############################################################################
#
# Makefile
#
###############################################################################

SOURCES  = main.cpp volume.cpp view.cpp misc.cpp photohull.cpp
DEPENDS  = $(SOURCES:.cpp=.d)
OBJECTS  = $(SOURCES:.cpp=.o)
TARGET	 = main

###############################################################################

OPTFLAGS = -O3

CC      = g++
CFLAGS  = -I/opt/local/include -Wall -Wno-format $(OPTFLAGS)
LDFLAGS = -lcv -lcvaux -lhighgui -lm


###############################################################################

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJECTS) $(DEPENDS)
	$(RM) $(TARGET)

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
