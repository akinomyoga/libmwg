// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_FILE_MWT_IMPL3
#define MWG_BIO_FILE_MWT_IMPL3
#include <climits>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
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

  template<typename Unsigned>
  inline bool check_overflow_add(Unsigned a, Unsigned b) {
    return b <= std::numeric_limits<Unsigned>::max() - a;
  }

  inline mwg_constexpr u4t fourcc(char a, char b, char c, char d) {
    return a | b << 8 | c << 16 | d << 24;
  }

  typedef u4t bid_t;
  static mwg_constexpr_const u8t   block_size       = 1024;
  static mwg_constexpr_const bid_t nblock_in_plane  = 1024 / sizeof(bid_t);
  static mwg_constexpr_const bid_t nplane_max       = UINT32_MAX / nblock_in_plane;
  static mwg_constexpr_const bid_t nblock_max       = nplane_max * nblock_in_plane;
  static mwg_constexpr_const u8t   plane_size       = block_size * nblock_in_plane;
  static mwg_constexpr_const u8t   mwtfile_size_max = block_size * nblock_max;
  static mwg_constexpr_const u4t   mwtfile_magic    = fourcc('m', 'w', 'h', '1');
  static mwg_constexpr_const u4t   mwtype_heap      = fourcc('h', 'e', 'a', 'p');
  static mwg_constexpr_const u4t   mwtype_tree      = fourcc('t', 'r', 'e', 'e'); // not yet used

  static_assert(UINT32_MAX <= SIZE_MAX, "size_t too small");


  static mwg_constexpr_const bid_t bid_unused = 0x0000;
  static mwg_constexpr_const bid_t bid_end    = 0x0100;

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
  //   byte type[4];
  //   u4t  version;
  //   bid_t heap_first[number_of_heap_levels]; // 8 16 32 64 128 256 512
  // };
  static mwg_constexpr_const bid_t bid_master = 1;
  static mwg_constexpr_const u8t   master_offset              = bid_master * block_size;
  static mwg_constexpr_const u8t   master_offset_of_heap_root = master_offset + 8;
  static mwg_constexpr_const int   number_of_heap_levels      = 7;
  static mwg_constexpr_const u8t   hcell_min_size = 8;
  static mwg_constexpr_const u8t   hcell_max_size = hcell_min_size << number_of_heap_levels - 1;
  static mwg_constexpr_const hid_t number_of_hnodes_in_block = block_size / sizeof(heap_entry);
  static mwg_constexpr_const hid_t hid_offset                = number_of_hnodes_in_block / 2;

  inline mwg_constexpr int get_heap_level(u4t size) {
    if (size <= sizeof(bid_t)) return -1;
    for (int i = 0; i < number_of_heap_levels; i++)
      if (size <= hcell_min_size << i) return i;
    return number_of_heap_levels;
  }

  template<typename Tape>
  class mwheap {
    typedef Tape tape_storage_t;
    typedef typename stdx::add_const_reference<Tape>::type tape_cref;
    typedef typename stdx::remove_reference<Tape>::type tape_type;

    tape_storage_t tape;
    tape_head<tape_type, little_endian_flag> m_head;

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
    tape_head<tape_type, little_endian_flag> const& head() const {return m_head;}

  private:
    bool seek_fill(u8t position) {
      if (position <= size)
        return m_head.seek(position) == 0;

      if (!m_head.can_write()) return false;
      m_head.seek(size);
      size += m_head.fill_n((byte) 0, position - size);
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
      mwg_check(m_head.seek(0, SEEK_END) == 0);
      size = m_head.tell();
      if (size == 0 && m_head.can_write()) {
        // will be initialized in load_fat.
        return true;
      } else if (size < 4 || mwtfile_size_max < (u8t) size) {
        message += "invalid size of mwt structure!\n";
        status = 1;
        return false;
      }
      if (size % block_size != 0) {
        if (m_head.can_write()) {
          m_head.align_fill(block_size);
          size = m_head.tell();
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
      if (size == 0 && m_head.can_write()) {
        allocate_plane();
        return true;
      }

      u4t const nplane = (u4t) ((size + plane_size - 1) / plane_size);
      fat.resize(nplane * nblock_in_plane);
      for (u4t iplane = 0; iplane < nplane; iplane++) {
        bid_t* const pfat1 = &fat[iplane * nblock_in_plane];
        mwg_check(m_head.seek(iplane * (u8t) plane_size) == 0);
        int const count = m_head.read(pfat1, pfat1 + nblock_in_plane);
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
        if (fat[bid] == bid_unused && fat[bid] != bid_master)
          free_blocks.push_back(bid);
      return true;
    }
    bool initialize_heap_free_cells() {
      std::vector<bool> hbitmaps[7];
      for (int level = 0; level < number_of_heap_levels; level++) {
        u8t const cell_size = hcell_min_size << level;
        u8t const cell_count_per_block = block_size / cell_size;
        u8t const cell_count = heap_buffers[level].blocks.size() * cell_count_per_block;
        hbitmaps[level].resize(cell_count, false);
      }

      heap_free_nodes.clear();
      for (std::size_t i = 0, iN = heap_index.blocks.size(); i < iN; i++) {
        bid_t const bid = heap_index.blocks[i];
        hid_t const hid_base = i * number_of_hnodes_in_block;
        int const jbeg = i == 0? hid_offset: 0;
        if (!mwg_check_call(m_head.seek(bid * block_size + jbeg * sizeof(heap_entry)) == 0,
            report_error, "failed to seek in bid:heap_index[%zd] = %08" PRIx32, i, bid))
          return false;

        for (int j = jbeg; j < number_of_hnodes_in_block; j++) {
          hid_t const hid = hid_base + j;
          heap_entry entry;
          if (!mwg_check_call(m_head.read(entry) == 1,
              report_error, "failed to read heap_entry[%" PRId32 "]", hid))
            return false;

          if (entry.size != 0) {
            int const level = get_heap_level(entry.size);
            if (level < 0 || number_of_heap_levels <= level) continue;

            if (!mwg_check_call(entry.address < hbitmaps[level].size(),
                report_error, "hnode[%" PRIx32 "].address (= %" PRId32 ") is out of range [0, %zd)",
                hid, entry.address, hbitmaps[level].size()))
              return false;
            if (!mwg_check_call(hbitmaps[level][entry.address] == false,
                report_error, "hcell%d[%" PRIx32 "] doubly referenced", level, entry.address))
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
        _bchain_initialize(heap_index, bid_master);
        mwg_check(m_head.seek(master_offset_of_heap_root) == 0);
        for (int level = 0; level < number_of_heap_levels; level++) {
          bid_t bid = 0;
          mwg_check(m_head.read(bid) == 1);
          _bchain_initialize(heap_buffers[level], bid);
        }
      }

      return initialize_heap_free_cells();
    }
  public:
    mwheap(tape_cref tape): tape(tape), m_head(tape) {
      status = 0;
      if (!this->load_size()) return;
      if (!this->load_fat()) return;
      if (!this->load_heap()) return;
    }

  public:
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
        mwg_check(m_head.seek(bid * (u8t) block_size + offset * sizeof(heap_entry)) == 0);
        heap_entry entry;
        for (int j = offset; j < number_of_hnodes_in_block; j++) {
          mwg_check(m_head.read(entry) == 1);
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
    void fat_write(bid_t bid, bid_t next) {
      mwg_check(status == 0 && m_head.can_write());
      u4t const iplane = bid / nblock_in_plane;
      u4t const column = bid % nblock_in_plane;
      u8t const offset = iplane * (u8t) plane_size + column * sizeof(u4t);
      mwg_check(m_head.seek(offset) == 0 && m_head.write(next) == 1);
      m_head.flush();
      fat[bid] = next;
    }

    void write_master_block() {
      mwg_check(m_head.can_write());
      mwg_check(size <= master_offset, "size = %lld, master_offset = %d", (long long) size, (int) master_offset);
      seek_fill(master_offset);
      m_head.fill_n((byte) 0, block_size);
      m_head.seek(master_offset);
      m_head.write((u4t) mwtype_heap);
      m_head.write((u4t) 0);
      m_head.fill_n((bid_t) 0, number_of_heap_levels);
      fat_write(bid_master, bid_end);
      size = (u8t) master_offset + block_size;
    }

  private:
    void allocate_plane() {
      mwg_check(status == 0 && m_head.can_write());

      bid_t const bid0 = (bid_t) fat.size();
      bid_t const bidN = bid0 + nblock_in_plane;
      mwg_check(bidN <= nblock_max, "too large mwt structure");

      fat.resize(bidN, bid_unused);
      fat[bid0] = mwtfile_magic;
      mwg_check(m_head.seek(block_size * (u8t) bid0) == 0);
      m_head.write_data(&fat[bid0], sizeof(u4t), nblock_in_plane);
      for (bid_t bid = bidN, bid1 = bid0 + 1; bid-- > bid1; )
        if (bid != bid_master)
          free_blocks.push_back(bid);
      size = (bid0 + 1) * block_size;
    }
    bid_t _block_allocate() {
      mwg_check(status == 0 && m_head.can_write());
      if (!free_blocks.size()) allocate_plane();
      bid_t const ret = free_blocks.back();
      free_blocks.pop_back();
      fat[ret] = bid_end;
      if (size < (ret + 1) * block_size)
        seek_fill((ret + 1) * block_size);
      return ret;
    }
    void _block_free(u4t const bid) {
      mwg_assert(bid < fat.size());
      fat_write(bid, bid_unused);
      free_blocks.push_back(bid);
    }

  private:
    void _bchain_initialize(mwtfile_block_chain& chain, bid_t bid) const {
      // ToDo 無限ループのチェック
      while (bid != bid_unused && bid != bid_end) {
        mwg_check(bid <= fat.size(), "block_chain: invalid bid");
        chain.blocks.push_back(bid);
        bid = fat[bid];
      }
    }
    void _bchain_free(bid_t const first) {
      bid_t bid = first;
      while (bid != bid_unused && bid != bid_end) {
        mwg_check(bid <= fat.size(), "block_chain: invalid bid");
        bid_t const next = fat[bid];
        _block_free(bid);
        bid = next;
      }
    }
    void _bchain_free(mwtfile_block_chain& chain) {
      for (std::size_t i = 0, iN = chain.blocks.size(); i < iN; i++)
        _block_free(chain.blocks[i]);
      chain.blocks.clear();
    }

    bid_t _bchain_add_block(mwtfile_block_chain& chain) {
      bid_t const newBlock = _block_allocate();
      fat_write(newBlock, bid_end);
      if (!chain.blocks.empty())
        fat_write(chain.blocks.back(), newBlock);
      chain.blocks.push_back(newBlock);
      return newBlock;
    }
    bid_t _bchain_add_block(mwtfile_block_chain& chain, u4t count) {
      if (count == 0) return bid_unused;

      std::size_t const old_nblock = chain.blocks.size();
      chain.blocks.reserve(chain.blocks.size() + count);
      for (u4t i = 0; i < count; i++)
        chain.blocks.push_back(_block_allocate());

      u4t iblock = chain.blocks.size() - 1;
      fat_write(chain.blocks[iblock], bid_end);
      for (; iblock > old_nblock; iblock--)
        fat_write(chain.blocks[iblock - 1], chain.blocks[iblock]);
      if (old_nblock != 0)
        fat_write(chain.blocks[old_nblock - 1], chain.blocks[old_nblock]);
      return chain.blocks.back();
    }

    bool _bchain_seek(mwtfile_block_chain const& chain, u8t offset) const {
      u8t const bindex = offset / block_size;
      u8t const boffset = offset % block_size;
      mwg_check(bindex < chain.blocks.size());
      bid_t const bid = chain.blocks[bindex];
      u8t const toffset = bid * (u8t) block_size + boffset;
      mwg_check(toffset < size, "toffset = %" PRId64 ", size = %" PRId64, toffset, size);
      return m_head.seek(toffset) == 0;
    }
    template<typename T>
    bool _bchain_aligned_read(mwtfile_block_chain const& chain, u8t offset, T& value) const {
      mwg_assert(offset % block_size + sizeof(value) <= block_size);
      _bchain_seek(chain, offset);
      return m_head.template read<T>(value) == 1;
    }
    template<typename T>
    bool _bchain_aligned_write(mwtfile_block_chain const& chain, u8t offset, T const& value) const {
      mwg_assert(offset % block_size + sizeof(value) <= block_size);
      _bchain_seek(chain, offset);
      return m_head.template write<T>(value) == 1;
    }
    void _bchain_generic_read(mwtfile_block_chain const& chain, u4t offset, void* _data, u4t sz) {
      byte* data = reinterpret_cast<byte*>(_data);
      u4t beg = offset;
      u4t const end = offset + sz;

      u4t iblock = beg / (u4t) block_size;
      u4t offset_in_block = beg % block_size;
      while (beg < end) {
        if (iblock >= chain.blocks.size()) {
          std::fill(data, data + (end - beg), 0);
          return;
        }

        m_head.seek(chain.blocks[iblock] * block_size + offset_in_block);
        u4t const boundary = (iblock + 1) * (u4t) block_size;
        if (end <= boundary) {
          m_head.read_data(data, 1, end - beg);
          return;
        }

        m_head.read_data(data, 1, boundary - beg);
        data += boundary - beg;
        beg = boundary;
        iblock++;
        offset_in_block = 0;
      }
    }
    void _bchain_generic_write(mwtfile_block_chain& chain, u4t offset, void const* _data, u4t sz) {
      byte const* data = reinterpret_cast<byte const*>(_data);
      u4t beg = offset;
      u4t const end = offset + sz;

      u4t const nblock = (end + (block_size - 1)) / block_size;
      if (nblock > chain.blocks.size())
        _bchain_add_block(chain, nblock - chain.blocks.size());

      u4t iblock = beg / (u4t) block_size;
      u4t offset_in_block = beg % block_size;
      while (beg < end) {
        m_head.seek(chain.blocks[iblock] * block_size + offset_in_block);
        u4t const boundary = (iblock + 1) * (u4t) block_size;
        if (end <= boundary) {
          m_head.write_data(data, 1, end - beg);
          return;
        }

        m_head.write_data(data, 1, boundary - beg);
        data += boundary - beg;
        beg = boundary;
        iblock++;
        offset_in_block = 0;
      }
    }

  private:
    hid_t _hnode_allocate() {
      mwg_check(status == 0 && m_head.can_write());

      if (heap_free_nodes.empty()) {
        if (heap_index.blocks.empty()) {
          write_master_block();
          heap_index.blocks.push_back(bid_master);
        } else {
          bid_t const bid = _bchain_add_block(heap_index);
          heap_entry const empty = {};
          m_head.seek(bid * (u8t) block_size);
          m_head.fill_n(empty, number_of_hnodes_in_block);
        }

        std::size_t ilast = heap_index.blocks.size() - 1;
        hid_t const hid0 = ilast == 0? hid_offset: ilast * number_of_hnodes_in_block;
        hid_t const hidN = (ilast + 1) * number_of_hnodes_in_block;
        for (hid_t hid = hidN; hid-- > hid0; ) heap_free_nodes.push_back(hid);
      }

      hid_t const ret = heap_free_nodes.back();
      heap_free_nodes.pop_back();
      return ret;
    }
    void _hnode_free(hid_t hid) {
      if (hid < hid_offset) return;

      bid_t bid;
      if (hid < number_of_hnodes_in_block)
        bid = bid_master;
      else
        bid = heap_index.blocks[hid / number_of_hnodes_in_block - 1];

      heap_entry const empty = {};
      mwg_check(m_head.seek(bid * (u8t) block_size + hid * (u8t) sizeof(heap_entry)) == 0);
      m_head.write(empty);
      m_head.flush();

      heap_free_nodes.push_back(hid);
    }
  public:
    bool _hnode_load(hid_t hid, heap_entry& entry) {
      return _bchain_aligned_read(heap_index, hid * sizeof(heap_entry), entry);
    }
  private:
    bool _hnode_write(hid_t hid, heap_entry const& entry) {
      bool const ret = _bchain_aligned_write(heap_index, hid * sizeof(heap_entry), entry);
      m_head.flush();
      return ret;
    }

  private:
    bid_t _hcell_allocate(int level) {
      std::vector<bid_t>& hcells = heap_free_cells[level];
      if (hcells.empty()) {
        bid_t const bid = _bchain_add_block(heap_buffers[level]);
        bid_t const iblock = heap_buffers[level].blocks.size() - 1;
        if (iblock == 0) {
          m_head.seek(master_offset_of_heap_root + level * sizeof(bid_t));
          m_head.write(bid);
          m_head.flush();
        }
        int const ncell_per_block = block_size / (hcell_min_size << level);
        bid_t const addr_base = iblock * ncell_per_block;
        for (int i = ncell_per_block; i--; )
          hcells.push_back(addr_base + i);
      }

      bid_t const ret = hcells.back();
      hcells.pop_back();
      return ret;
    }
    void _hcell_deallocate(int level, bid_t addr) {
      std::vector<bid_t>& hcells = heap_free_cells[level];
      hcells.push_back(addr);
    }

  private:
    static void _hentry_to_array(heap_entry const& entry, byte* data) {
      for (u4t i = 0; i < entry.size; i++)
        data[i] = (byte) (entry.address >> i * 8);
    }
    static void _hentry_from_array(heap_entry& entry, byte* data) {
      u4t address = 0;
      for (u4t i = 0; i < entry.size; i++)
        address |= (u4t) data[i] << i * 8;
      entry.address = address;
    }

  public:
    hid_t allocate(u4t sz) {
      if (sz == 0) sz = 1;
      hid_t const ret = _hnode_allocate();
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
      _hnode_write(ret, entry);
      return ret;
    }
    void deallocate(hid_t hid) {
      heap_entry entry;
      _hnode_load(hid, entry);
      if (entry.size == 0) return;

      int const level = get_heap_level(entry.size);
      if (0<= level && level < number_of_heap_levels) {
        _hcell_deallocate(level, entry.address);
      } else if (level == number_of_heap_levels) {
        _bchain_free(entry.address);
      }

      _hnode_free(hid);
    }
    void reallocate(hid_t const hid, u4t const new_size) {
      heap_entry entry;
      _hnode_load(hid, entry);
      return reallocate(hid, new_size, entry);
    }
    void reallocate(hid_t const hid, u4t const new_size, heap_entry& entry) {
      if (entry.size == 0) return;
      u4t const old_size = entry.size;

      int const level1 = get_heap_level(old_size);
      int const level2 = get_heap_level(new_size);
      if (level1 == level2) {
        entry.size = new_size;
        _hnode_write(hid, entry);
        return;
      }

      // ToDo: 先に元データ領域を解放してしまうと
      // 突然プログラムが停止した時にデータが消えてしまうのでは?

      byte data[hcell_max_size];
      if (level1 < 0) {
        _hentry_to_array(entry, data);
      } else if (level1 < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level1], entry.address * (hcell_min_size << level1));
        m_head.read_data(data, 1, std::min(new_size, old_size));
        _hcell_deallocate(level1, entry.address);
      } else if (level1 == number_of_heap_levels) {
        mwg_assert(new_size < old_size);
        bid_t const bid = entry.address;
        if (bid != bid_unused) {
          m_head.seek(bid * block_size);
          m_head.read_data(data, 1, new_size);
          _bchain_free(bid);
        } else
          std::fill(data, data + new_size, 0);
      } else
        mwg_assert(0);

      entry.size = new_size;
      if (level2 < 0) {
        _hentry_from_array(entry, data);
      } else if (level2 < number_of_heap_levels) {
        entry.address = _hcell_allocate(level2);
        _bchain_seek(heap_buffers[level2], entry.address * (hcell_min_size << level2));
        if (new_size <= old_size) {
          m_head.write_data(data, 1, new_size);
        } else {
          m_head.write_data(data, 1, old_size);
          m_head.fill_n((byte) 0, new_size - old_size);
        }
      } else if (level2 == number_of_heap_levels) {
        mwg_assert(new_size > old_size);
        bid_t const bid = _block_allocate();
        fat_write(bid, bid_end);
        m_head.seek(bid * block_size);
        m_head.write_data(data, 1, old_size);
        m_head.fill_n((byte) 0, (u4t) std::min<u8t>(new_size, block_size) - old_size);
        entry.address = bid;
        // ToDo: データの短縮に対応する。bchain を開放する。
      } else
        mwg_assert(0);
      _hnode_write(hid, entry);
    }

  public:
    void heap_read(hid_t hid, heap_entry const& entry, u4t offset, void* data, u4t sz) {
      mwg_check(check_overflow_add<u4t>(offset, sz), "overflow");
      mwg_check(offset + sz <= entry.size, "out of range");
      int const level = get_heap_level(entry.size);
      if (level < 0) {
        byte content[4];
        _hentry_to_array(entry, content);
        std::copy(content + offset, content + offset + sz, reinterpret_cast<byte*>(data));
      } else if(level < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level], entry.address * (hcell_min_size << level) + offset);
        m_head.read_data(data, 1, sz);
      } else {
        mwg_assert(level == number_of_heap_levels);
        mwtfile_block_chain chain;
        _bchain_initialize(chain, entry.address);
        _bchain_generic_read(chain, offset, data, sz);
      }
    }
    void heap_write(hid_t hid, heap_entry& entry, u4t offset, void const* data, u4t sz) {
      mwg_check(check_overflow_add<u4t>(offset, sz), "overflow");
      mwg_check(offset + sz <= entry.size, "out of range. stream size is not automatically extended");
      int const level = get_heap_level(entry.size);
      if (level < 0) {
        byte content[4];
        _hentry_to_array(entry, content);
        byte const* const bdata = reinterpret_cast<byte const*>(data);
        std::copy(bdata, bdata + sz, content + offset);
        _hentry_from_array(entry, content);
        _hnode_write(hid, entry);
      } else if(level < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level], entry.address * (hcell_min_size << level) + offset);
        m_head.write_data(data, 1, sz);
      } else {
        mwg_assert(level == number_of_heap_levels);
        mwtfile_block_chain chain;
        _bchain_initialize(chain, entry.address);
        bool const is_empty = chain.blocks.empty();
        _bchain_generic_write(chain, offset, data, sz);
        if (is_empty && !chain.blocks.empty()) {
          entry.address = chain.blocks[0];
          _hnode_write(hid, entry);
        }
      }
    }
  };


