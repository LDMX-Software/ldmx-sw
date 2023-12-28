#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/// @brief Invokes addr2line utility to determine the function name
/// and the line information from an address in the code segment.
static char *addr2line(const char *image, void *addr, bool color_output) {
  static char exename[4096] = {0};
  static char result[4096] = {0};

  if (exename[0] == 0) {
    int ret = readlink("/proc/self/exe", exename, 4095);
    if (ret == -1) {
      exename[0] = 0;
      return result;
    }
    exename[ret] = 0;
  }

  int pipefd[2];
  if (pipe(pipefd) != 0) {
    return result;
  }
  pid_t pid = fork();
  if (pid == 0) {
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    if (execlp("addr2line", "addr2line", exename, "-f", "-C", "-e", image,
               reinterpret_cast<void *>(NULL)) == -1) {
      exit(0);
    }
  }

  close(pipefd[1]);
  const int line_max_length = 4096;
  char *line = result;
  ssize_t len = read(pipefd[0], line, line_max_length);
  close(pipefd[0]);
  if (len == 0) {
    return result;
  }
  line[len] = 0;

  if (waitpid(pid, NULL, 0) != pid) {
    return result;
  }
  if (line[0] == '?') {
    line[0] = 0;
    return line;
    //    char* straddr = Safe::ptoa(addr, *memory);
    if (color_output) {
      strcpy(line, "\033[32;1m");  // NOLINT(runtime/printf)
    }
    //    strcat(line, straddr);  // NOLINT(runtime/printf)
    if (color_output) {
      strcat(line, "\033[0m");  // NOLINT(runtime/printf)
    }
    strcat(line, " at ");  // NOLINT(runtime/printf)
    strcat(line, image);   // NOLINT(runtime/printf)
    strcat(line, " ");     // NOLINT(runtime/printf)
  } else {
    if (*(strstr(line, "\n") + 1) == '?') {
      //      char* straddr = Safe::ptoa(addr, *memory);
      strcpy(strstr(line, "\n") + 1, image);  // NOLINT(runtime/printf)
      strcat(line, ":");                      // NOLINT(runtime/printf)
      //      strcat(line, straddr);  // NOLINT(runtime/printf)
      //      strcat(line, "\n");  // NOLINT(runtime/printf)
    }
  }
  return line;
}

/*
 * Copyright (c) 2009-2017, Farooq Mela
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cxxabi.h>    // for __cxa_demangle
#include <dlfcn.h>     // for dladdr
#include <execinfo.h>  // for backtrace

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

// This function produces a stack backtrace with demangled function & method
// names.
static std::string Backtrace(int skip = 1) throw() {
  void *callstack[128];
  const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
  char buf[1024];
  int nFrames = backtrace(callstack, nMaxFrames);
  char **symbols = backtrace_symbols(callstack, nFrames);

  std::ostringstream trace_buf;
  for (int i = skip; i < nFrames - 2; i++) {
    // printf("%s\n", symbols[i]);

    Dl_info info;
    if (dladdr(callstack[i], &info) && info.dli_sname) {
      char *demangled = NULL;
      int status = -1;
      if (info.dli_sname[0] == '_')
        demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
      char *line = addr2line(info.dli_fname, callstack[i], false);

      snprintf(buf, sizeof(buf), "%5d %s + %zd %s\n",
               i - skip,  // int(2 + sizeof(void*) * 2), callstack[i],
               status == 0           ? demangled
               : info.dli_sname == 0 ? symbols[i]
                                     : info.dli_sname,
               (char *)callstack[i] - (char *)info.dli_saddr, line);

      free(demangled);
    } else {
      snprintf(buf, sizeof(buf), "%5d %s\n",
               i - skip,  // int(2 + sizeof(void*) * 2), callstack[i],
               symbols[i]);
    }
    trace_buf << buf;
  }
  free(symbols);
  if (nFrames == nMaxFrames) trace_buf << "[truncated]\n";
  return trace_buf.str();
}

#include "Framework/Exception/Exception.h"

namespace framework {
namespace exception {
void Exception::buildStackTrace() throw() { stackTrace_ = Backtrace(2); }
}  // namespace exception
}  // namespace framework
