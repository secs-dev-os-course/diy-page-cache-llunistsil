#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>

#include "cache/cache_api.h"

const size_t fileSize = 256 * (1ULL << 20ULL) / sizeof(int64_t);
const int64_t GoalNum = 437;
const size_t BufferSize = fileSize / 8;
const int32_t seed = 364;
const size_t place = 986356;
int fd = -1;

void randomArrayEma() {
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<int64_t> dis(1, fileSize - 1);

  std::vector<int64_t> output(fileSize);

  for (size_t i = 0; i < fileSize; ++i) {
    output[i] = dis(gen);
  }
//    const size_t place = dis(gen);
  output[place] = GoalNum;

  std::ofstream out_file("input.bin", std::ios::binary | std::ios::trunc); // NOLINT
  if (!out_file) {
    throw std::runtime_error("Failed to open output file");
  }
  out_file.write(reinterpret_cast<const char *>(output.data()), sizeof(int64_t) * fileSize); // NOLINT
  out_file.close();
  std::cout << "Position: " << place << "\n\n";
}

int emaSearchInt() {
  int fd = lab2_open("input.bin");
  if (fd == -1) {
    throw std::runtime_error("Cannot open input file");
  }

  size_t pos = 0;
  size_t block = 0;
  std::vector<int64_t> buffer(BufferSize);
  int res;

  while ((res = lab2_read(fd, buffer.data(), BufferSize * sizeof(int64_t))) == 0) {
    size_t num_read = BufferSize;

    for (size_t i = 0; i < num_read; i++) {
      if (buffer[i] == GoalNum) {
        pos = BufferSize * block + i;
        break;
      }
    }
    block++;

    if (pos != 0) {
      break;
    }
    lab2_lseek(fd, block * BufferSize * sizeof(int64_t), SEEK_SET);
  }
  lab2_close(fd);

  std::cout << "Found position: " << pos << "\n" << "With cache, ";
  return pos;
}

int emaSearchIntNoCache() {
    std::ifstream file("input.bin", std::ios::binary);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open input file");
    }

    size_t pos = 0;
    size_t block = 0;
    std::vector<int64_t> buffer(BufferSize);

    while (file.read(reinterpret_cast<char*>(buffer.data()), buffer.size() * sizeof(int64_t))) {
      size_t num_read = file.gcount() / sizeof(int64_t);
      if (num_read == 0) {
        break;
      }
      for (size_t i = 0; i < num_read; i++) {
        if (buffer[i] == GoalNum) {
          pos = BufferSize * block + i;
          break;
        }
      }
      block++;
    }
    std::cout << "Found position: " << pos << "\n" << "No cache, ";
    return pos;
}

int main() {
  randomArrayEma();
  auto start_time = std::chrono::high_resolution_clock::now();
  emaSearchIntNoCache();
  auto end_time = std::chrono::high_resolution_clock::now();
  double exec_time =  std::chrono::duration<double>(end_time - start_time).count();
  std::cout << "Time: " << exec_time << " s\n\n";

  auto start_time_2 = std::chrono::high_resolution_clock::now();
  emaSearchInt();
  auto end_time_2 = std::chrono::high_resolution_clock::now();
  double exec_time_2 =  std::chrono::duration<double>(end_time_2 - start_time_2).count();
  std::cout << "Time: " << exec_time_2 << " s\n";

  return 0;
}
