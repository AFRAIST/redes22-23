#include "rcwg.hpp"
#include <arpa/inet.h>
#include <cstdio>

namespace rcwg {
ALWAYS_INLINE void Main(int argc, char *argv[]) {
    RCWG_UNUSED(argc, argv);
    util::io::PrintF("Hello World from client!\n");
    util::io::SetVerbose();
    util::io::VerbosePrintF("aaaa\n");
    util::io::PrintF("bbba\n");
}
} // namespace rcwg

int main(int argc, char *argv[]) { rcwg::Main(argc, argv); }
