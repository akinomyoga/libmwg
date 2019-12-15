// -*- mode:C++;coding:shift_jis -*-
#include <cstdlib>
#include <cstdio>
#include <mwg/bio/mwb_format.h>
#include "mwb_dump.h"

//-----------------------------------------------------------------------------

void usage() {
  std::fprintf(stderr, "usage: mwb [hdv] <filename>\n");
}

int main(int argc, char** argv) {
  if (argc < 2) {
    usage();
    return 2;
  }

  bool flag_error = false;
  bool flag_stdin = false;
  bool flag_dump = false, flag_header = false;
  bool flag_v = false;
  const char* filename = nullptr;
  for (int i = 1; i < argc; i++) {
    const char* arg = argv[i];
    if (arg[0] == '-' || i == 1) {
      if (arg[0] == '-') {
        if (!*arg++) {
          flag_stdin = true;
          continue;
        }
      }
      while (*arg) {
        char c = *arg++;
        switch (c) {
        case 'd':
          flag_dump = true;
          break;

        case 'h':
          flag_header = true;
          break;

        case 'v':
          flag_v = true;
          break;

        default:
          std::fprintf(stderr, "mwb: unrecognized flag '%c'\n", c);
          flag_error = true;
          break;
        }
      }
    } else {
      if (filename) {
        std::fprintf(stderr, "mwb: filename was specified more than once\n");
        flag_error = true;
      }
      filename = arg;
      break;
    }
  }
  if (flag_error) return 2;

  if (flag_dump || flag_header) {
    int flags;
    if (flag_header && flag_dump) {
      flags = mwg::bio::mwb1::dump_mwb_flags::WithDef;
    } else if (flag_header) {
      flags = mwg::bio::mwb1::dump_mwb_flags::Definition;
    } else if (flag_v) {
      flags = mwg::bio::mwb1::dump_mwb_flags::Verbose;
    } else {
      flags = mwg::bio::mwb1::dump_mwb_flags::Simple;
    }

    if (flag_stdin) {
      mwg::bio::mwb1::dump_mwb(stdout, stdin, flags);
    }else{
      mwg::bio::mwb1::dump_mwb(stdout, filename, flags);
    }
    return 0;
  }

  std::fprintf(stderr, "usage: mwb t <filename>\n");
  return 2;
}
