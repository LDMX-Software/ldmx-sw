#ifndef PACKING_RAWDATAFILE_STREAM_H_
#define PACKING_RAWDATAFILE_STREAM_H_

#include <cstddef>
#include <istream>
#include <ostream>

namespace packing {
namespace rawdatafile {

typedef std::basic_istream<std::byte> istream;
typedef std::basic_ostream<std::byte> ostream;

}
}

#endif  // PACKING_RAWDATAFILE_STREAM_H_
