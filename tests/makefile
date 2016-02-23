CC = gcc
CFLAGS = -Wall -O2
LFLAGS =
INCLUDES = -Ixc32/
LIBS = -lcmocka
RM = rm

SOURCEDIR = ../src/
BUILDDIR = build
INCLUDES += -I$(SOURCEDIR)


all: dir $(BUILDDIR)/test_rfid $(BUILDDIR)/test_gsm


dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/test_rfid:
	$(CC) $(INCLUDES) -o $(BUILDDIR)/test_rfid test_rfid.c $(LFLAGS) $(LIBS) \
		-Wl,--wrap=WriteCoreTimer,--wrap=ReadCoreTimer,--wrap=write_string,--wrap=delay_ms

$(BUILDDIR)/test_gsm:
	$(CC) $(INCLUDES) -o $(BUILDDIR)/test_gsm test_gsm.c $(LFLAGS) $(LIBS) \
		-Wl,--wrap=ReadTimer23,--wrap=WriteTimer23,--wrap=OpenTimer23,--wrap=CloseTimer23,--wrap=delay_ms,--wrap=write_string,--wrap=WriteCoreTimer,--wrap=ReadCoreTimer

clean:
	$(RM) $(BUILDDIR)/*
