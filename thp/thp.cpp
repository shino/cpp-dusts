#include <sys/mman.h>
#include <cstring>
#include <chrono>
#include <thread>

#include <iostream>

#define KB 1024

int main(int argc, char** argv) {
  if(argc != 5) {
    std::cout << "Usage: thp <size-in-kb> <touch:never(0)/half(1)/full(2)/full-read(4)>" << std::endl;
    std::cout << "           <madv(1)-or-not(0)> <sleep-sec>" << std::endl;
    return EXIT_FAILURE;
  }
  uint64_t size_in_1kb = std::stoi(argv[1]);
  uint64_t madv_or_not = std::stoi(argv[2]);
  uint64_t how_touch = std::stoi(argv[2]);
  uint64_t sleep_sec = std::stoi(argv[3]);
  uint64_t len = size_in_1kb * KB;

  char* ptr = reinterpret_cast<char*>(
    ::mmap(nullptr, len,
           PROT_READ | PROT_WRITE,
           MAP_NORESERVE |
           MAP_ANONYMOUS |    // no backing files
           MAP_PRIVATE,       // private mapping, it will be CoW
           -1,  // file descriptor for anonymous mapping should be -1, always
           0));
  if (ptr == nullptr || ptr == MAP_FAILED) {
    int error_number = errno;
    std::cout << "mmap() failed. " << std::endl;
    std::cout << "[Errno " << error_number << "] " << std::strerror(error_number) << std::endl;
    return EXIT_FAILURE;
  }

  if (madv_or_not == 1) {
    std::cout << "About to madvice(MAD_HUGEPAGE)..." << std::endl;
    if (madvise(ptr, len, MADV_HUGEPAGE)) {
      int error_number = errno;
      std::cout << "madvise() failed. " << std::endl;
      std::cout << "[Errno " << error_number << "] " << std::strerror(error_number) << std::endl;
      return EXIT_FAILURE;
    }
  } else {
    std::cout << "Don't madvice(MAD_HUGEPAGE)." << std::endl;
  }

  uint64_t total = 0;
  if(how_touch == 0) {
    std::cout << "Don't touch the region." << std::endl;
  } else {
    if (how_touch == 1) {
      std::cout << "Touch first half of the region." << std::endl;
      for(size_t s=0; s < size_in_1kb/2; ++s) {
        ptr[s*KB] = 1;
      }
    } else if (how_touch == 2) {
      std::cout << "Touch full of the region." << std::endl;
      for(size_t s=0; s < size_in_1kb; ++s) {
        ptr[s*KB] = 1;
      }
    } else if (how_touch == 4) {
      std::cout << "Touch full by read of the region." << std::endl;
      for(size_t s=0; s < size_in_1kb; ++s) {
        total += ptr[s*KB];
      }
    } else {
      std::cout << "Invalid value for touch: " << how_touch << std::endl;
      return EXIT_FAILURE;
    }
  }    

  std::cout << "total (no specific meaning) : " << total << std::endl;
  std::chrono::seconds sleep_duration(sleep_sec);
  std::this_thread::sleep_for(sleep_duration);
  return EXIT_SUCCESS;
}
