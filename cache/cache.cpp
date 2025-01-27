#include "cache.h"
#include <iostream>
#include <cstring>

Cache::Block::Block(size_t blockSize)
    : data(new char[blockSize]), frequency(0), dirty(false) {}

Cache::Cache(size_t blockSize, size_t cacheSize)
    : blockSize(blockSize), cacheSize(cacheSize) {}

size_t Cache::getLFUBlockIndex(const HANDLE &fileHandle) {
  size_t minFrequency = SIZE_MAX;
  size_t blockIndex = SIZE_MAX;

  for (const auto &[index, block] : fileCacheMap[fileHandle]) {
    if (block.frequency < minFrequency) {
      minFrequency = block.frequency;
      blockIndex = index;
    }
  }

  return blockIndex;
}

void Cache::writeBlockToDisk(const HANDLE &fileHandle, size_t blockIndex) {
  auto &block = fileCacheMap[fileHandle][blockIndex];
  if (block.dirty) {
    LARGE_INTEGER offset;
    offset.QuadPart = blockIndex * blockSize;

    SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

    DWORD bytesWritten;
    if (!WriteFile(fileHandle, block.data.get(), blockSize, &bytesWritten, nullptr)) {
      std::cerr << "Failed to write block to disk. Error: " << GetLastError() << std::endl;
    }
    block.dirty = false;
  }
}

void Cache::writeBlock(HANDLE fileHandle, size_t blockIndex, const char *data, size_t size) {
  if (fileCacheMap[fileHandle].size() >= cacheSize) {
    size_t lfuIndex = getLFUBlockIndex(fileHandle);
    writeBlockToDisk(fileHandle, lfuIndex);
    fileCacheMap[fileHandle].erase(lfuIndex);
  }

  auto &block = fileCacheMap[fileHandle][blockIndex];
  if (!block.data) {
    block = Block(blockSize);
  }

  std::memcpy(block.data.get(), data, size);
  block.frequency++;
  block.dirty = true;
}

void Cache::readBlock(HANDLE fileHandle, size_t blockIndex, char *buffer, size_t size) {
  if (fileCacheMap[fileHandle].find(blockIndex) != fileCacheMap[fileHandle].end()) {
    auto &block = fileCacheMap[fileHandle][blockIndex];
    std::memcpy(buffer, block.data.get(), size);
    block.frequency++;
  } else {
    LARGE_INTEGER offset;
    offset.QuadPart = blockIndex * blockSize;

    SetFilePointerEx(fileHandle, offset, nullptr, FILE_BEGIN);

    DWORD bytesRead;
    if (!ReadFile(fileHandle, buffer, size, &bytesRead, nullptr)) {
      std::cerr << "Failed to read block from disk. Error: " << GetLastError() << std::endl;
    }

    writeBlock(fileHandle, blockIndex, buffer, bytesRead);
  }
}

void Cache::flushToDisk(HANDLE fileHandle) {
  for (auto &[blockIndex, block] : fileCacheMap[fileHandle]) {
    writeBlockToDisk(fileHandle, blockIndex);
  }
}

void Cache::closeFile(HANDLE fileHandle) {
  flushToDisk(fileHandle);
  fileCacheMap.erase(fileHandle);
  CloseHandle(fileHandle);
}
