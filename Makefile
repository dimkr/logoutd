CC ?= cc
CFLAGS ?= -O2
LDFLAGS ?=
DESTDIR ?= /

PREFIX ?= /usr
BIN_DIR ?= $(PREFIX)/bin
SBIN_DIR ?= $(PREFIX)/sbin
MAN_DIR ?= $(PREFIX)/man
DOC_DIR ?= $(PREFIX)/doc
CONF_DIR ?= /etc

PACKAGE = logoutd

CFLAGS += -std=gnu99 \
          -Wall \
          -pedantic \
          -D_GNU_SOURCE \
          -I. \
          -DPACKAGE_STRING=\"$(PACKAGE)\" \
          -D_GNU_SOURCE \
          -DX_SERVER=\"$(BIN_DIR)/X\" \
          -DHAVE_ACL \
          -D_FILE_OFFSET_BITS=64 \
          -DHAVE_DECL_NAME_TO_HANDLE_AT=1 \
          -DHAVE_SECURE_GETENV \
          -DSYSTEMD_STDIO_BRIDGE_BINARY_PATH=\"$(BIN_DIR)/systemd-stdio-bridge\" \
          -DPKGSYSCONFDIR=\"$(CONF_DIR)\" \
          $(shell pkg-config --cflags libudev dbus-1)
LDFLAGS += $(shell pkg-config --libs libudev dbus-1) \
           -lpam \
           -lpam_misc \
           -lacl \
           -lcap \
           -lrt

SRCS = $(wildcard *.c) logind-gperf.c
OBJECTS = $(SRCS:.c=.o)
DAEMON = $(PACKAGE)

all: $(DAEMON)

logind-gperf.c: logind-gperf.gperf
	gperf < $< > $@

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(DAEMON): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(DAEMON) $(OBJECTS) logind-gperf.c

install: $(DAEMON)
	install -D -m 755 $(DAEMON) $(DESTDIR)$(SBIN_DIR)/$(DAEMON)
	install -D -m 644 README $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/README
	install -m 644 AUTHORS $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/AUTHORS
	install -m 644 LICENSE.LGPL2.1 $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/LICENSE.LGPL2.1
	install -m 644 LICENSE.MIT $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/LICENSE.MIT
