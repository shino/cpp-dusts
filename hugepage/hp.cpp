#include <sys/mman.h>
#include <cstring>
#include <chrono>
#include <thread>

#include <iostream>

#ifndef MAP_HUGE_SHIFT
#define MAP_HUGE_SHIFT  26
#endif

#ifndef MAP_HUGE_2MB
#define MAP_HUGE_2MB (21 << MAP_HUGE_SHIFT)
#endif

#define TWO_MB 2*1024*1024

int main(int argc, char** argv) {
  if(argc != 3) {
    std::cout << "Usage: hp <size-in-2mb>" << std::endl;
    return EXIT_FAILURE;
  }
  uint64_t size_in_2mb = std::stoi(argv[1]);
  uint64_t sleep_sec = std::stoi(argv[2]);
  int pagesize_mask = MAP_HUGE_2MB | MAP_HUGETLB;
  
  char* ret = reinterpret_cast<char*>(
    ::mmap(nullptr,
           size_in_2mb*TWO_MB,
           PROT_READ | PROT_WRITE,
           MAP_ANONYMOUS |    // no backing files
           MAP_PRIVATE |      // private mapping, it will be CoW
           pagesize_mask,
           -1,  // file descriptor for anonymous mapping should be -1, always
           0));
  if (ret == nullptr || ret == MAP_FAILED) {
    int error_number = errno;
    std::cout << "mmap() failed. " << std::endl;
    std::cout << "[Errno " << error_number << "] " << std::strerror(error_number) << std::endl;
    return EXIT_FAILURE;
  }

  for(size_t s=0; s < size_in_2mb; ++s) {
    ret[s*TWO_MB] = 1;
  }

  std::chrono::seconds sleep_duration(sleep_sec);
  std::this_thread::sleep_for(sleep_duration);
  return EXIT_SUCCESS;
}
