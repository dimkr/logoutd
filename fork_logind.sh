# This file is part of logoutd.
#
# Copyright 2015 Dima Krasner
#
# logoutd is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2.1 of the License, or (at your option)
# any later version.
#
# logoutd is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with logoutd; If not, see <http://www.gnu.org/licenses/>.

#!/bin/sh

FILES="systemd-204/src/login/logind.c systemd-204/src/login/logind-gperf.gperf systemd-204/src/login/logind-acl.c systemd-204/src/login/logind-action.c systemd-204/src/login/logind-button.c systemd-204/src/login/logind-dbus.c systemd-204/src/login/logind-device.c systemd-204/src/login/logind-inhibit.c systemd-204/src/login/logind-seat.c systemd-204/src/login/logind-seat-dbus.c systemd-204/src/login/logind-session.c systemd-204/src/login/logind-session-dbus.c systemd-204/src/login/logind-user.c systemd-204/src/login/logind-user-dbus.c sd-login.c sysfs-show.c systemd-204/src/libsystemd-daemon/sd-daemon.c systemd-204/src/shared/strv.c systemd-204/src/shared/label.c systemd-204/src/shared/set.c systemd-204/src/shared/unit-name.c systemd-204/src/shared/sleep-config.c systemd-204/src/shared/log.c systemd-204/src/shared/mkdir.c systemd-204/src/shared/fileio.c systemd-204/src/shared/path-util.c systemd-204/src/shared/time-util.c systemd-204/src/shared/cgroup-util.c systemd-204/src/shared/hashmap.c systemd-204/src/shared/dbus-common.c systemd-204/src/shared/util.c systemd-204/src/shared/env-util.c systemd-204/src/shared/exit-status.c systemd-204/src/shared/utf8.c systemd-204/src/shared/polkit.c systemd-204/src/shared/acl-util.c systemd-204/src/shared/conf-parser.c systemd-204/src/shared/dbus-loop.c systemd-204/src/shared/cgroup-label.c systemd-204/src/shared/audit.c systemd-204/src/shared/virt.c systemd-204/src/shared/capability.c systemd-204/src/shared/fileio-label.c systemd-204/src/shared/macro.h systemd-204/src/systemd/sd-id128.h systemd-204/src/shared/ioprio.h systemd-204/src/shared/missing.h systemd-204/src/shared/def.h systemd-204/src/systemd/sd-messages.h systemd-204/src/systemd/sd-sdaemon.h systemd-204/src/shared/list.h systemd-204/src/core/special.h systemd-204/src/shared/socket-util.h systemd-204/src/systemd/sd-daemon.h systemd-204/LICENSE.LGPL2.1 systemd-204/LICENSE.MIT"

[ ! -f systemd-204.tar.xz ] && wget http://www.freedesktop.org/software/systemd/systemd-204.tar.xz
[ -d systemd-204 ] && rm -rf systemd-204
tar -xJvf systemd-204.tar.xz

[ -d logind-204 ] && rm -rf logind-204
mkdir logind-204
mv -vf $FILES logind-204/
for i in $FILES
do
	h="${i%%.*}.h"
	[ "$h" = "$i" ] && continue
	[ -f "$h" ] && mv -vf "$h" logind-204/
done

cd logind-204
ln -s . systemd

echo 'CC ?= cc
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
	install -m 644 LICENSE.MIT $(DESTDIR)$(DOC_DIR)/$(PACKAGE)/LICENSE.MIT' > Makefile

echo ' _                         _      _
| | ___   __ _  ___  _   _| |_ __| |
| |/ _ \ / _` |/ _ \| | | | __/ _` |
| | (_) | (_| | (_) | |_| | || (_| |
|_|\___/ \__, |\___/ \__,_|\__\__,_|
         |___/

Overview
========

logoutd is a small, standalone, incredibly experimental fork of logind from
systemd (http://www.freedesktop.org/software/systemd/) 204.

The entire forking process is documented in fork_logind.sh.

Credits and Legal Information
=============================

All code in logoutd is free software released under the terms of the LGPLv2.1+
license, except sd-daemon.c and sd-daemon, which are MIT-licensed. For a list of
its authors and contributors, see AUTHORS.

The ASCII art logo at the top was made using FIGlet (http://www.figlet.org/).' > README

echo "Lennart Poettering
Kay Sievers
many others" > AUTHORS

make
make DESTDIR=out install
find out
