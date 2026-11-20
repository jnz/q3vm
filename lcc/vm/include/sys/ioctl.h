/* sys/ioctl.h -- terminal control. The VM has no terminal: ioctl fails (see vmlib.c). */

#ifndef VM_SYS_IOCTL_H
#define VM_SYS_IOCTL_H

#define TIOCGWINSZ 0x5413

struct winsize
{
    unsigned short ws_row;
    unsigned short ws_col;
    unsigned short ws_xpixel;
    unsigned short ws_ypixel;
};

int ioctl(int fd, unsigned long request, void* arg);

#endif
