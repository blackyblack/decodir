VER       = 1.0.0

# prog name
PROG    = decodir

ifdef   VER
DVER    = -DVERSION=$(VER)
endif

PROTOCOLS_SOURCE = granit_protocols/granit_protocols_iridium.c
PROTOCOLS_O = $(addsuffix .o,$(basename $(PROTOCOLS_SOURCE)))

MAIN_SOURCE = main.c
MAIN_O = $(addsuffix .o,$(basename $(MAIN_SOURCE)))

ALL_O = $(PROTOCOLS_O) $(MAIN_O)

all: $(PROG)

CDEFS   = $(DVER) 
OFLAG    = -O3
CC       ?= gcc
CFLAGS   ?= -W -Wall -g $(OFLAG)
LFLAGS   ?=
PROJ_DIR =.

OBJS     = $(ALL_O)
INCS     = -I$(PROJ_DIR)/ -I$(PROJ_DIR)/granit_protocols/
LIBS     =

$(PROG): $(OBJS)
	$(CC) $(CDEFS) $(CFLAGS) $(LFLAGS) $(ALL_O) $(LDFLAGS) $(LIBS) -o $@

$(ALL_O): %.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $(INCS) $< -o $@

clean:
	rm -f $(ALL_O) $(PROG)

.PHONY: all clean
