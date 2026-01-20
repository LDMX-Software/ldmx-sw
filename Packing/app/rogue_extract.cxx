/**
 * decoder for viewing raw data files in terminal
 */

#include <iostream>

#include "Packing/Utility/Reader.h"
#include "Packing/RogueFrame.h"

static void usage() {
  std::cout << "\n"
               " USAGE:\n"
               "  rogue-extract [options] input_file.dat\n"
               "\n"
               " OPTIONS:\n"
               "  -h,--help    : print this help and exit\n"
            << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    // can't do anything without any arguments
    usage();
    return 1;
  }

  std::string in_file;
  for (int i_arg{1}; i_arg < argc; i_arg++) {
    std::string arg{argv[i_arg]};
    if (arg[0] == '-') {
      // option
      if (arg == "-h" or arg == "--help") {
        usage();
        return 0;
      } else {
        std::cerr << "Unrecognized option " << arg << std::endl;
        return 1;
      }
    } else {
      if (not in_file.empty()) {
        std::cerr << "Can only decode one file at a time.\n";
        return 1;
      } else {
        in_file = arg;
      }
    }
  }

  if (in_file.empty()) {
    std::cerr << "Need to provide a file to decode.\n";
    usage();
    return 1;
  }

  packing::utility::Reader r(in_file);
  if (not r) {
    std::cerr << "Unable to open file '" << in_file << "'.\n";
    return 1;
  }

  packing::RogueFrame frame;
  int frame_count{0};
  try {
    while (r) {
      r >> frame;
      frame_count++;
      if (frame.channel() == 0) {
        printf("frame %d\n", frame_count);
        printf("  channel = %d, size = %ld\n", frame.channel(), frame.data().size());
        printf("  vers = %d, subsys = %d, contrib = %d, sentinel = %02x\n",
            frame.data().at(0), frame.data().at(1), frame.data().at(2), frame.data().at(3));
        const uint32_t* words = std::bit_cast<const uint32_t*>(frame.data().data());
        printf("--------\n");
        for (std::size_t i_word{0}; i_word < frame.data().size()/4; i_word++) {
          printf("%08x\n", words[i_word]);
        }
      }
    }
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}
