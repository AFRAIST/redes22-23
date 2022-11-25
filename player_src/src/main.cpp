#include "util/rcwg.hpp"
#include <arpa/inet.h>
#include <cstdio>

int main() {
    std::printf("Hello World from client!\n");
    std::printf("%p\n", reinterpret_cast<void *>(inet_ntop));
}
