/**
 * decoder for viewing raw data files in terminal
 */

#include <iostream>

#include "Packing/LDMXRoRHeader.h"
#include "Packing/RogueFrameHeader.h"
#include "Packing/Utility/Reader.h"

static void usage() {
  std::cout << "\n"
               " USAGE:\n"
               "  rogue-extract [options] input_file.dat\n"
               "\n"
               " OPTIONS:\n"
               "  -h,--help      : print this help and exit\n"
               "  -n,--nevent    : maximum number of events to unpack\n"
               "  -s,--subsystem : which subsystem\n                   "
               "('ecal','hcal','ts','tdaq', 'tracker', or integer)\n"
            << std::endl;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    // can't do anything without any arguments
    usage();
    return 1;
  }

  int nevents{-1};
  std::string in_file;
  int subsystem{-1}, contributor{-1};
  for (int i_arg{1}; i_arg < argc; i_arg++) {
    std::string arg{argv[i_arg]};
    if (arg[0] == '-') {
      // option
      if (arg == "-h" or arg == "--help") {
        usage();
        return 0;
      } else if (arg == "-n" or arg == "--nevent") {
        if (i_arg + 1 == argc or argv[i_arg + 1][0] == '-') {
          std::cerr << "The " << arg
                    << " parameter requires an argument after it.\n";
          return 1;
        }
        i_arg++;
        try {
          nevents = std::stoi(argv[i_arg]);
        } catch (const std::invalid_argument& e) {
          std::cerr << "The argument to " << arg << " '" << argv[i_arg]
                    << "' is not an integer.\n";
          return 1;
        }
      } else if (arg == "-s" or arg == "--subsystem") {
        if (i_arg + 1 == argc or argv[i_arg + 1][0] == '-') {
          std::cerr << "The " << arg
                    << " parameter requires an argument after it.\n";
          return 1;
        }
        i_arg++;
        std::string subsys_arg{argv[i_arg]};
        auto [subsys, contrib] = packing::LDMXRoRHeader::subsystem(subsys_arg);
        if (subsys == -1) {
          try {
            subsystem = std::stoi(subsys_arg.c_str());
          } catch (const std::invalid_argument& e) {
            std::cerr << "The argument to " << arg << " '" << subsys_arg
                      << "' is not 'ecal', 'hcal', 'ts', 'tdaq', 'tracker'"
                      << " or a positive integer.\n";
            return 1;
          }
        } else {
          subsystem = subsys;
          contributor = contrib;
        }
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
  std::vector<uint8_t> bytes;
  int frame_count{0};
  int event_count{0};
  try {
    while (r) {
      r >> frame_header;
      frame_count++;
      const int frame_end = r.tell() + frame_header.size();
      if (frame_header.channel() != 0) {
        // non-data channel in StreamWriter, skip
        r.seek(frame_end);
        continue;
      }

      if (frame_header.probablyYaml()) {
        // non-data channel in StreamWriter, skip
        r.seek(frame_end);
        continue;
      }

      r >> ror_header;
      if (ror_header.subsystem() != subsystem) {
        // wrong subsystem ID number
        r.seek(frame_end);
        continue;
      }

      if (contributor >= 0 and ror_header.contributor() != contributor) {
        // wrong contributor ID number
        r.seek(frame_end);
        continue;
      }

      // correct subsystem and contributor channel
      event_count++;
      printf("frame %d\n", frame_count);
      printf("  channel = %d, size = %d, trailer = 0x%02x\n",
             frame_header.channel(), frame_header.size(),
             frame_header.trailer());
      printf("  vers = %d, subsys = %d, contrib = %d\n", ror_header.version(),
             ror_header.subsystem(), ror_header.contributor());

      r.read(bytes, (frame_header.size() - packing::LDMXRoRHeader::SIZE));

      // print similar to hexdump -C
      for (std::size_t i_row{0}; i_row < bytes.size() / 16; i_row++) {
        printf("%8lu  ", i_row);
        for (std::size_t i_byte{0}; i_byte < 16; i_byte++) {
          printf("%02x ", bytes[16 * i_row + i_byte]);
        }
        printf("\n");
      }

      // leave early if reached number of events
      if (nevents > 0 and event_count >= nevents) {
        return 0;
      }
    }
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << "\n";
    return 1;
  }

  return 0;
}
