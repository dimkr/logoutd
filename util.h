/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

#pragma once

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

#include <alloca.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sched.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/resource.h>
#include <stddef.h>
#include <unistd.h>
#include <locale.h>

#include "macro.h"
#include "time-util.h"

union dirent_storage {
        struct dirent de;
        uint8_t storage[offsetof(struct dirent, d_name) +
                        ((NAME_MAX + 1 + sizeof(long)) & ~(sizeof(long) - 1))];
};

/* What is interpreted as whitespace? */
#define WHITESPACE " \t\n\r"
#define NEWLINE "\n\r"
#define QUOTES "\"\'"
#define COMMENTS "#;"

#define FORMAT_BYTES_MAX 8

#define ANSI_HIGHLIGHT_ON "\x1B[1;39m"
#define ANSI_RED_ON "\x1B[31m"
#define ANSI_HIGHLIGHT_RED_ON "\x1B[1;31m"
#define ANSI_GREEN_ON "\x1B[32m"
#define ANSI_HIGHLIGHT_GREEN_ON "\x1B[1;32m"
#define ANSI_HIGHLIGHT_YELLOW_ON "\x1B[1;33m"
#define ANSI_HIGHLIGHT_OFF "\x1B[0m"
#define ANSI_ERASE_TO_END_OF_LINE "\x1B[K"

size_t page_size(void);
#define PAGE_ALIGN(l) ALIGN_TO((l), page_size())

#define streq(a,b) (strcmp((a),(b)) == 0)
#define strneq(a, b, n) (strncmp((a), (b), (n)) == 0)
#define strcaseeq(a,b) (strcasecmp((a),(b)) == 0)
#define strncaseeq(a, b, n) (strncasecmp((a), (b), (n)) == 0)

bool streq_ptr(const char *a, const char *b) _pure_;

#define new(t, n) ((t*) malloc_multiply(sizeof(t), (n)))

#define new0(t, n) ((t*) calloc((n), sizeof(t)))

#define newa(t, n) ((t*) alloca(sizeof(t)*(n)))

#define newdup(t, p, n) ((t*) memdup_multiply(p, sizeof(t), (n)))

#define malloc0(n) (calloc((n), 1))

static inline const char* yes_no(bool b) {
        return b ? "yes" : "no";
}

static inline const char* strempty(const char *s) {
        return s ? s : "";
}

static inline const char* strnull(const char *s) {
        return s ? s : "(null)";
}

static inline const char *strna(const char *s) {
        return s ? s : "n/a";
}

static inline bool isempty(const char *p) {
        return !p || !p[0];
}

char *endswith(const char *s, const char *postfix) _pure_;
char *startswith(const char *s, const char *prefix) _pure_;
char *startswith_no_case(const char *s, const char *prefix) _pure_;

bool first_word(const char *s, const char *word) _pure_;

int close_nointr(int fd);
void close_nointr_nofail(int fd);
void close_many(const int fds[], unsigned n_fd);

int parse_boolean(const char *v) _pure_;
int parse_bytes(const char *t, off_t *bytes);
int parse_pid(const char *s, pid_t* ret_pid);
int parse_uid(const char *s, uid_t* ret_uid);
#define parse_gid(s, ret_uid) parse_uid(s, ret_uid)

int safe_atou(const char *s, unsigned *ret_u);
int safe_atoi(const char *s, int *ret_i);

int safe_atollu(const char *s, unsigned long long *ret_u);
int safe_atolli(const char *s, long long int *ret_i);

int safe_atod(const char *s, double *ret_d);

#if __WORDSIZE == 32
static inline int safe_atolu(const char *s, unsigned long *ret_u) {
        assert_cc(sizeof(unsigned long) == sizeof(unsigned));
        return safe_atou(s, (unsigned*) ret_u);
}
static inline int safe_atoli(const char *s, long int *ret_u) {
        assert_cc(sizeof(long int) == sizeof(int));
        return safe_atoi(s, (int*) ret_u);
}
#else
static inline int safe_atolu(const char *s, unsigned long *ret_u) {
        assert_cc(sizeof(unsigned long) == sizeof(unsigned long long));
        return safe_atollu(s, (unsigned long long*) ret_u);
}
static inline int safe_atoli(const char *s, long int *ret_u) {
        assert_cc(sizeof(long int) == sizeof(long long int));
        return safe_atolli(s, (long long int*) ret_u);
}
#endif

