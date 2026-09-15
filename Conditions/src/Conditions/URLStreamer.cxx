#include "Conditions/URLStreamer.h"

#include <curl/curl.h>

#include <fstream>
#include <sstream>
#include <string>

#include "Framework/Exception/Exception.h"

namespace conditions {

static unsigned int http_requests = 0;
static unsigned int http_failures = 0;

void urlstatistics(unsigned int& requests, unsigned int& failures) {
  requests = http_requests;
  failures = http_failures;
}

/**
 * Callback for libcurl to write received data into a std::string buffer.
 */
static size_t writeCallback(char* ptr, size_t size, size_t nmemb,
                            void* userdata) {
  auto* buffer = static_cast<std::string*>(userdata);
  size_t total = size * nmemb;
  buffer->append(ptr, total);
  return total;
}

std::unique_ptr<std::istream> urlstream(const std::string& url) {
  if (url.find("file://") == 0 || (url.length() > 0 && url[0] == '/')) {
    std::string fname = url;
    if (fname.find("file://") == 0) fname = url.substr(7);
    auto fs = std::make_unique<std::ifstream>(fname);
    if (!fs->good()) {
      EXCEPTION_RAISE("ConditionsException",
                      "Unable to open CSV file '" + fname + "'");
    }
    return fs;
  }
  if ((url.find("http://") == 0) || (url.find("https://") == 0)) {
    http_requests++;

    CURL* curl = curl_easy_init();
    if (!curl) {
      http_failures++;
      EXCEPTION_RAISE("ConditionsException",
                      "Failed to initialize libcurl for URL '" + url + "'");
    }

    std::string response_body;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      curl_easy_cleanup(curl);
      http_failures++;
      EXCEPTION_RAISE("ConditionsException",
                      "Curl error (" + std::string(curl_easy_strerror(res)) +
                          ") retrieving URL '" + url + "'");
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (http_code != 200) {
      http_failures++;
      EXCEPTION_RAISE("ConditionsException",
                      "HTTP error " + std::to_string(http_code) +
                          " retrieving URL '" + url + "'");
    }

    auto ss = std::make_unique<std::stringstream>(std::move(response_body));
    return ss;
  }
  EXCEPTION_RAISE("ConditionsException", "Unable to handle URL '" + url + "'");
  return std::unique_ptr<std::istream>(nullptr);
}

}  // namespace conditions
