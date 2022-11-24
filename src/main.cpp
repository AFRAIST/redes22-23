#include <cstdio>
#include "util/rcwg.hpp"
#include <arpa/inet.h>

int main() {
    std::printf("Hello World!\n");
    std::printf("%p\n", reinterpret_cast<void *>(inet_ntop));
}

