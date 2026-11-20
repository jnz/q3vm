/* signal.h -- signals. The VM has none: signal and raise do nothing (see vmlib.c). */

#ifndef VM_SIGNAL_H
#define VM_SIGNAL_H

#define SIGINT  2
#define SIGTERM 15
#define SIGSEGV 11
#define SIGPIPE 13
#define SIG_DFL ((void (*)(int))0)
#define SIG_IGN ((void (*)(int))1)

typedef void (*sighandler_t)(int);

sighandler_t signal(int sig, sighandler_t handler);
int          raise(int sig);

#endif
