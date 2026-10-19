/*
 * run.h - helpers for the tests that  make test  runs, with Acutest.
 *
 * There is one test program per topic, run inside that topic's directory.
 * Each test runs one example program and checks that it does what its
 * header comment says:
 *
 *   run("./prog args")      run it (at most 30 s) with stdout a pipe and
 *                           keep its output and exit status
 *   expect(re)              a line of the output matches re (POSIX ERE)
 *   reject(re)              no line of the output matches re
 *   expect_count(n, re)     exactly n lines match re
 *   expect_values(re, v)    re has one group; on the lines that match re,
 *                           the groups, joined by spaces, are v
 *   expect_status(n)        the exit status is n (128 + s: killed by
 *                           signal s)
 *   expect_no_leftovers()   no object named k22_* is left in /dev/shm
 *   values(re)              the string that expect_values() compares
 *
 * When a check fails, the program's output is printed with the failure.
 */
#ifndef RUN_H
#define RUN_H

#define TEST_MSG_MAXSIZE 8192
#include "acutest.h"

#include <dirent.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

static char ran[256];           /* the command line */
static char out[65536];         /* its output, stdout and stderr */
static int status;              /* its exit status */

static inline void run(const char *cmd)
{
    char line[512];

    snprintf(ran, sizeof(ran), "%s", cmd);
    snprintf(line, sizeof(line), "timeout 30 %s 2>&1 < /dev/null", cmd);
    FILE *p = popen(line, "r");
    TEST_ASSERT_(p != NULL, "popen(\"%s\")", cmd);
    size_t n = fread(out, 1, sizeof(out) - 1, p);
    out[n] = '\0';
    int st = pclose(p);
    status = WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
}

/*
 * Find the lines of the output that match re. Return how many there are;
 * if vals is not NULL, also store their first groups there, joined by
 * spaces.
 */
static inline int match_lines(const char *re, char *vals, size_t size)
{
    regex_t r;
    regmatch_t m[2];
    int count = 0;

    TEST_ASSERT_(regcomp(&r, re, REG_EXTENDED) == 0, "bad regex %s", re);
    if (vals)
        vals[0] = '\0';
    for (char *s = out; *s; ) {
        const char *end = strchr(s, '\n');
        size_t len = end ? (size_t) (end - s) : strlen(s);
        char line[1024];
        snprintf(line, sizeof(line), "%.*s", (int) len, s);
        if (regexec(&r, line, 2, m, 0) == 0) {
            count++;
            if (vals && m[1].rm_so >= 0)
                snprintf(vals + strlen(vals), size - strlen(vals), "%s%.*s",
                         count > 1 ? " " : "",
                         (int) (m[1].rm_eo - m[1].rm_so), line + m[1].rm_so);
        }
        s += end ? len + 1 : len;
    }
    regfree(&r);
    return count;
}

static inline const char *values(const char *re)
{
    static char vals[4096];

    match_lines(re, vals, sizeof(vals));
    return vals;
}

/* Shown only if the check just before it failed. */
#define SHOW_OUTPUT()   TEST_MSG("output of %s:\n%s", ran, out)

#define expect(re) do {                                                     \
        TEST_CHECK_(match_lines(re, NULL, 0) > 0,                           \
                    "%s: a line matches %s", ran, re);                      \
        SHOW_OUTPUT();                                                      \
    } while (0)

#define reject(re) do {                                                     \
        TEST_CHECK_(match_lines(re, NULL, 0) == 0,                          \
                    "%s: no line matches %s", ran, re);                     \
        SHOW_OUTPUT();                                                      \
    } while (0)

#define expect_count(n, re) do {                                            \
        int found_ = match_lines(re, NULL, 0);                              \
        TEST_CHECK_(found_ == (n), "%s: %d lines match %s (found %d)",      \
                    ran, (n), re, found_);                                  \
        SHOW_OUTPUT();                                                      \
    } while (0)

#define expect_values(re, v) do {                                           \
        const char *found_ = values(re);                                    \
        TEST_CHECK_(strcmp(found_, (v)) == 0,                               \
                    "%s: values of %s are \"%s\" (found \"%s\")",           \
                    ran, re, (v), found_);                                  \
        SHOW_OUTPUT();                                                      \
    } while (0)

#define expect_status(n) do {                                               \
        TEST_CHECK_(status == (n), "%s: exit status %d (found %d)",         \
                    ran, (n), status);                                      \
        SHOW_OUTPUT();                                                      \
    } while (0)

/* The names in /dev/shm that contain k22_, joined by spaces. */
static inline const char *shm_leftovers(void)
{
    static char names[1024];
    DIR *d = opendir("/dev/shm");
    struct dirent *e;

    names[0] = '\0';
    while (d && (e = readdir(d)) != NULL)
        if (strstr(e->d_name, "k22_"))
            snprintf(names + strlen(names), sizeof(names) - strlen(names),
                     " %s", e->d_name);
    if (d)
        closedir(d);
    return names;
}

#define expect_no_leftovers() do {                                          \
        const char *left_ = shm_leftovers();                                \
        TEST_CHECK_(left_[0] == '\0', "%s: nothing named k22_* is left "    \
                    "in /dev/shm (found:%s)", ran, left_);                  \
    } while (0)

#endif
