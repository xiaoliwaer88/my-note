#include <errno.h>
#include <stdarg.h>
#include <stdio.h> // 标准 I/O（可选）
#include <string.h>
#include <sys/types.h>
#include <syslog.h>
#include <syslog.h> // syslog 相关函数
#include <time.h>
#include <unistd.h> // 进程相关（可选）

#define errno_s strerror(errno)

#define _POSIX_C_SOURCE 199309L

// Function declaration (prototype)
void fatal(const char *fmt, ...);

struct wlog {
  int prio;
  u_int16_t wid;
  size_t loglen;
  char logmsg[];
};
// gcc 像解释printf一样占位符报错
static void log_print(int, const char *, ...)
    __attribute__((format(printf, 2, 3)));

static FILE *fp = NULL;

void kore_log_init(void) {
  const char *name = "demo2";
  fp = stdout;
  openlog(name, LOG_NDELAY | LOG_PID, LOG_DAEMON);
}

void log_set_file(const char *path) {
  if ((fp = fopen(path, "a")) == NULL) {
    fp = stdout;
    fatal("fopen(%s): %s", path, errno_s);
  }
}

void kore_log(int prio, const char *fmt, ...) {
  va_list args;
  const char *str;
  struct wlog wlog;
  struct kore_buf buf, pkt;

  kore_buf_init(&buf, 128);

  va_start(args, fmt);
  kore_buf_appendv(&buf, fmt, args);
  va_end(args);

  if (worker != NULL) {
    kore_buf_init(&pkt, sizeof(wlog) + buf.offset);

    memset(&wlog, 0, sizeof(wlog));

    wlog.prio = prio;
    wlog.wid = worker->id;
    wlog.loglen = buf.offset;

    kore_buf_append(&pkt, &wlog, sizeof(wlog));
    kore_buf_append(&pkt, buf.data, buf.offset);

    kore_msg_send(KORE_MSG_PARENT, KORE_MSG_WORKER_LOG, pkt.data, pkt.offset);

    kore_buf_cleanup(&pkt);
  } else {
    str = kore_buf_stringify(&buf, NULL);

    if (kore_foreground || fp != stdout)
      log_print(prio, "proc=[parent] log=[%s]\n", str);
    else
      syslog(prio, "proc=[parent] log=[%s]", str);
  }

  kore_buf_cleanup(&buf);
}

static void log_print(int prio, const char *fmt, ...) {
  struct tm *t;
  struct timespec ts;
  va_list args;
  char tbuf[32];

  va_start(args, fmt);

  switch (prio) {
  case LOG_ERR:
  case LOG_WARNING:
  case LOG_NOTICE:
  case LOG_INFO:
  case LOG_DEBUG:
    break;
  }

  (void)clock_gettime(CLOCK_REALTIME, &ts);
  t = gmtime(&ts.tv_sec);

  if (strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", t) > 0)
    fprintf(fp, "%s.%03ld UTC ", tbuf, ts.tv_nsec / 1000000);

  vfprintf(fp, fmt, args);
  fflush(fp);

  va_end(args);
}
