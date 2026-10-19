#ifndef IOCTL_WRAPPER_H
#define IOCTL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

int ioctl(int fd, unsigned long op, void *argp);
int real_ioctl(int fd, unsigned long op, void *argp);

#ifdef __cplusplus
}
#endif

#endif