/**
 * decoder for viewing raw data files in terminal
 */

#include <iostream>

#include "Packing/Utility/Reader.h"
#include "Packing/RogueFrameHeader.h"
#include "Packing/LDMXRoRHeader.h"

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

  packing::RogueFrameHeader frame_header;
  packing::LDMXRoRHeader ror_header;
  std::vector<uint32_t> words;
  int frame_count{0};
  try {
    while (r) {
      r >> frame_header;
      frame_count++;
      if (frame_header.channel() == 0 and not frame_header.probablyYaml()) {
        printf("frame %d\n", frame_count);
        printf("  channel = %d, size = %d, trailer = 0x%02x\n",
            frame_header.channel(), frame_header.size(), frame_header.trailer());
        r >> ror_header;
        printf("  vers = %d, subsys = %d, contrib = %d\n",
            ror_header.version(), ror_header.subsystem(), ror_header.contributor());
        r.read(words, (frame_header.size() - packing::LDMXRoRHeader::size)/4);
        printf("--------\n");
        for (std::size_t i_word{0}; i_word < words.size(); i_word++) {
          printf("%08x\n", words[i_word]);
        }
      } else {
        // skip this frame
        r.seek(r.tell()+frame_header.size());
      }
    }
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}
