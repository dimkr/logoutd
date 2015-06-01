/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <errno.h>
#include <string.h>
#include <assert.h>

#include "path-util.h"
#include "util.h"
#include "unit-name.h"

#define VALID_CHARS                             \
        "0123456789"                            \
        "abcdefghijklmnopqrstuvwxyz"            \
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"            \
        ":-_.\\"

static const char* const unit_type_table[_UNIT_TYPE_MAX] = {
        [UNIT_SERVICE] = "service",
        [UNIT_SOCKET] = "socket",
        [UNIT_TARGET] = "target",
        [UNIT_DEVICE] = "device",
        [UNIT_MOUNT] = "mount",
        [UNIT_AUTOMOUNT] = "automount",
        [UNIT_SNAPSHOT] = "snapshot",
        [UNIT_TIMER] = "timer",
        [UNIT_SWAP] = "swap",
        [UNIT_PATH] = "path",
};

DEFINE_STRING_TABLE_LOOKUP(unit_type, UnitType);

static const char* const unit_load_state_table[_UNIT_LOAD_STATE_MAX] = {
        [UNIT_STUB] = "stub",
        [UNIT_LOADED] = "loaded",
        [UNIT_ERROR] = "error",
        [UNIT_MERGED] = "merged",
        [UNIT_MASKED] = "masked"
};

DEFINE_STRING_TABLE_LOOKUP(unit_load_state, UnitLoadState);

bool unit_name_is_valid(const char *n, bool template_ok) {
        const char *e, *i, *at;

        /* Valid formats:
         *
         *         string@instance.suffix
         *         string.suffix
         */

        assert(n);

        if (strlen(n) >= UNIT_NAME_MAX)
                return false;

        e = strrchr(n, '.');
        if (!e || e == n)
                return false;

        if (unit_type_from_string(e + 1) < 0)
                return false;

        for (i = n, at = NULL; i < e; i++) {

                if (*i == '@' && !at)
                        at = i;

                if (!strchr("@" VALID_CHARS, *i))
                        return false;
        }

        if (at) {
                if (at == n)
                        return false;

                if (!template_ok && at+1 == e)
                        return false;
        }

        return true;
}

bool unit_instance_is_valid(const char *i) {
        assert(i);

        /* The max length depends on the length of the string, so we
         * don't really check this here. */

        if (i[0] == 0)
                return false;

        /* We allow additional @ in the instance string, we do not
         * allow them in the prefix! */

        for (; *i; i++)
                if (!strchr("@" VALID_CHARS, *i))
                        return false;

        return true;
}

bool unit_prefix_is_valid(const char *p) {

        /* We don't allow additional @ in the instance string */

        if (p[0] == 0)
                return false;

        for (; *p; p++)
                if (!strchr(VALID_CHARS, *p))
                        return false;

        return true;
}

int unit_name_to_instance(const char *n, char **instance) {
        const char *p, *d;
        char *i;

        assert(n);
        assert(instance);

        /* Everything past the first @ and before the last . is the instance */
        p = strchr(n, '@');
        if (!p) {
                *instance = NULL;
                return 0;
        }

        assert_se(d = strrchr(n, '.'));
        assert(p < d);

        i = strndup(p+1, d-p-1);
        if (!i)
                return -ENOMEM;

        *instance = i;
        return 0;
}

char *unit_name_to_prefix_and_instance(const char *n) {
        const char *d;

        assert(n);

        assert_se(d = strrchr(n, '.'));

        return strndup(n, d - n);
}

char *unit_name_to_prefix(const char *n) {
        const char *p;

        p = strchr(n, '@');
        if (p)
                return strndup(n, p - n);

        return unit_name_to_prefix_and_instance(n);
}

char *unit_name_change_suffix(const char *n, const char *suffix) {
        char *e, *r;
        size_t a, b;

        assert(n);
        assert(unit_name_is_valid(n, true));
        assert(suffix);

        assert_se(e = strrchr(n, '.'));
        a = e - n;
        b = strlen(suffix);

        r = new(char, a + b + 1);
        if (!r)
                return NULL;

        memcpy(r, n, a);
        memcpy(r+a, suffix, b+1);

        return r;
}

char *unit_name_build(const char *prefix, const char *instance, const char *suffix) {
        assert(prefix);
        assert(unit_prefix_is_valid(prefix));
        assert(!instance || unit_instance_is_valid(instance));
        assert(suffix);

        if (!instance)
                return strappend(prefix, suffix);

        return strjoin(prefix, "@", instance, suffix, NULL);
}

static char *do_escape_char(char c, char *t) {
        *(t++) = '\\';
        *(t++) = 'x';
        *(t++) = hexchar(c >> 4);
        *(t++) = hexchar(c);
        return t;
}

static char *do_escape(const char *f, char *t) {
        assert(f);
        assert(t);

        /* do not create units with a leading '.', like for "/.dotdir" mount points */
        if (*f == '.') {
                t = do_escape_char(*f, t);
                f++;
        }

        for (; *f; f++) {
                if (*f == '/')
                        *(t++) = '-';
                else if (*f == '-' || *f == '\\' || !strchr(VALID_CHARS, *f))
                        t = do_escape_char(*f, t);
                else
                        *(t++) = *f;
        }

        return t;
}

char *unit_name_escape(const char *f) {
        char *r, *t;

        r = new(char, strlen(f)*4+1);
        if (!r)
                return NULL;

        t = do_escape(f, r);
        *t = 0;

        return r;
}

