#include "cache_api.h"
#include "cache.h"
#include <iostream>
#include <unordered_map>
#include <filesystem>
#include <algorithm>


const size_t blockSize = 256 * 1 << 10;
static Cache cache(blockSize, 1024);

// Таблица для маппинга fd -> HANDLE
static std::unordered_map<int, HANDLE> fdToHandleMap;
static int nextFd = 3;
size_t currentOffset = 0;
char tempBuffer[256 * 1 << 10];

int lab2_open(const char *path) {
  HANDLE fileHandle = CreateFile(
      path,
      GENERIC_READ | GENERIC_WRITE,
      0,
      nullptr,
      OPEN_EXISTING,
      FILE_FLAG_NO_BUFFERING,
      nullptr
  );

  if (fileHandle == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open file. Error: " << GetLastError() << std::endl;
    return -1;
  }

  int fd = nextFd++;
  fdToHandleMap[fd] = fileHandle;

  return fd;
}

int lab2_close(int fd) {
  if (fdToHandleMap.find(fd) == fdToHandleMap.end()) {
    std::cerr << "Invalid file descriptor: " << fd << std::endl;
    return -1;
  }

  HANDLE fileHandle = fdToHandleMap[fd];
  cache.closeFile(fileHandle);
  fdToHandleMap.erase(fd);

  return 0;
}

int lab2_write(int fd, const void *buf, size_t count) {
  if (fdToHandleMap.find(fd) == fdToHandleMap.end()) {
    std::cerr << "Invalid file descriptor: " << fd << std::endl;
    return -1;
  }

  HANDLE fileHandle = fdToHandleMap[fd];

  size_t bytesToWrite = count;
  size_t blockIndex = currentOffset / blockSize;
  size_t blockOffset = currentOffset % blockSize;

  const char *data = static_cast<const char *>(buf);

  while (bytesToWrite > 0) {
    size_t bytesToBlock = (bytesToWrite < blockSize - blockOffset) ? bytesToWrite : blockSize - blockOffset;
    char tempBuffer[blockSize];
    cache.readBlock(fileHandle, blockIndex, tempBuffer, blockSize);
    std::memcpy(tempBuffer + blockOffset, data, bytesToBlock);
    cache.writeBlock(fileHandle, blockIndex, tempBuffer, blockSize);

    blockIndex++;
    blockOffset = 0;
    data += bytesToBlock;
    bytesToWrite -= bytesToBlock;
  }

  currentOffset += count;
  return count;
}

int lab2_read(int fd, void *buf, size_t count) {
  if (fdToHandleMap.find(fd) == fdToHandleMap.end()) {
    std::cerr << "Invalid file descriptor: " << fd << std::endl;
    return -1;
  }

  HANDLE fileHandle = fdToHandleMap[fd];

  size_t bytesToRead = count;
  size_t blockIndex = currentOffset / blockSize;
  size_t blockOffset = currentOffset % blockSize;

  while (bytesToRead > 0) {
    size_t bytesFromBlock = (bytesToRead < blockSize - blockOffset) ? bytesToRead : blockSize - blockOffset;
    cache.readBlock(fileHandle, blockIndex, tempBuffer, blockSize);
    std::memcpy(static_cast<char *>(buf) + (count - bytesToRead), tempBuffer + blockOffset, bytesFromBlock);

    blockIndex++;
    blockOffset = 0;
    bytesToRead -= bytesFromBlock;
  }

  currentOffset += count;
  return 0;
}

off_t lab2_lseek(int fd, off_t offset, int whence) {
  if (fdToHandleMap.find(fd) == fdToHandleMap.end()) {
    std::cerr << "Invalid file descriptor: " << fd << std::endl;
    return -1;
  }

  HANDLE fileHandle = fdToHandleMap[fd];
  size_t newOffset = 0;

  switch (whence) {
  case SEEK_SET:
    newOffset = offset;
    break;
  case SEEK_CUR:
    newOffset = currentOffset + offset;
    break;
  case SEEK_END:
    newOffset = currentOffset + offset;
    LARGE_INTEGER move;
    LARGE_INTEGER newPointer;
    move.QuadPart = 0;
        if (!SetFilePointerEx(fileHandle, move, &newPointer, FILE_END)) {
          std::cerr << "Failed to lseek. Error: " << GetLastError() << std::endl;
          return -1;
        }
    break;
  default:
    std::cerr << "Invalid whence value" << std::endl;
    return -1;
  }

  currentOffset = newOffset;

  return currentOffset;
}

int lab2_fsync(int fd) {
  if (fdToHandleMap.find(fd) == fdToHandleMap.end()) {
    std::cerr << "Invalid file descriptor: " << fd << std::endl;
    return -1;
  }

  HANDLE fileHandle = fdToHandleMap[fd];
  cache.flushToDisk(fileHandle);

  if (!FlushFileBuffers(fileHandle)) {
    std::cerr << "Failed to fsync. Error: " << GetLastError() << std::endl;
    return -1;
  }

  return 0;
}
