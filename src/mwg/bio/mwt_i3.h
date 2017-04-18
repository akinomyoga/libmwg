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
  static mwg_constexpr_const u4t block_size       = 1024;
  static mwg_constexpr_const u4t nblock_in_plane  = 1024 / sizeof(bid_t);
  static mwg_constexpr_const u4t nplane_max       = UINT32_MAX / nblock_in_plane;
  static mwg_constexpr_const u4t nblock_max       = nplane_max * nblock_in_plane;
  static mwg_constexpr_const u4t plane_size       = block_size * nblock_in_plane;
  static mwg_constexpr_const u8t mwtfile_size_max = (u8t) block_size * (u8t) nblock_max;
  static mwg_constexpr_const u4t mwtfile_magic    = 'M' | 'W' << 8 | 'T' << 16 | '3' << 24;

  static_assert(UINT32_MAX <= SIZE_MAX, "size_t too small");

  enum bid_constants {
    bid_unused = 0x0000,
    bid_end    = 0x0100,

    bid_master = 1,
    offset_fat_master = bid_master * sizeof(bid_t),
  };

  struct mwtfile_block_chain {
    std::vector<bid_t> blocks;
  };

  typedef u4t hid_t;
  struct heap_entry {
    u4t   size;
    bid_t address;
  };
  static_assert(sizeof(heap_entry) == 8, "struct heap_entry: unexpected size");

  // struct master_block {
  //   bid_t heap_first[number_of_heap_levels]; // 8 16 32 64 128 256 512
  // };
  enum master_block_constans {
    number_of_heap_levels = 7,
    heap_cell_size_base   = 8,
    offset_master_block  = block_size,
    offset_master_hnodes = block_size + 512,
    number_of_hnodes_in_block        = block_size / sizeof(heap_entry),
    number_of_hnodes_in_master_block = block_size / 2 / sizeof(heap_entry),
    hid_offset = number_of_hnodes_in_block - number_of_hnodes_in_master_block,
  };

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
    std::vector<u8t>    heap_bitmaps[7];
    std::vector<hid_t>  heap_free_nodes;

    int status;
  public:
    std::string message;

  public:
    operator bool() const {return status == 0;}

  private:
    bool seek_fill(u8t position) {
      if (head.seek(position) == 0) return true;

      if (size <= position) return false;

      if (head.can_write()) {
        if (head.seek(0, SEEK_END) != 0) return false;
        size = head.tell();
        if (position <= size) return false;
        std::size_t fill_count = position - size;
        size += head.memset(0, position - size);
        return size == position;
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
      for (bid_t bid = fat.size(); bid--; )
        if (fat[bid] == bid_unused)
          free_blocks.push_back(bid);
      return true;
    }
    bool scan_heap_index_block(
      std::size_t iblock, bid_t const bid, int const ibeg, int const iend
    ) {
      hid_t const hid_base = iblock * number_of_hnodes_in_block;
      if (head.seek(bid * (u8t) block_size + ibeg * sizeof(heap_entry)) != 0) {
        u4t const iblock = hid_base / number_of_hnodes_in_block;
        std::ostringstream sstr;
        sstr << "failed to seek in bid:heap_index[" << iblock << "]\n";
        message += sstr.str();
        status = 1;
        return false;
      }

      for (int j = ibeg; j < iend; j++) {
        heap_entry entry;
        if (head.read(entry) != 1) {
          u4t const iblock = hid_base / number_of_hnodes_in_block;
          std::ostringstream sstr;
          sstr << "failed to read bid:heap_index[" << iblock << "]/heap_entry[" << j << "]\n";
          message += sstr.str();
          status = 1;
          return false;
        }

        hid_t const hid = hid_base + j;
        if (entry.size == 0) {
          heap_free_nodes.push_back(hid);
        } else {
          int const level = get_heap_level(entry.size);
          if (0 <= level && level < number_of_heap_levels) {
            heap_bitmaps[level][entry.address / bits_per_hbitmap_element]
              |= (u8t) 1 << entry.address % bits_per_hbitmap_element;
          }
        }
      }

      return true;
    }
    bool initialize_heap_bitmaps() {
      heap_free_nodes.clear();
      for (int level = 0; level < number_of_heap_levels; level++) {
        std::size_t const cell_size = heap_cell_size_base << level;
        std::size_t const cell_count_per_block = block_size / cell_size;
        std::size_t const cell_count = heap_buffers[level].blocks.size() * cell_count_per_block;
        heap_bitmaps[level].clear();
        heap_bitmaps[level].resize(cell_count / bits_per_hbitmap_element, 0);
      }

      for (std::size_t i = 0, iN = heap_index.blocks.size(); i < iN; i++) {
        bid_t const bid = heap_index.blocks[i];
        int const offset = i == 0? hid_offset: 0;
        if (!scan_heap_index_block(i, bid, offset, number_of_hnodes_in_block)) return false;
      }

      std::reverse(heap_free_nodes.begin(), heap_free_nodes.end());
      return true;
    }
    bool load_heap() {
      if (fat[bid_master] == bid_unused) {
        _bchain_initialize(heap_index, bid_unused);
        for (int i = 0; i < number_of_heap_levels; i++)
          _bchain_initialize(heap_buffers[i], bid_unused);
      } else {
        mwg_check(head.seek(bid_master * (u8t) block_size) == 0);
        _bchain_initialize(heap_index, bid_master);
        for (int i = 0; i < number_of_heap_levels; i++) {
          bid_t bid = 0;
          mwg_check(head.read(bid) == 1);
          _bchain_initialize(heap_buffers[i], bid);
        }
      }

      return initialize_heap_bitmaps();
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
        std::printf("%08x ", fat[i]);
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
      for (int i = 0; i < number_of_heap_levels; i++) {
        std::printf("master.heap[%d]:", i);
        print_chain(stdout, heap_buffers[i]);
        std::putchar('\n');
      }

      std::printf("master.heap_free_nodes:\n");
      for (std::size_t i = 0, iN = heap_free_nodes.size(); i < iN; i++) {
        std::printf(" %08x", heap_free_nodes[i]);
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
      if (size < ret * (u8t) block_size) {
        size = ret * (u8t) block_size;
        seek_fill(size);
      }
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
      mwg_check(toffset < size, "toffset = %zd, size = %zd", (std::size_t) toffset, (std::size_t) size);
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
      mwg_check(size <= offset_master_block);
      seek_fill(offset_master_block);
      head.memset(0, block_size);
      head.seek(offset_master_block);
      for (int i = 0; i < number_of_heap_levels; i++)
        head.template write<bid_t>(0);
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
          for (int i = 0; i < number_of_hnodes_in_block; i++)
            head.write(empty);
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

    bid_t _hbitmap_allocate(int level) {
      std::vector<u8t>& hbitmap = heap_bitmaps[level];
      for (std::size_t i = 0, iN = hbitmap.size(); i < iN; i++) {
        if (hbitmap[i] == UINT64_MAX) continue;
        for (int j = 0; j < bits_per_hbitmap_element; j++) {
          if (hbitmap[i] & 1 << j == 0) {
            hbitmap[i] |= 1 << j;
            return i * bits_per_hbitmap_element + j;
          }
        }
      }
      // ToDo
      return 0;
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
        entry.address = _hbitmap_allocate(level);
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
  Mwt::hid_t const hid = mwt.allocate(4);
  std::printf("hid = %d\n", (int) hid);
}
#pragma%x end_test

}
}
}

#endif
