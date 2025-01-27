#ifndef CACHE_H
#define CACHE_H

#include <windows.h>
#include <unordered_map>
#include <memory>
#include <mutex>

class Cache {
private:
  struct Block {
    std::unique_ptr<char[]> data;
    size_t frequency;
    bool dirty;

    Block(size_t blockSize);
    Block() : data(nullptr), frequency(0), dirty(false) {}
  };

  size_t blockSize;
  size_t cacheSize;
  std::unordered_map<HANDLE, std::unordered_map<size_t, Block>> fileCacheMap;

  size_t getLFUBlockIndex(const HANDLE &fileHandle);
  void writeBlockToDisk(const HANDLE &fileHandle, size_t blockIndex);

public:
  Cache(size_t blockSize, size_t cacheSize);

  void writeBlock(HANDLE fileHandle, size_t blockIndex, const char *data, size_t size);
  void readBlock(HANDLE fileHandle, size_t blockIndex, char *buffer, size_t size);
  void flushToDisk(HANDLE fileHandle);
  void closeFile(HANDLE fileHandle);
};

#endif // CACHE_H
