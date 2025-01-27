#pragma once

#ifdef CACHE_EXPORTS
#define CACHEAPI __declspec(dllexport)
#else
#define CACHEAPI __declspec(dllimport)
#endif

#include "cache_api.h"
#include <cstddef>
#include <sys/types.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

CACHEAPI int lab2_open(const char *path);
CACHEAPI int lab2_close(int fd);
CACHEAPI int lab2_write(int fd, const void *buf, size_t count);
CACHEAPI int lab2_read(int fd, void *buf, size_t count);
CACHEAPI off_t lab2_lseek(int fd, off_t offset, int whence);
CACHEAPI int lab2_fsync(int fd);

#ifdef __cplusplus
}
#endif
