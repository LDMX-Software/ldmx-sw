/**
 * @file URLStreamer provides a utility function which can be used to load URLs 
 * including file://, http://, and https:// via an STL istream.
 */
#ifndef CONDITIONS_URLSTREAMER_H_
#define CONDITIONS_URLSTREAMER_H_

#include <istream>
#include <string>
#include <memory>

namespace conditions {
  /**
   * urlstream loads the defined url as an istream if possible.
   * Raises an exception if it is unable to open the file, URL, etc or if the url isn't understood
   */
  std::unique_ptr<std::istream> urlstream(const std::string& url);
}


#endif