#pragma%x begin_test
typedef mwg::bio::mwtfile_detail::mwheap<mwg::bio::ftape const&> mwheap_t;
void test_heap_read_and_write(mwheap_t& mwt, mwg::bio::mwtfile_detail::hid_t hid, mwg::u4t cell_size) {
  namespace Mwt = mwg::bio::mwtfile_detail;
  Mwt::heap_entry entry;
  mwt._hnode_load(hid, entry);
  if (entry.size == 0) {
    hid = mwt.allocate(cell_size);
    mwt._hnode_load(hid, entry);
  }

  mwg::i4t value;
  mwt.heap_read(hid, entry, 0, &value, sizeof(value));
  value++;
  mwt.heap_write(hid, entry, 0, &value, sizeof(value));
  std::printf("cell[%" PRIu32 "]=%" PRId32 "\n", cell_size, value);
}
void test() {
  namespace Mwt = mwg::bio::mwtfile_detail;

  mwg::bio::ftape file("a.mwheap", "r+");
  mwg_check(file);
  Mwt::mwheap<mwg::bio::ftape const&> mwt(file);
  if (!mwt) {
    std::fprintf(stderr, "a.mwheap is an invalid mwheap file.\n%s", mwt.message.c_str());
    return;
  }

  mwt.debug_print_fat();
  mwt.debug_print_master();
  mwt.debug_print_heap();

  // test allocate
  {
    Mwt::hid_t const hid1 = mwt.allocate(4);
    Mwt::hid_t const hid2 = mwt.allocate(8);
    mwt.deallocate(hid2);
    mwt.deallocate(hid1);

    std::vector<Mwt::hid_t> hids;
    hids.push_back(mwt.allocate(1));
    hids.push_back(mwt.allocate(4));
    hids.push_back(mwt.allocate(5));
    hids.push_back(mwt.allocate(8));
    hids.push_back(mwt.allocate(500));
    hids.push_back(mwt.allocate(1000));
    for (int i = hids.size(); i--; )
      mwt.deallocate(hids[i]);
  }

  // test reallocate
  {
    Mwt::hid_t hid = mwt.allocate(1);
    mwt.reallocate(hid, 2);
    mwt.reallocate(hid, 4);
    mwt.reallocate(hid, 8);
    mwt.reallocate(hid, 500);
    mwt.reallocate(hid, 1000);
    mwt.deallocate(hid);
  }

  // test heap_read/heap_write
  test_heap_read_and_write(mwt, 0x40, 4);
  test_heap_read_and_write(mwt, 0x41, 8);
  test_heap_read_and_write(mwt, 0x42, 512);
  test_heap_read_and_write(mwt, 0x43, 1000);
  test_heap_read_and_write(mwt, 0x44, 2000);

  // mwt.debug_print_fat();
  // mwt.debug_print_master();
  mwt.debug_print_heap();
}
#pragma%x end_test

}
}
}

#endif
