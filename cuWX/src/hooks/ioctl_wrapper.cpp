#include <ioctl_wrapper.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <hexdump.h>
#include <nv_uvm_ioctl.h>
#include <nv_esc_ioctl.h>
#include <unistd.h>
#include <log.h>
#include <allocations.h>
#include <map>
#include <cstring>

std::map<int, const char*> allowed_fds;
std::map<int, const char*> blocked_fds;

int real_ioctl(int fd, unsigned long op, void *argp) {
    static int (*ioctl_func)(int, unsigned long, void *) = NULL;
    if (!ioctl_func)
    {
        ioctl_func = (int (*)(int, unsigned long, void *))dlsym(RTLD_NEXT, "ioctl");
    }
    return ioctl_func(fd, op, argp);
}

int ioctl(int fd, unsigned long op, void *argp)
{
    char *dev_name = nullptr;
    bool allowed = false;

    // This is needed in environments in which other libraries also use ioctl, such as when using cuda-gdb.
    // We want to avoid logging IOCTL calls from other libraries, so we keep track of which fds are associated with nvidia devices and which are not.
    if (allowed_fds.find(fd) != allowed_fds.end()) {
        dev_name = (char*)allowed_fds[fd];
        allowed = true;
    } else if (blocked_fds.find(fd) != blocked_fds.end()) {
        dev_name = (char*)blocked_fds[fd];
    } else {
        // get file name from fd
        char path[100];
        char link[100];
        snprintf(link, 100, "/proc/self/fd/%d", fd);
        size_t len = readlink(link, path, 100);
        path[len] = '\0';

        dev_name = new char[strlen(path) + 1];
        strcpy(dev_name, path);

        if(strstr(dev_name, "nvidia")) {
            // LOG_VERBOSE("Found new nvidia IOCTL fd: %d -> %s\n", fd, dev_name);
            allowed_fds[fd] = dev_name;
            allowed = true;
        } else {
            // LOG_VERBOSE("Found new external IOCTL fd: %d -> %s\n", fd, dev_name);
            blocked_fds[fd] = dev_name;
        }
    }

    if(!allowed) {
        // LOG_VERBOSE("IOCTL on external device %s, skipping it\n", dev_name);
        return real_ioctl(fd, op, argp);
    }

    uint8_t nr = op & (1 << 8) - 1;
    uint8_t type = (op >> 8) & (1 << 8) - 1;
    uint16_t size = (op >> 16) & (1 << 14) - 1;
    uint8_t dir = (op >> 30) & (1 << 2) - 1;

    int ret = real_ioctl(fd, op, argp);

    allocations_process_ioctl(fd, nr, size, argp);

    // if(
    //     !print_uvm_call(nr, size, argp)
    //     && !print_ctl_call(nr, size, argp)
    // ) {
    //     LOG("IOCTL on %16s\top: 0x%08lx, nr: %03u, type: %03u, size: %03u, dir: %s\n", dev_name, op, nr, type, size, dir == 0 ? "none" : dir == 1 ? "write" : dir == 2   ? "read" : "read|write");
    //     hexdump(argp, size);
    // }

    return ret;
}