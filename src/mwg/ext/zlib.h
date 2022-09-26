// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_EXT_ZLIB_H
#define MWG_EXT_ZLIB_H
#include <mwg/defs.h>
#include <mwg/bio/tape.h>
namespace mwg {
namespace bio {
  mwg::bio::filter_function_type zlib_decode;
  mwg::bio::filter_function_type zlib_encode;
  mwg::bio::filter_function_type gzip_decode;
  mwg::bio::filter_function_type gzip_encode;
}
}
#endif
