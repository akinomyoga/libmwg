// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_FILE_MWT_IMPL3
#define MWG_BIO_FILE_MWT_IMPL3
#include <climits>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/std/utility>
#include <mwg/std/cinttypes>
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

  typedef u4t bid_t;
  static mwg_constexpr_const u8t   block_size       = 1024;
  static mwg_constexpr_const bid_t nblock_in_plane  = 1024 / sizeof(bid_t);
  static mwg_constexpr_const bid_t nplane_max       = UINT32_MAX / nblock_in_plane;
  static mwg_constexpr_const bid_t nblock_max       = nplane_max * nblock_in_plane;
  static mwg_constexpr_const u8t   plane_size       = block_size * nblock_in_plane;
  static mwg_constexpr_const u8t   mwtfile_size_max = block_size * nblock_max;
  static mwg_constexpr_const u4t   mwtfile_magic    = 'M' | 'W' << 8 | 'T' << 16 | '3' << 24;

  static_assert(UINT32_MAX <= SIZE_MAX, "size_t too small");


  static mwg_constexpr_const bid_t bid_unused = 0x0000;
  static mwg_constexpr_const bid_t bid_end    = 0x0100;

  static mwg_constexpr_const bid_t bid_master = 1;
  static mwg_constexpr_const u8t   offset_fat_master = bid_master * sizeof(bid_t);

  struct mwtfile_block_chain {
    std::vector<bid_t> blocks;
  };

  typedef u4t hid_t;
  struct heap_entry {
    u4t   size;
    bid_t address;
  };
  static_assert(sizeof(heap_entry) == 8, "struct heap_entry: unexpected size");

  static mwg_constexpr_const int number_of_heap_levels = 7;
  static mwg_constexpr_const u8t heap_cell_size_base   = 8;
  static mwg_constexpr_const u8t offset_master_block   = block_size;
  static mwg_constexpr_const u8t offset_master_hnodes = block_size + 512;
  static mwg_constexpr_const hid_t number_of_hnodes_in_block = block_size / sizeof(heap_entry);
  static mwg_constexpr_const hid_t hid_offset                = number_of_hnodes_in_block / 2;

  // struct master_block {
  //   bid_t heap_first[number_of_heap_levels]; // 8 16 32 64 128 256 512
  // };

  inline mwg_constexpr int get_heap_level(u4t size) {
    if (size <= sizeof(bid_t)) return -1;
    for (int i = 0; i < number_of_heap_levels; i++)
      if (size <= heap_cell_size_base << i) return i;
    return number_of_heap_levels;
  }

  template<typename ITape>
  class mwtfile {
    typedef ITape tape_storage_t;
    typedef typename stdx::add_const_reference<ITape>::type tape_cref;
    typedef typename stdx::remove_reference<ITape>::type tape_type;

    tape_storage_t tape;
    tape_head<tape_type, little_endian_flag> head;

    i8t size;
    std::vector<bid_t> fat;
    std::vector<bid_t> free_blocks;

    static std::size_t mwg_constexpr_const bits_per_hbitmap_element = 64;
    mwtfile_block_chain heap_index;
    mwtfile_block_chain heap_buffers[7];
    std::vector<bid_t>  heap_free_cells[7];
    std::vector<hid_t>  heap_free_nodes;

    int status;
  public:
    std::string message;

  public:
    operator bool() const {return status == 0;}

  private:
    bool seek_fill(u8t position) {
      if (position <= size)
        return head.seek(position) == 0;

      if (!head.can_write()) return false;
      head.seek(size);
      size += head.fill_n((byte) 0, position - size);
      return size == position;
    }

  private:
    bool report_error(const char* expr, const char* position, const char* funcname, const char* fmt, ...) {
      mwg_unused(funcname);
      status = 1;
      if (fmt && *fmt) {
        char message1[1024];
        va_list arg;
        va_start(arg, fmt);
#ifdef MWGCONF_HAS_VSNPRINTF
        ::vsnprintf(message1, sizeof message1, fmt, arg);
#else
        std::vsprintf(message1, fmt, arg);
#endif
        va_end(arg);
        message += message1;
        message += "\n";
      } else {
        message += "expr =";
        message += " @ ";
        message += position;
      }
      return false;
    }

  private:
    bool load_size() {
      mwg_check(head.seek(0, SEEK_END) == 0);
      size = head.tell();
      if (size == 0 && head.can_write()) {
        // will be initialized in load_fat.
        return true;
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
      fat.clear();
      free_blocks.clear();
      if (size == 0 && head.can_write()) {
        allocate_plane();
        return true;
      }

      u4t const nplane = (u4t) ((size + plane_size - 1) / plane_size);
      fat.resize(nplane * nblock_in_plane);
      for (u4t iplane = 0; iplane < nplane; iplane++) {
        bid_t* const pfat1 = &fat[iplane * nblock_in_plane];
        mwg_check(head.seek(iplane * (u8t) plane_size) == 0);
        int const count = head.read(pfat1, pfat1 + nblock_in_plane);
        if (count == 0) {
          std::ostringstream ostr;
          ostr << "failed to read FATBlock[" << iplane << "] !\n";
          message += ostr.str();
          status = 1;
          return false;
        } else if (count < nblock_in_plane)
          std::fill(pfat1 + count, pfat1 + nblock_in_plane, bid_unused);

        if (*pfat1 != mwtfile_magic) {
          std::ostringstream ostr;
          ostr << "invalid FATBlock[" << iplane << "] !\n";
          message += ostr.str();
          status = 1;
          return false;
        }
      }
      for (bid_t bid = fat.size(); --bid > bid_master; )
        if (fat[bid] == bid_unused)
          free_blocks.push_back(bid);
      return true;
    }
    bool initialize_heap_free_cells() {
      std::vector<bool> hbitmaps[7];
      for (int level = 0; level < number_of_heap_levels; level++) {
        u8t const cell_size = heap_cell_size_base << level;
        u8t const cell_count_per_block = block_size / cell_size;
        u8t const cell_count = heap_buffers[level].blocks.size() * cell_count_per_block;
        hbitmaps[level].resize(cell_count, false);
      }

      heap_free_nodes.clear();
      for (std::size_t i = 0, iN = heap_index.blocks.size(); i < iN; i++) {
        bid_t const bid = heap_index.blocks[i];
        hid_t const hid_base = i * number_of_hnodes_in_block;
        int const jbeg = i == 0? hid_offset: 0;
        if (!mwg_check_call(head.seek(bid * block_size + jbeg * sizeof(heap_entry)) == 0,
            report_error, "failed to seek in bid:heap_index[%zd] = %08" PRIx32, i, bid))
          return false;

        for (int j = jbeg; j < number_of_hnodes_in_block; j++) {
          hid_t const hid = hid_base + j;
          heap_entry entry;
          if (!mwg_check_call(head.read(entry) == 1,
              report_error, "failed to read heap_entry[%" PRId32 "]", hid))
            return false;

          if (entry.size != 0) {
            int const level = get_heap_level(entry.size);
            if (level < 0 || number_of_heap_levels <= level) continue;

            if (!mwg_check_call(entry.address < hbitmaps[level].size(),
                report_error, "hnode[%" PRId32 "].address out of range", hid))
              return false;
            if (!mwg_check_call(hbitmaps[level][entry.address] == false,
                report_error, "hcell%d[%" PRId32 "] doubly referenced", level, entry.address))
              return false;
            hbitmaps[level][entry.address] = true;
          } else
            heap_free_nodes.push_back(hid);
        }
      }
      std::reverse(heap_free_nodes.begin(), heap_free_nodes.end());

      for (int level = 0; level < number_of_heap_levels; level++) {
        heap_free_cells[level].clear();
        for (bid_t addr = hbitmaps[level].size(); addr--; ) {
          if (!hbitmaps[level][addr])
            heap_free_cells[level].push_back(addr);
        }
      }

      return true;
    }
    bool load_heap() {
      if (fat[bid_master] == bid_unused) {
        _bchain_initialize(heap_index, bid_unused);
        for (int level = 0; level < number_of_heap_levels; level++)
          _bchain_initialize(heap_buffers[level], bid_unused);
      } else {
        mwg_check(head.seek(bid_master * (u8t) block_size) == 0);
        _bchain_initialize(heap_index, bid_master);
        for (int level = 0; level < number_of_heap_levels; level++) {
          bid_t bid = 0;
          mwg_check(head.read(bid) == 1);
          _bchain_initialize(heap_buffers[level], bid);
        }
      }

      return initialize_heap_free_cells();
    }
  public:
    mwtfile(tape_cref tape): tape(tape), head(tape) {
      status = 0;
      if (!this->load_size()) return;
      if (!this->load_fat()) return;
      if (!this->load_heap()) return;
    }

    void debug_print_fat() const {
      std::printf("---- FAT ----\n");
      for (std::size_t i = 0; i < fat.size(); i++) {
        if (fat[i] == bid_unused)
          std::printf("-------- ");
        else if (fat[i] == bid_end)
          std::printf("[ end  ] ");
        else
          std::printf("%08" PRIx32 " ", fat[i]);

        if (i % 16 == 15) std::putchar('\n');
      }
      std::printf("-------------\n");
      std::fflush(stdout);
    }
    static void print_chain(std::FILE* file, mwtfile_block_chain const& chain) {
      std::vector<bid_t> const& blocks = chain.blocks;
      if (blocks.size() == 0) {
        std::fprintf(file, " null");
      } else {
        for (std::size_t j = 0; j < blocks.size(); j++)
          std::fprintf(file, " %08x", blocks[j]);
      }
    }
    void debug_print_master() const {
      std::printf("master.heap_index:");
      print_chain(stdout, heap_index);
      std::putchar('\n');
      for (int level = 0; level < number_of_heap_levels; level++) {
        std::printf("master.heap[%d]:", level);
        print_chain(stdout, heap_buffers[level]);
        std::putchar('\n');
      }

      std::fflush(stdout);
    }
    void debug_print_heap() const {
      for (std::size_t i = 0, iN = heap_index.blocks.size(); i < iN; i++) {
        bid_t const bid = heap_index.blocks[i];
        hid_t const offset = i == 0? hid_offset: 0;
        mwg_check(head.seek(bid * (u8t) block_size + offset * sizeof(heap_entry)) == 0);
        heap_entry entry;
        for (int j = offset; j < number_of_hnodes_in_block; j++) {
          mwg_check(head.read(entry) == 1);
          if (entry.size == 0) continue;
          int const level = get_heap_level(entry.size);
          std::printf("heap_entry[%" PRIx32 "]: ", i * number_of_hnodes_in_block + j);
          if (level < 0)
            std::printf("size = %" PRId32 ", value = %08" PRIx32 "\n", entry.size, entry.address);
          else
            std::printf("size = %" PRId32 ", addr = %" PRId32 "\n", entry.size, entry.address);
        }
      }

      std::printf("heap_free_nodes:\n");
      for (std::size_t i = 0, iN = heap_free_nodes.size(); i < iN; i++) {
        std::printf(" %08" PRIx32, heap_free_nodes[i]);
        if (i % 16 == 15) std::putchar('\n');
      }
      if (heap_free_nodes.size() % 16 != 0)
        std::putchar('\n');

      std::fflush(stdout);
    }

  private:
    void allocate_plane() {
      mwg_check(status == 0 && head.can_write());

      bid_t const bid0 = (bid_t) fat.size();
      bid_t const bidN = bid0 + nblock_in_plane;
      mwg_check(bidN <= nblock_max, "too large mwt structure");

      fat.resize(bidN, bid_unused);
      fat[bid0] = mwtfile_magic;
      mwg_check(head.seek(block_size * (u8t) bid0) == 0);
      head.write_data(&fat[bid0], sizeof(u4t), nblock_in_plane);
      for (bid_t bid = bid0 + 1; bid < bidN; bid++)
        free_blocks.push_back(bid);
      size = (bid0 + 1) * block_size;
    }
    bid_t allocate_block() {
      mwg_check(status == 0 && head.can_write());
      if (!free_blocks.size()) allocate_plane();
      bid_t const ret = free_blocks.back();
      free_blocks.pop_back();
      fat[ret] = bid_end;
      if (size < (ret + 1) * block_size)
        seek_fill((ret + 1) * block_size);
      return ret;
    }
    void free_block(u4t const bid) {
      mwg_assert(bid < fat.size());
      fat[bid] = bid_unused;
      free_blocks.push_back(bid);
      return 0;
    }

  private:
    void _bchain_initialize(mwtfile_block_chain& chain, bid_t bid) const {
      while (bid != bid_unused && bid != bid_end) {
        mwg_check(bid <= fat.size(), "block_chain: invalid bid");
        chain.blocks.push_back(bid);
        bid = fat[bid];
      }
    }

    bid_t _bchain_add_block(mwtfile_block_chain& chain) {
      bid_t const newBlock = allocate_block();
      fat_write(newBlock, bid_end);
      if (!chain.blocks.empty())
        fat_write(chain.blocks.back(), newBlock);
      chain.blocks.push_back(newBlock);
      return newBlock;
    }

    template<typename T>
    void _bchain_write(mwtfile_block_chain const& chain, u8t offset, T const& value) const {
      u8t const bindex = offset / block_size;
      u8t const boffset = offset % block_size;
      mwg_check(boffset + sizeof(value) <= block_size);
      mwg_check(bindex < chain.blocks.size());
      bid_t const bid = chain.blocks[bindex];
      u8t const toffset = bid * (u8t) block_size + boffset;
      mwg_check(toffset < size, "toffset = %" PRId64 ", size = %" PRId64, toffset, size);
      head.seek(toffset);
      head.template write<T>(value);
    }

  private:
    void fat_write(bid_t bid, bid_t next) {
      mwg_check(status == 0 && head.can_write());
      u4t const iplane = bid / nblock_in_plane;
      u4t const column = bid % nblock_in_plane;
      u8t const offset = iplane * (u8t) plane_size + column * sizeof(u4t);
      mwg_check(head.seek(offset) == 0 && head.write(next) == 1);
      fat[bid] = next;
    }

    void write_master_block() {
      mwg_check(head.can_write());
      mwg_check(size <= offset_master_block, "size = %lld, offset_master_block = %d", (long long) size, (int) offset_master_block);
      seek_fill(offset_master_block);
      head.fill_n((byte) 0, block_size);
      head.seek(offset_master_block);
      head.fill_n((bit_t) 0, number_of_heap_levels);
      fat_write(bid_master, bid_end);
      size = (u8t) offset_master_block + block_size;
    }

    hid_t allocate_hnode() {
      mwg_check(status == 0 && head.can_write());

      if (heap_free_nodes.empty()) {
        if (heap_index.blocks.empty()) {
          write_master_block();
          heap_index.blocks.push_back(bid_master);
        } else {
          bid_t const bid = _bchain_add_block(heap_index);
          heap_entry const empty = {};
          head.seek(bid * (u8t) block_size);
          head.fill_n(empty, number_of_hnodes_in_block);
        }

        std::size_t ilast = heap_index.blocks.size() - 1;
        hid_t const hid0 = ilast == 0? hid_offset: ilast * number_of_hnodes_in_block;
        hid_t const hidN = hid0 + number_of_hnodes_in_block;
        for (hid_t hid = hidN; hid-- > hid0; ) heap_free_nodes.push_back(hid);
      }

      hid_t const ret = heap_free_nodes.back();
      heap_free_nodes.pop_back();
      return ret;
    }
    void free_hnode(hid_t hid) {
      if (hid < hid_offset) return;

      bid_t bid;
      if (hid < number_of_hnodes_in_block)
        bid = bid_master;
      else
        bid = heap_index.blocks[hid / number_of_hnodes_in_block - 1];

      heap_entry const empty = {};
      mwg_check(head.seek(bid * (u8t) block_size + hid * (u8t) sizeof(heap_entry)) == 0);
      head.write(empty);

      heap_free_nodes.push_back(hid);
    }

    bid_t _hcell_allocate(int level) {
      std::vector<bid_t>& hcells = heap_free_cells[level];
      if (hcells.empty()) {
        bid_t const bid = _bchain_add_block(heap_buffers[level]);
        bid_t const iblock = heap_buffers[level].blocks.size() - 1;
        if (iblock == 0) {
          head.seek(offset_master_block + level * sizeof(bid_t));
          head.write(bid);
        }
        int const ncell_per_block = block_size / (heap_cell_size_base << level);
        bid_t const addr_base = iblock * ncell_per_block;
        for (int i = ncell_per_block; i--; )
          hcells.push_back(addr_base + i);
      }

      bid_t const ret = hcells.back();
      hcells.pop_back();
      return ret;
    }
  public:
    hid_t allocate(std::size_t sz) {
      hid_t const ret = allocate_hnode();
      heap_entry entry;
      entry.size = sz;
      int const level = get_heap_level(sz);
      if (level < 0) {
        entry.address = 0;
      } else if (level < number_of_heap_levels) {
        entry.address = _hcell_allocate(level);
      } else {
        // allocate on write
        entry.address = 0;
      }
      _bchain_write(heap_index, ret * sizeof(heap_entry), entry);
      return ret;
    }
  };


#pragma%x begin_test
void test() {
  namespace Mwt = mwg::bio::mwtfile_detail;

  mwg::bio::ftape file("a.mwt", "r+");
  mwg_check(file);
  mwg::bio::tape_head<mwg::bio::ftape> head(file);
  head.write<mwg::u4t>(Mwt::mwtfile_magic);

  Mwt::mwtfile<mwg::bio::ftape const&> mwt(file);
  mwg_assert(mwt, "%s", mwt.message.c_str());

  mwt.debug_print_fat();
  mwt.debug_print_master();
  mwt.debug_print_heap();
  Mwt::hid_t const hid1 = mwt.allocate(4);
  Mwt::hid_t const hid2 = mwt.allocate(8);
  std::printf("hid1 = %d\n", (int) hid1);
  std::printf("hid2 = %d\n", (int) hid2);
}
#pragma%x end_test

}
}
}

#endif
