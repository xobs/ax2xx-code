SOURCES=gpio.c sd.c main.c crc-16.c eim.c debugger.c \
		disasm.c disasm-mnem.c nand.c
OBJECTS=$(SOURCES:.c=.o)
EXEC=../../ax211
MY_CFLAGS += -Wall -O0 -g
MY_LIBS += -lreadline

all: $(OBJECTS)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJECTS) $(MY_LIBS) -o $(EXEC)

clean:
	rm -f $(EXEC) $(OBJECTS)

.c.o:
	$(CC) -c $(CFLAGS) $(MY_CFLAGS) $< -o $@

