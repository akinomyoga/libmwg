// -*- mode:C++;coding:shift_jis -*-
#include <cstdlib>
#include <cstdio>
#include <mwg/bio/mwb_format.h>
#include "mwb_dump.h"

//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
  if (argc != 3 || argv[1][0] != 't' || std::strlen(argv[2]) == 0) {
    std::fprintf(stderr, "usage: mwb t <filename>\n");
    std::exit(EXIT_FAILURE);
  }

  if (std::strcmp(argv[2], "-") == 0) {
    mwg::bio::mwb1::dump_mwb(stdout, stdin, mwg::bio::mwb1::dump_mwb_flags::Verbose);
  }else{
    mwg::bio::mwb1::dump_mwb(stdout, argv[2], mwg::bio::mwb1::dump_mwb_flags::Verbose);
  }
  return 0;
}