static inline int safe_atou32(const char *s, uint32_t *ret_u) {
        assert_cc(sizeof(uint32_t) == sizeof(unsigned));
        return safe_atou(s, (unsigned*) ret_u);
}

static inline int safe_atoi32(const char *s, int32_t *ret_i) {
        assert_cc(sizeof(int32_t) == sizeof(int));
        return safe_atoi(s, (int*) ret_i);
}

static inline int safe_atou64(const char *s, uint64_t *ret_u) {
        assert_cc(sizeof(uint64_t) == sizeof(unsigned long long));
        return safe_atollu(s, (unsigned long long*) ret_u);
}

static inline int safe_atoi64(const char *s, int64_t *ret_i) {
        assert_cc(sizeof(int64_t) == sizeof(long long int));
        return safe_atolli(s, (long long int*) ret_i);
}

char *split(const char *c, size_t *l, const char *separator, char **state);
char *split_quoted(const char *c, size_t *l, char **state);

#define FOREACH_WORD(word, length, s, state)                            \
        for ((state) = NULL, (word) = split((s), &(length), WHITESPACE, &(state)); (word); (word) = split((s), &(length), WHITESPACE, &(state)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        for ((state) = NULL, (word) = split((s), &(length), (separator), &(state)); (word); (word) = split((s), &(length), (separator), &(state)))

#define FOREACH_WORD_QUOTED(word, length, s, state)                     \
        for ((state) = NULL, (word) = split_quoted((s), &(length), &(state)); (word); (word) = split_quoted((s), &(length), &(state)))

pid_t get_parent_of_pid(pid_t pid, pid_t *ppid);
int get_starttime_of_pid(pid_t pid, unsigned long long *st);

char *strappend(const char *s, const char *suffix);
char *strnappend(const char *s, const char *suffix, size_t length);

char *replace_env(const char *format, char **env);
char **replace_env_argv(char **argv, char **env);

int readlink_malloc(const char *p, char **r);
int readlink_and_make_absolute(const char *p, char **r);
int readlink_and_canonicalize(const char *p, char **r);

int reset_all_signal_handlers(void);

char *strstrip(char *s);
char *delete_chars(char *s, const char *bad);
char *truncate_nl(char *s);

char *file_in_same_dir(const char *path, const char *filename);

int rmdir_parents(const char *path, const char *stop);

int get_process_comm(pid_t pid, char **name);
int get_process_cmdline(pid_t pid, size_t max_length, bool comm_fallback, char **line);
int get_process_exe(pid_t pid, char **name);
int get_process_uid(pid_t pid, uid_t *uid);
int get_process_gid(pid_t pid, gid_t *gid);

char hexchar(int x) _const_;
int unhexchar(char c) _const_;
char octchar(int x) _const_;
int unoctchar(char c) _const_;
char decchar(int x) _const_;
int undecchar(char c) _const_;

char *cescape(const char *s);
char *cunescape(const char *s);
char *cunescape_length(const char *s, size_t length);
char *cunescape_length_with_prefix(const char *s, size_t length, const char *prefix);

char *xescape(const char *s, const char *bad);

char *bus_path_escape(const char *s);
char *bus_path_unescape(const char *s);

char *ascii_strlower(char *path);

bool dirent_is_file(const struct dirent *de) _pure_;
bool dirent_is_file_with_suffix(const struct dirent *de, const char *suffix) _pure_;

bool ignore_file(const char *filename) _pure_;

bool chars_intersect(const char *a, const char *b) _pure_;

int make_stdio(int fd);
int make_null_stdio(void);
int make_console_stdio(void);

unsigned long long random_ull(void);

/* For basic lookup tables with strictly enumerated entries */
#define __DEFINE_STRING_TABLE_LOOKUP(name,type,scope)                   \
        scope const char *name##_to_string(type i) {                    \
                if (i < 0 || i >= (type) ELEMENTSOF(name##_table))      \
                        return NULL;                                    \
                return name##_table[i];                                 \
        }                                                               \
        scope type name##_from_string(const char *s) {                  \
                type i;                                                 \
                assert(s);                                              \
                for (i = 0; i < (type)ELEMENTSOF(name##_table); i++)    \
                        if (name##_table[i] &&                          \
                            streq(name##_table[i], s))                  \
                                return i;                               \
                return (type) -1;                                       \
        }                                                               \
        struct __useless_struct_to_allow_trailing_semicolon__

#define DEFINE_STRING_TABLE_LOOKUP(name,type) __DEFINE_STRING_TABLE_LOOKUP(name,type,)
#define DEFINE_PRIVATE_STRING_TABLE_LOOKUP(name,type) __DEFINE_STRING_TABLE_LOOKUP(name,type,static)

/* For string conversions where numbers are also acceptable */
#define DEFINE_STRING_TABLE_LOOKUP_WITH_FALLBACK(name,type,max)         \
        int name##_to_string_alloc(type i, char **str) {                \
                char *s;                                                \
                int r;                                                  \
                if (i < 0 || i > max)                                   \
                        return -ERANGE;                                 \
                if (i < (type) ELEMENTSOF(name##_table)) {              \
                        s = strdup(name##_table[i]);                    \
                        if (!s)                                         \
                                return log_oom();                       \
                } else {                                                \
                        r = asprintf(&s, "%u", i);                      \
                        if (r < 0)                                      \
                                return log_oom();                       \
                }                                                       \
                *str = s;                                               \
                return 0;                                               \
        }                                                               \
        type name##_from_string(const char *s) {                        \
                type i;                                                 \
                unsigned u = 0;                                         \
                assert(s);                                              \
                for (i = 0; i < (type)ELEMENTSOF(name##_table); i++)    \
                        if (name##_table[i] &&                          \
                            streq(name##_table[i], s))                  \
                                return i;                               \
                if (safe_atou(s, &u) >= 0 && u <= max)                  \
                        return (type) u;                                \
                return (type) -1;                                       \
        }                                                               \
        struct __useless_struct_to_allow_trailing_semicolon__

int fd_nonblock(int fd, bool nonblock);
int fd_cloexec(int fd, bool cloexec);

int close_all_fds(const int except[], unsigned n_except);

bool fstype_is_network(const char *fstype);

int chvt(int vt);

int read_one_char(FILE *f, char *ret, usec_t timeout, bool *need_nl);
int ask(char *ret, const char *replies, const char *text, ...) _printf_attr_(3, 4);

int reset_terminal_fd(int fd, bool switch_to_text);
int reset_terminal(const char *name);

int open_terminal(const char *name, int mode);
int acquire_terminal(const char *name, bool fail, bool force, bool ignore_tiocstty_eperm, usec_t timeout);
int release_terminal(void);

int flush_fd(int fd);

int ignore_signals(int sig, ...);
int default_signals(int sig, ...);
int sigaction_many(const struct sigaction *sa, ...);

int close_pipe(int p[]);
int fopen_temporary(const char *path, FILE **_f, char **_temp_path);

ssize_t loop_read(int fd, void *buf, size_t nbytes, bool do_poll);
ssize_t loop_write(int fd, const void *buf, size_t nbytes, bool do_poll);

bool is_device_path(const char *path);

int dir_is_empty(const char *path);
char* dirname_malloc(const char *path);

void rename_process(const char name[8]);

void sigset_add_many(sigset_t *ss, ...);

bool hostname_is_set(void);

char* gethostname_malloc(void);
char* getlogname_malloc(void);
char* getusername_malloc(void);

int getttyname_malloc(int fd, char **r);
int getttyname_harder(int fd, char **r);

int get_ctty_devnr(pid_t pid, dev_t *d);
int get_ctty(pid_t, dev_t *_devnr, char **r);

int chmod_and_chown(const char *path, mode_t mode, uid_t uid, gid_t gid);
int fchmod_and_fchown(int fd, mode_t mode, uid_t uid, gid_t gid);

int rm_rf_children(int fd, bool only_dirs, bool honour_sticky, struct stat *root_dev);
int rm_rf_children_dangerous(int fd, bool only_dirs, bool honour_sticky, struct stat *root_dev);
int rm_rf(const char *path, bool only_dirs, bool delete_root, bool honour_sticky);
int rm_rf_dangerous(const char *path, bool only_dirs, bool delete_root, bool honour_sticky);

int pipe_eof(int fd);

cpu_set_t* cpu_set_malloc(unsigned *ncpus);

int status_vprintf(const char *status, bool ellipse, bool ephemeral, const char *format, va_list ap) _printf_attr_(4,0);
int status_printf(const char *status, bool ellipse, bool ephemeral, const char *format, ...) _printf_attr_(4,5);
int status_welcome(void);

int fd_columns(int fd);
unsigned columns(void);
int fd_lines(int fd);
unsigned lines(void);
void columns_lines_cache_reset(int _unused_ signum);

bool on_tty(void);

int running_in_chroot(void);

char *ellipsize(const char *s, size_t length, unsigned percent);
char *ellipsize_mem(const char *s, size_t old_length, size_t new_length, unsigned percent);

int touch(const char *path);

char *unquote(const char *s, const char *quotes);
char *normalize_env_assignment(const char *s);

int wait_for_terminate(pid_t pid, siginfo_t *status);
int wait_for_terminate_and_warn(const char *name, pid_t pid);

_noreturn_ void freeze(void);

bool null_or_empty(struct stat *st) _pure_;
int null_or_empty_path(const char *fn);

DIR *xopendirat(int dirfd, const char *name, int flags);

char *fstab_node_to_udev_node(const char *p);

char *resolve_dev_console(char **active);
bool tty_is_vc(const char *tty);
bool tty_is_vc_resolve(const char *tty);
bool tty_is_console(const char *tty) _pure_;
int vtnr_from_tty(const char *tty);
const char *default_term_for_tty(const char *tty);

void execute_directory(const char *directory, DIR *_d, char *argv[]);

int kill_and_sigcont(pid_t pid, int sig);

bool nulstr_contains(const char*nulstr, const char *needle);

bool plymouth_running(void);

bool hostname_is_valid(const char *s) _pure_;
char* hostname_cleanup(char *s, bool lowercase);

char* strshorten(char *s, size_t l);

int terminal_vhangup_fd(int fd);
int terminal_vhangup(const char *name);

int vt_disallocate(const char *name);

int copy_file(const char *from, const char *to);

int symlink_atomic(const char *from, const char *to);

int fchmod_umask(int fd, mode_t mode);

bool display_is_local(const char *display) _pure_;
int socket_from_display(const char *display, char **path);

int get_user_creds(const char **username, uid_t *uid, gid_t *gid, const char **home, const char **shell);
int get_group_creds(const char **groupname, gid_t *gid);

int in_gid(gid_t gid);
int in_group(const char *name);

char* uid_to_name(uid_t uid);
char* gid_to_name(gid_t gid);

int glob_exists(const char *path);

int dirent_ensure_type(DIR *d, struct dirent *de);

int in_search_path(const char *path, char **search);
int get_files_in_directory(const char *path, char ***list);

char *strjoin(const char *x, ...) _sentinel_;

bool is_main_thread(void);

bool in_charset(const char *s, const char* charset) _pure_;

int block_get_whole_disk(dev_t d, dev_t *ret);

int file_is_priv_sticky(const char *p);

int strdup_or_null(const char *a, char **b);

#define NULSTR_FOREACH(i, l)                                    \
        for ((i) = (l); (i) && *(i); (i) = strchr((i), 0)+1)

#define NULSTR_FOREACH_PAIR(i, j, l)                             \
        for ((i) = (l), (j) = strchr((i), 0)+1; (i) && *(i); (i) = strchr((j), 0)+1, (j) = *(i) ? strchr((i), 0)+1 : (i))

int ioprio_class_to_string_alloc(int i, char **s);
int ioprio_class_from_string(const char *s);

const char *sigchld_code_to_string(int i) _const_;
int sigchld_code_from_string(const char *s) _pure_;

int log_facility_unshifted_to_string_alloc(int i, char **s);
int log_facility_unshifted_from_string(const char *s);

int log_level_to_string_alloc(int i, char **s);
int log_level_from_string(const char *s);

int sched_policy_to_string_alloc(int i, char **s);
int sched_policy_from_string(const char *s);

const char *rlimit_to_string(int i) _const_;
int rlimit_from_string(const char *s) _pure_;

int ip_tos_to_string_alloc(int i, char **s);
int ip_tos_from_string(const char *s);

const char *signal_to_string(int i) _const_;
int signal_from_string(const char *s) _pure_;

int signal_from_string_try_harder(const char *s);

extern int saved_argc;
extern char **saved_argv;

bool kexec_loaded(void);

int prot_from_flags(int flags) _const_;

char *format_bytes(char *buf, size_t l, off_t t);

int fd_wait_for_event(int fd, int event, usec_t timeout);

void* memdup(const void *p, size_t l) _alloc_(2);

int is_kernel_thread(pid_t pid);

int fd_inc_sndbuf(int fd, size_t n);
int fd_inc_rcvbuf(int fd, size_t n);

int fork_agent(pid_t *pid, const int except[], unsigned n_except, const char *path, ...);

int setrlimit_closest(int resource, const struct rlimit *rlim);

int getenv_for_pid(pid_t pid, const char *field, char **_value);

bool is_valid_documentation_url(const char *url) _pure_;

bool in_initrd(void);

void warn_melody(void);

int get_home_dir(char **ret);

static inline void freep(void *p) {
        free(*(void**) p);
}

static inline void fclosep(FILE **f) {
        if (*f)
                fclose(*f);
}

static inline void pclosep(FILE **f) {
        if (*f)
                pclose(*f);
}

static inline void closep(int *fd) {
        if (*fd >= 0)
                close_nointr_nofail(*fd);
}

static inline void closedirp(DIR **d) {
        if (*d)
                closedir(*d);
}

static inline void umaskp(mode_t *u) {
        umask(*u);
}

#define _cleanup_free_ _cleanup_(freep)
#define _cleanup_fclose_ _cleanup_(fclosep)
#define _cleanup_pclose_ _cleanup_(pclosep)
#define _cleanup_close_ _cleanup_(closep)
#define _cleanup_closedir_ _cleanup_(closedirp)
#define _cleanup_umask_ _cleanup_(umaskp)
#define _cleanup_globfree_ _cleanup_(globfree)

_malloc_  _alloc_(1, 2) static inline void *malloc_multiply(size_t a, size_t b) {
        if (_unlikely_(b == 0 || a > ((size_t) -1) / b))
                return NULL;

        return malloc(a * b);
}

_alloc_(2, 3) static inline void *memdup_multiply(const void *p, size_t a, size_t b) {
        if (_unlikely_(b == 0 || a > ((size_t) -1) / b))
                return NULL;

        return memdup(p, a * b);
}

bool filename_is_safe(const char *p) _pure_;
bool path_is_safe(const char *p) _pure_;
bool string_is_safe(const char *p) _pure_;
bool string_has_cc(const char *p) _pure_;

void *xbsearch_r(const void *key, const void *base, size_t nmemb, size_t size,
                 int (*compar) (const void *, const void *, void *),
                 void *arg);

bool is_locale_utf8(void);

typedef enum DrawSpecialChar {
        DRAW_TREE_VERT,
        DRAW_TREE_BRANCH,
        DRAW_TREE_RIGHT,
        DRAW_TREE_SPACE,
        DRAW_TRIANGULAR_BULLET,
        _DRAW_SPECIAL_CHAR_MAX
} DrawSpecialChar;
const char *draw_special_char(DrawSpecialChar ch);

char *strreplace(const char *text, const char *old_string, const char *new_string);

char *strip_tab_ansi(char **p, size_t *l);

int on_ac_power(void);

int search_and_fopen(const char *path, const char *mode, const char **search, FILE **_f);
int search_and_fopen_nulstr(const char *path, const char *mode, const char *search, FILE **_f);
int create_tmp_dir(char template[], char** dir_name);

#define FOREACH_LINE(line, f, on_error)                         \
        for (;;)                                                \
                if (!fgets(line, sizeof(line), f)) {            \
                        if (ferror(f)) {                        \
                                on_error;                       \
                        }                                       \
                        break;                                  \
                } else

#define FOREACH_DIRENT(de, d, on_error)                                 \
        for (errno = 0, de = readdir(d);; errno = 0, de = readdir(d))   \
                if (!de) {                                              \
                        if (errno > 0) {                                \
                                on_error;                               \
                        }                                               \
                        break;                                          \
                } else if (ignore_file((de)->d_name))                   \
                        continue;                                       \
                else

static inline void *mempset(void *s, int c, size_t n) {
        memset(s, c, n);
        return (uint8_t*)s + n;
}

char *hexmem(const void *p, size_t l);
void *unhexmem(const char *p, size_t l);

char *strextend(char **x, ...) _sentinel_;
char *strrep(const char *s, unsigned n);

void* greedy_realloc(void **p, size_t *allocated, size_t need);
#define GREEDY_REALLOC(array, allocated, need) \
        greedy_realloc((void**) &(array), &(allocated), sizeof((array)[0]) * (need))

static inline void _reset_errno_(int *saved_errno) {
        errno = *saved_errno;
}

#define PROTECT_ERRNO _cleanup_(_reset_errno_) __attribute__((unused)) int _saved_errno_ = errno

struct _umask_struct_ {
        mode_t mask;
        bool quit;
};

static inline void _reset_umask_(struct _umask_struct_ *s) {
        umask(s->mask);
};

#define RUN_WITH_UMASK(mask)                                            \
        for (_cleanup_(_reset_umask_) struct _umask_struct_ _saved_umask_ = { umask(mask), false }; \
             !_saved_umask_.quit ;                                      \
             _saved_umask_.quit = true)

static inline unsigned u64log2(uint64_t n) {
        return (n > 1) ? __builtin_clzll(n) ^ 63U : 0;
}

static inline bool logind_running(void) {
        return access("/run/systemd/seats/", F_OK) >= 0;
}

#define DECIMAL_STR_WIDTH(x)                            \
        ({                                              \
                typeof(x) _x_ = (x);                    \
                unsigned ans = 1;                       \
                while (_x_ /= 10)                       \
                        ans++;                          \
                ans;                                    \
        })

int unlink_noerrno(const char *path);

#define alloca0(n)                                      \
        ({                                              \
                char *_new_;                            \
                size_t _len_ = n;                       \
                _new_ = alloca(_len_);                  \
                (void *) memset(_new_, 0, _len_);       \
        })

#define strappenda(a, b)                                \
        ({                                              \
                const char *_a_ = (a), *_b_ = (b);      \
                char *_c_;                              \
                size_t _x_, _y_;                        \
                _x_ = strlen(_a_);                      \
                _y_ = strlen(_b_);                      \
                _c_ = alloca(_x_ + _y_ + 1);            \
                strcpy(stpcpy(_c_, _a_), _b_);          \
                _c_;                                    \
        })

#define procfs_file_alloca(pid, field)                                  \
        ({                                                              \
                pid_t _pid_ = (pid);                                    \
                char *_r_;                                              \
                _r_ = alloca(sizeof("/proc/") -1 + DECIMAL_STR_MAX(pid_t) + 1 + sizeof(field)); \
                sprintf(_r_, "/proc/%lu/" field, (unsigned long) _pid_); \
                _r_;                                                    \
        })

struct _locale_struct_ {
        locale_t saved_locale;
        locale_t new_locale;
        bool quit;
};

static inline void _reset_locale_(struct _locale_struct_ *s) {
        PROTECT_ERRNO;
        if (s->saved_locale != (locale_t) 0)
                uselocale(s->saved_locale);
        if (s->new_locale != (locale_t) 0)
                freelocale(s->new_locale);
}

#define RUN_WITH_LOCALE(mask, loc) \
        for (_cleanup_(_reset_locale_) struct _locale_struct_ _saved_locale_ = { (locale_t) 0, (locale_t) 0, false }; \
             ({                                                         \
                     if (!_saved_locale_.quit) {                        \
                             PROTECT_ERRNO;                             \
                             _saved_locale_.new_locale = newlocale((mask), (loc), (locale_t) 0); \
                             if (_saved_locale_.new_locale != (locale_t) 0)     \
                                     _saved_locale_.saved_locale = uselocale(_saved_locale_.new_locale); \
                     }                                                  \
                     !_saved_locale_.quit; }) ;                         \
             _saved_locale_.quit = true)

bool id128_is_valid(const char *s) _pure_;
