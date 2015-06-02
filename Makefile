CC ?= cc
CFLAGS ?= -O2
LDFLAGS ?=
DESTDIR ?= /

PREFIX ?= /usr
BIN_DIR ?= $(PREFIX)/bin
SBIN_DIR ?= $(PREFIX)/sbin
MAN_DIR ?= $(PREFIX)/man
DATA_DIR ?= $(PREFIX)/share
DOC_DIR ?= $(DATA_DIR)/doc
CONF_DIR ?= /etc

PACKAGE = logoutd

CFLAGS += -std=gnu99 \
          -Wall \
          -pedantic \
          -D_GNU_SOURCE \
          -DDISABLE_SYSTEMD \
          -DPACKAGE_STRING=\"$(PACKAGE)\" \
          -D_GNU_SOURCE \
          -DX_SERVER=\"$(BIN_DIR)/X\" \
          -DHAVE_ACL \
          -D_FILE_OFFSET_BITS=64 \
          -DHAVE_DECL_NAME_TO_HANDLE_AT=1 \
          -DHAVE_SECURE_GETENV \
          -DSYSTEMD_STDIO_BRIDGE_BINARY_PATH=\"$(BIN_DIR)/systemd-stdio-bridge\" \
          -DPKGSYSCONFDIR=\"$(CONF_DIR)\" \
          -I. \
          $(shell pkg-config --cflags libudev dbus-1)
LDFLAGS += $(shell pkg-config --libs libudev dbus-1) \
           -lpam \
           -lpam_misc \
           -lacl \
           -lcap \
           -lrt

SRCS = $(wildcard *.c) logind-gperf.c
OBJECTS = $(SRCS:.c=.o)

all: logoutd org.freedesktop.login1.service

logind-gperf.c: logind-gperf.gperf
	gperf < $< > $@

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

logoutd: $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

org.freedesktop.login1.service: org.freedesktop.login1.service.in
	sed s~@SBIN_DIR@~$(SBIN_DIR)~ $< > $@

clean:
	rm -f logoutd $(OBJECTS) logind-gperf.c

install: all
	install -D -m 755 logoutd $(DESTDIR)$(SBIN_DIR)/logoutd
	install -D -m 755 logoutd-launch $(DESTDIR)$(SBIN_DIR)/logoutd-launch
	install -D -m 644 org.freedesktop.login1.service $(DESTDIR)$(DATA_DIR)/dbus-1/system-services/org.freedesktop.login1.service
	install -D -m 644 org.freedesktop.login1.policy $(DESTDIR)$(DATA_DIR)/polkit-1/actions/org.freedesktop.login1.policy
	install -D -m 644 README $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/README
	install -m 644 AUTHORS $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/AUTHORS
	install -m 644 LICENSE.LGPL2.1 $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/LICENSE.LGPL2.1
	install -m 644 LICENSE.MIT $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/LICENSE.MIT
