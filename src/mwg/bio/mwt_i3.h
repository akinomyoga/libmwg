// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_FILE_MWT_IMPL3
#define MWG_BIO_FILE_MWT_IMPL3
#include <vector>
#include <string>
#include <sstream>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/std/utility>
#include "defs.h"
#include "tape.h"
#pragma%include "../impl/ManagedTest.pp"
#pragma%x begin_check
#include <cstdio>
#include <mwg/except.h>
#include <mwg/bio/mwt_i3.h>
int main(){
  managed_test::run_tests();
  return 0;
}
#pragma%x end_check

namespace mwg {
namespace bio {
namespace mwtfile_detail {

  typedef u4t index_t;
  static mwg_constexpr_const u4t block_size       = 1024;
  static mwg_constexpr_const u4t nblock_in_plane  = 1024 / sizeof(index_t);
  static mwg_constexpr_const u4t plane_size       = block_size * nblock_in_plane;
  static mwg_constexpr_const u8t mwtfile_size_max = (u8t) block_size * (u8t) UINT32_MAX;
  static mwg_constexpr_const u4t mwtfile_magic    = 'M' | 'W' << 8 | 'T' << 16 | '3' << 24;

  template<typename ITape>
  class mwtfile {
    typedef ITape tape_storage_t;
    typedef typename stdx::add_const_reference<ITape>::type tape_cref;
    typedef typename stdx::remove_reference<ITape>::type tape_type;

    tape_storage_t tape;
    tape_head<tape_type, little_endian_flag> head;

    i8t size;
    std::vector<u4t> fat;

    int status;
    std::string message;
  public:
    operator bool() const {return status == 0;}

  private:
    bool load_size() {
      head.seek(0, SEEK_END);
      size = head.tell();
      if (size == 0 && head.can_write()) {
        size = 4;
        head.template write<u4t>(mwtfile_magic);
      } else if (size < 4 || mwtfile_size_max < (u8t) size) {
        message += "invalid size of mwt structure!\n";
        status = 1;
        return false;
      }
      if (size % block_size != 0) {
        if (head.can_write()) {
          head.align_fill(block_size);
          size = head.tell();
        } else {
          message += "invalid size of mwt structure!\n";
          status = 1;
          return false;
        }
      }
      return true;
    }
    bool load_fat() {
      u4t const nplane = (u4t) ((size + plane_size - 1) / plane_size);
      fat.resize(nplane * nblock_in_plane);
      for (u4t iplane = 0; iplane < nplane; iplane++) {
        u4t* const pfat1 = &fat[iplane * nblock_in_plane];
        head.seek((u8t) iplane * (u8t) plane_size);
        if (nblock_in_plane != head.read(pfat1, pfat1 + nblock_in_plane)) {
          std::ostringstream ostr;
          ostr << "failed to read FATBlock[" << iplane << "] !\n";
          message += ostr.str();
          status = 1;
          return false;
        }
        if (*pfat1 != mwtfile_magic) {
          std::ostringstream ostr;
          ostr << "invalid FATBlock[" << iplane << "] !\n";
          message += ostr.str();
          status = 1;
          return false;
        }
      }
      return true;
    }
  public:
    mwtfile(tape_cref tape): tape(tape), head(tape) {
      status = 0;
      if (!this->load_size()) return;
      if (!this->load_fat()) return;
    }

    void debug_print_fat() const {
      std::printf("---- FAT ----\n");
      for (std::size_t i = 0; i < fat.size(); i++) {
        std::printf("%08x ", fat[i]);
        if (i % 16 == 15) std::putchar('\n');
      }
      std::printf("-------------\n");
      std::fflush(stdout);
    }
  };

#pragma%x begin_test
void test() {
  namespace Mwt = mwg::bio::mwtfile_detail;

  mwg::bio::ftape file("a.mwt", "r+");
  mwg::bio::tape_head<mwg::bio::ftape> head(file);
  head.write<mwg::u4t>(Mwt::mwtfile_magic);

  Mwt::mwtfile<mwg::bio::ftape const&> mwt(file);
  mwg_assert(mwt);

  mwt.debug_print_fat();
  //mwg::bio::mwtfile_detail::mwtfile hello;
}
#pragma%x end_test

}
}
}

#endif