char *unit_name_unescape(const char *f) {
        char *r, *t;

        assert(f);

        r = strdup(f);
        if (!r)
                return NULL;

        for (t = r; *f; f++) {
                if (*f == '-')
                        *(t++) = '/';
                else if (*f == '\\') {
                        int a, b;

                        if (f[1] != 'x' ||
                            (a = unhexchar(f[2])) < 0 ||
                            (b = unhexchar(f[3])) < 0) {
                                /* Invalid escape code, let's take it literal then */
                                *(t++) = '\\';
                        } else {
                                *(t++) = (char) ((a << 4) | b);
                                f += 3;
                        }
                } else
                        *(t++) = *f;
        }

        *t = 0;

        return r;
}

char *unit_name_path_escape(const char *f) {
        char *p, *e;

        assert(f);

        p = strdup(f);
        if (!p)
                return NULL;

        path_kill_slashes(p);

        if (streq(p, "/")) {
                free(p);
                return strdup("-");
        }

        e = unit_name_escape(p[0] == '/' ? p + 1 : p);
        free(p);

        return e;
}

char *unit_name_path_unescape(const char *f) {
        char *e;

        assert(f);

        e = unit_name_unescape(f);
        if (!e)
                return NULL;

        if (e[0] != '/') {
                char *w;

                w = strappend("/", e);
                free(e);

                return w;
        }

        return e;
}

bool unit_name_is_template(const char *n) {
        const char *p;

        assert(n);

        p = strchr(n, '@');
        if (!p)
                return false;

        return p[1] == '.';
}

bool unit_name_is_instance(const char *n) {
        const char *p;

        assert(n);

        p = strchr(n, '@');
        if (!p)
                return false;

        return p[1] != '.';
}

char *unit_name_replace_instance(const char *f, const char *i) {
        const char *p, *e;
        char *r, *k;
        size_t a, b;

        assert(f);

        p = strchr(f, '@');
        if (!p)
                return strdup(f);

        e = strrchr(f, '.');
        if (!e)
                assert_se(e = strchr(f, 0));

        a = p - f;
        b = strlen(i);

        r = new(char, a + 1 + b + strlen(e) + 1);
        if (!r)
                return NULL;

        k = mempcpy(r, f, a + 1);
        k = mempcpy(k, i, b);
        strcpy(k, e);

        return r;
}

char *unit_name_template(const char *f) {
        const char *p, *e;
        char *r;
        size_t a;

        p = strchr(f, '@');
        if (!p)
                return strdup(f);

        assert_se(e = strrchr(f, '.'));
        a = p - f + 1;

        r = new(char, a + strlen(e) + 1);
        if (!r)
                return NULL;

        strcpy(mempcpy(r, f, a), e);
        return r;

}

char *unit_name_from_path(const char *path, const char *suffix) {
        char *p, *r;

        assert(path);
        assert(suffix);

        p = unit_name_path_escape(path);
        if (!p)
                return NULL;

        r = strappend(p, suffix);
        free(p);

        return r;
}

char *unit_name_from_path_instance(const char *prefix, const char *path, const char *suffix) {
        char *p, *r;

        assert(prefix);
        assert(path);
        assert(suffix);

        p = unit_name_path_escape(path);
        if (!p)
                return NULL;

        r = strjoin(prefix, "@", p, suffix, NULL);
        free(p);

        return r;
}

char *unit_name_to_path(const char *name) {
        char *w, *e;

        assert(name);

        w = unit_name_to_prefix(name);
        if (!w)
                return NULL;

        e = unit_name_path_unescape(w);
        free(w);

        return e;
}

char *unit_dbus_path_from_name(const char *name) {
        char *e, *p;

        assert(name);

        e = bus_path_escape(name);
        if (!e)
                return NULL;

        p = strappend("/org/freedesktop/systemd1/unit/", e);
        free(e);

        return p;
}

char *unit_name_mangle(const char *name) {
        char *r, *t;
        const char *f;

        assert(name);

        /* Try to turn a string that might not be a unit name into a
         * sensible unit name. */

        if (is_device_path(name))
                return unit_name_from_path(name, ".device");

        if (path_is_absolute(name))
                return unit_name_from_path(name, ".mount");

        /* We'll only escape the obvious characters here, to play
         * safe. */

        r = new(char, strlen(name) * 4 + 1 + sizeof(".service")-1);
        if (!r)
                return NULL;

        for (f = name, t = r; *f; f++) {
                if (*f == '/')
                        *(t++) = '-';
                else if (!strchr("@" VALID_CHARS, *f))
                        t = do_escape_char(*f, t);
                else
                        *(t++) = *f;
        }

        if (unit_name_to_type(name) < 0)
                strcpy(t, ".service");
        else
                *t = 0;

        return r;
}

char *snapshot_name_mangle(const char *name) {
        char *r, *t;
        const char *f;

        assert(name);

        /* Similar to unit_name_mangle(), but is called when we know
         * that this is about snapshot units. */

        r = new(char, strlen(name) * 4 + 1 + sizeof(".snapshot")-1);
        if (!r)
                return NULL;

        for (f = name, t = r; *f; f++) {
                if (*f == '/')
                        *(t++) = '-';
                else if (!strchr(VALID_CHARS, *f))
                        t = do_escape_char(*f, t);
                else
                        *(t++) = *f;
        }

        if (!endswith(name, ".snapshot"))
                strcpy(t, ".snapshot");
        else
                *t = 0;

        return r;
}

UnitType unit_name_to_type(const char *n) {
        const char *e;

        assert(n);

        e = strrchr(n, '.');
        if (!e)
                return _UNIT_TYPE_INVALID;

        return unit_type_from_string(e + 1);
}
