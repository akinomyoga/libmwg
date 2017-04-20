// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_FILE_MWT_IMPL3
#define MWG_BIO_FILE_MWT_IMPL3
#include <climits>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <functional>
#include <mwg/std/type_traits>
#include <mwg/std/memory>
#include <mwg/std/utility>
#include <mwg/std/cinttypes>
#include "defs.h"
#include "tape.h"
#pragma%include "../impl/ManagedTest.pp"
#pragma%x begin_check
#include <cstdio>
#include <string>
#include <cstring>
#include <mwg/except.h>
#include <mwg/bio/mwt_i3.h>
std::string outputdir = ".";
int main(int argc, char** argv){
  if (argc > 0) {
    char* const last = std::strrchr(argv[0], '/');
    if (last) outputdir = std::string(argv[0], last);
  }

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
  struct heap_record {
    u4t   size;
    bid_t address;

    void get_raw_data(byte* const data) const {
      for (u4t i = 0; i < this->size; i++)
        data[i] = (byte) (this->address >> i * 8);
    }
    void set_raw_data(byte* const data) {
      u4t value = 0;
      for (u4t i = 0; i < this->size; i++)
        value |= (u4t) data[i] << i * 8;
      this->address = value;
    }
  };
  static_assert(sizeof(heap_record) == 8, "struct heap_record: unexpected size");

  struct hdata_handle {
    hid_t hid;
    heap_record record;
    mwtfile_block_chain chain;

    hdata_handle() {
      hid = 0;
      record.size = 0;
      record.address = 0;
    }

    explicit operator bool() const {return record.size != 0;}
  };

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
  static mwg_constexpr_const hid_t number_of_hrecords_in_block = block_size / sizeof(heap_record);
  static mwg_constexpr_const hid_t hid_offset                = number_of_hrecords_in_block / 2;

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
        hid_t const hid_base = i * number_of_hrecords_in_block;
        int const jbeg = i == 0? hid_offset: 0;
        if (!mwg_check_call(m_head.seek(bid * block_size + jbeg * sizeof(heap_record)) == 0,
            report_error, "failed to seek in bid:heap_index[%zd] = %08" PRIx32, i, bid))
          return false;

        for (int j = jbeg; j < number_of_hrecords_in_block; j++) {
          hid_t const hid = hid_base + j;
          heap_record record;
          if (!mwg_check_call(m_head.read(record) == 1,
              report_error, "failed to read heap_record[%" PRId32 "]", hid))
            return false;

          if (record.size != 0) {
            int const level = get_heap_level(record.size);
            if (level < 0 || number_of_heap_levels <= level) continue;

            if (!mwg_check_call(record.address < hbitmaps[level].size(),
                report_error, "hnode[%" PRIx32 "].address (= %" PRId32 ") is out of range [0, %zd)",
                hid, record.address, hbitmaps[level].size()))
              return false;
            if (!mwg_check_call(hbitmaps[level][record.address] == false,
                report_error, "hcell%d[%" PRIx32 "] doubly referenced", level, record.address))
              return false;
            hbitmaps[level][record.address] = true;
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
        mwg_check(m_head.seek(bid * (u8t) block_size + offset * sizeof(heap_record)) == 0);
        heap_record record;
        for (int j = offset; j < number_of_hrecords_in_block; j++) {
          mwg_check(m_head.read(record) == 1);
          if (record.size == 0) continue;
          int const level = get_heap_level(record.size);
          std::printf("heap_record[%" PRIx32 "]: ", i * number_of_hrecords_in_block + j);
          if (level < 0)
            std::printf("size = %" PRId32 ", value = %08" PRIx32 "\n", record.size, record.address);
          else
            std::printf("size = %" PRId32 ", addr = %" PRId32 "\n", record.size, record.address);
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
    bid_t _block_allocate(bid_t bid_next) {
      mwg_check(status == 0 && m_head.can_write());
      if (!free_blocks.size()) allocate_plane();
      bid_t const ret = free_blocks.back();
      free_blocks.pop_back();
      fat_write(ret, bid_next);
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
      bid_t const newBlock = _block_allocate(bid_end);
      if (!chain.blocks.empty())
        fat_write(chain.blocks.back(), newBlock);
      chain.blocks.push_back(newBlock);
      return newBlock;
    }
    bid_t _bchain_truncate(bid_t const first, u4t const nblock) {
      u4t iblock = 0;
      bid_t bid = first;
      bid_t bid_prev = bid_unused;
      while (bid != bid_unused && bid != bid_end) {
        mwg_check(bid <= fat.size(), "block_chain: invalid bid");
        bid_prev = bid;
        bid = fat[bid];
        iblock++;

        if (iblock == nblock) {
          if (fat[bid_prev] != bid_end)
            fat_write(bid_prev, bid_end);
        } else if (iblock > nblock)
          _block_free(bid_prev);
      }

      if (iblock >= nblock) return first;

      bid_t bid_next = bid_end;
      for (u4t jblock = nblock; jblock-- > iblock; )
        bid_next = _block_allocate(bid_next);

      if (bid_prev == bid_unused) return bid_next;
      fat_write(bid_prev, bid_next);
      return first;
    }
    void _bchain_truncate(mwtfile_block_chain& chain, u4t const nblock) {
      std::vector<bid_t>& blocks = chain.blocks;
      if (blocks.size() <= nblock) {
        std::size_t const old_nblock = blocks.size();
        if (old_nblock == nblock) return;

        blocks.reserve(nblock);
        bid_t bid_next = bid_end;
        for (u4t iblock = nblock; iblock-- > old_nblock; ) {
          bid_t const bid = _block_allocate(bid_next);
          blocks.push_back(bid);
          bid_next = bid;
        }
        std::reverse(blocks.begin() + old_nblock, blocks.end());

        if (old_nblock > 0)
          fat_write(blocks[old_nblock - 1], bid_next);
      } else {
        if (nblock > 0)
          fat_write(blocks[nblock - 1], bid_end);
        for (std::size_t i = nblock, iN = blocks.size(); i < iN; i++)
          _block_free(blocks[i]);
        blocks.erase(blocks.begin() + nblock, blocks.end());
      }
    }
    bool _bchain_seek(mwtfile_block_chain const& chain, u8t const offset) const {
      u8t const bindex = offset / block_size;
      u8t const boffset = offset % block_size;
      mwg_check(bindex < chain.blocks.size());
      bid_t const bid = chain.blocks[bindex];
      u8t const toffset = bid * (u8t) block_size + boffset;
      mwg_check(toffset < size, "toffset = %" PRId64 ", size = %" PRId64, toffset, size);
      return m_head.seek(toffset) == 0;
    }
    template<typename T>
    bool _bchain_aligned_read(mwtfile_block_chain const& chain, u8t const offset, T& value) const {
      mwg_assert(offset % block_size + sizeof(value) <= block_size);
      _bchain_seek(chain, offset);
      return m_head.template read<T>(value) == 1;
    }
    template<typename T>
    bool _bchain_aligned_write(mwtfile_block_chain const& chain, u8t const offset, T const& value) const {
      mwg_assert(offset % block_size + sizeof(value) <= block_size);
      _bchain_seek(chain, offset);
      return m_head.template write<T>(value) == 1;
    }
    bool _bchain_generic_read(mwtfile_block_chain const& chain, u4t const offset, void* const _data, u4t const sz) {
      byte* data = reinterpret_cast<byte*>(_data);
      u4t beg = offset;
      u4t const end = offset + sz;

      u4t iblock = beg / (u4t) block_size;
      u4t offset_in_block = beg % block_size;
      while (beg < end) {
        if (iblock >= chain.blocks.size()) {
          std::fill(data, data + (end - beg), 0);
          return true;
        }

        m_head.seek(chain.blocks[iblock] * block_size + offset_in_block);
        u4t const boundary = (iblock + 1) * (u4t) block_size;
        if (end <= boundary)
          return m_head.read_data(data, 1, end - beg) != 0;

        m_head.read_data(data, 1, boundary - beg);
        data += boundary - beg;
        beg = boundary;
        iblock++;
        offset_in_block = 0;
      }
      return true;
    }
    bool _bchain_generic_write(mwtfile_block_chain& chain, u4t offset, void const* _data, u4t sz) {
      byte const* data = reinterpret_cast<byte const*>(_data);
      u4t beg = offset;
      u4t const end = offset + sz;

      u4t const nblock = (end + (block_size - 1)) / block_size;
      if (nblock > chain.blocks.size())
        _bchain_truncate(chain, nblock);

      u4t iblock = beg / (u4t) block_size;
      u4t offset_in_block = beg % block_size;
      while (beg < end) {
        m_head.seek(chain.blocks[iblock] * block_size + offset_in_block);
        u4t const boundary = (iblock + 1) * (u4t) block_size;
        if (end <= boundary)
          return m_head.write_data(data, 1, end - beg) != 0;

        m_head.write_data(data, 1, boundary - beg);
        data += boundary - beg;
        beg = boundary;
        iblock++;
        offset_in_block = 0;
      }
      return true;
    }

  private:
    bool _hrecord_allocate_at(hid_t const hid) {
      std::vector<hid_t>& nodes = heap_free_nodes;
      std::greater<hid_t> compare;
      std::sort(nodes.begin(), nodes.end(), compare);
      std::vector<hid_t>::iterator const it = std::lower_bound(nodes.begin(), nodes.end(), hid, compare);
      if (it != nodes.end() && *it == hid) {
        nodes.erase(it);
        return true;
      } else
        return false;
    }
    hid_t _hrecord_allocate() {
      mwg_check(status == 0 && m_head.can_write());

      if (heap_free_nodes.empty()) {
        if (heap_index.blocks.empty()) {
          write_master_block();
          heap_index.blocks.push_back(bid_master);
        } else {
          bid_t const bid = _bchain_add_block(heap_index);
          heap_record const empty = {};
          m_head.seek(bid * (u8t) block_size);
          m_head.fill_n(empty, number_of_hrecords_in_block);
        }

        std::size_t ilast = heap_index.blocks.size() - 1;
        hid_t const hid0 = ilast == 0? hid_offset: ilast * number_of_hrecords_in_block;
        hid_t const hidN = (ilast + 1) * number_of_hrecords_in_block;
        for (hid_t hid = hidN; hid-- > hid0; ) heap_free_nodes.push_back(hid);
      }

      hid_t const ret = heap_free_nodes.back();
      heap_free_nodes.pop_back();
      return ret;
    }
    void _hrecord_free(hid_t const hid) {
      if (hid < hid_offset) return;

      bid_t bid;
      if (hid < number_of_hrecords_in_block)
        bid = bid_master;
      else
        bid = heap_index.blocks[hid / number_of_hrecords_in_block - 1];

      heap_record const empty = {};
      mwg_check(m_head.seek(bid * (u8t) block_size + hid * (u8t) sizeof(heap_record)) == 0);
      m_head.write(empty);
      m_head.flush();

      heap_free_nodes.push_back(hid);
    }
    bool _hrecord_load(hid_t const hid, heap_record& record) {
      return _bchain_aligned_read(heap_index, hid * sizeof(heap_record), record);
    }
    bool _hrecord_write(hid_t const hid, heap_record const& record) {
      bool const ret = _bchain_aligned_write(heap_index, hid * sizeof(heap_record), record);
      m_head.flush();
      return ret;
    }

  private:
    bid_t _hcell_allocate(int const level) {
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
    void _hcell_deallocate(int const level, bid_t const addr) {
      std::vector<bid_t>& hcells = heap_free_cells[level];
      hcells.push_back(addr);
    }

  public:
    hid_t allocate(u4t sz) {
      if (sz == 0) sz = 1;
      hid_t const ret = _hrecord_allocate();
      heap_record record;
      record.size = sz;
      int const level = get_heap_level(sz);
      if (level < 0) {
        record.address = 0;
      } else if (level < number_of_heap_levels) {
        record.address = _hcell_allocate(level);
      } else {
        // allocate on write
        record.address = 0;
      }
      _hrecord_write(ret, record);
      return ret;
    }
    void deallocate(hid_t const hid) {
      heap_record record;
      _hrecord_load(hid, record);
      if (record.size == 0) return;

      int const level = get_heap_level(record.size);
      if (0<= level && level < number_of_heap_levels) {
        _hcell_deallocate(level, record.address);
      } else if (level == number_of_heap_levels) {
        _bchain_free(record.address);
      }

      _hrecord_free(hid);
    }
  private:
    bool reallocate_impl(hid_t const hid, heap_record& record, mwtfile_block_chain* pchain, u4t new_size) {
      if (record.size == 0) {
        // newly allocate
        if (!_hrecord_allocate_at(hid)) return false;
      }

      u4t const old_size = record.size;
      if (new_size == 0) new_size = 1;
      int const level1 = get_heap_level(old_size);
      int const level2 = get_heap_level(new_size);
      if (level1 == level2) {
        if (level1 == number_of_heap_levels && new_size < old_size) {
          u4t const old_nblock = (old_size + (block_size - 1)) / block_size;
          u4t const new_nblock = (new_size + (block_size - 1)) / block_size;
          if (new_nblock < old_nblock) {
            if (pchain)
              _bchain_truncate(*pchain, new_nblock);
            else
              _bchain_truncate(record.address, new_nblock);
          }
        }

        record.size = new_size;
        _hrecord_write(hid, record);
        return true;
      }

      // ToDo: 先に元データ領域を解放してしまうと
      // 突然プログラムが停止した時にデータが消えてしまうのでは?

      byte data[hcell_max_size];
      if (old_size == 0) {
        // do nothing
      } else if (level1 < 0) {
        record.get_raw_data(data);
      } else if (level1 < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level1], record.address * (hcell_min_size << level1));
        m_head.read_data(data, 1, std::min(new_size, old_size));
        _hcell_deallocate(level1, record.address);
      } else if (level1 == number_of_heap_levels) {
        mwg_assert(new_size < old_size);
        bid_t const bid = record.address;
        if (bid != bid_unused) {
          m_head.seek(bid * block_size);
          m_head.read_data(data, 1, new_size);
          if (pchain)
            _bchain_free(*pchain);
          else
            _bchain_free(bid);
        } else
          std::fill(data, data + new_size, 0);
      } else
        mwg_assert(0);

      record.size = new_size;
      if (level2 < 0) {
        record.set_raw_data(data);
      } else if (level2 < number_of_heap_levels) {
        record.address = _hcell_allocate(level2);
        _bchain_seek(heap_buffers[level2], record.address * (hcell_min_size << level2));
        if (new_size <= old_size) {
          m_head.write_data(data, 1, new_size);
        } else {
          m_head.write_data(data, 1, old_size);
          m_head.fill_n((byte) 0, new_size - old_size);
        }
      } else if (level2 == number_of_heap_levels) {
        mwg_assert(new_size > old_size);
        bid_t const bid = _block_allocate(bid_end);
        m_head.seek(bid * block_size);
        m_head.write_data(data, 1, old_size);
        m_head.fill_n((byte) 0, (u4t) std::min<u8t>(new_size, block_size) - old_size);
        record.address = bid;
        if (pchain) pchain->blocks.push_back(bid);
      } else
        mwg_assert(0);
      _hrecord_write(hid, record);
      return true;
    }
  public:
    bool reallocate(hid_t const hid, u4t const new_size) {
      heap_record record;
      _hrecord_load(hid, record);
      return reallocate_impl(hid, record, nullptr, new_size);
    }
    bool reallocate(hdata_handle& hdata, u4t const new_size) {
      return reallocate_impl(hdata.hid, hdata.record, &hdata.chain, new_size);
    }

  public:
    bool hdata_initialize(hdata_handle& hdata, hid_t hid) {
      hdata.hid = hid;
      _hrecord_load(hid, hdata.record);
      if (hdata.record.size == 0) return false;
      if (get_heap_level(hdata.record.size) == number_of_heap_levels)
        _bchain_initialize(hdata.chain, hdata.record.address);
      else
        hdata.chain.blocks.clear();
      return true;
    }
    bool hdata_read(hdata_handle const& hdata, u4t const offset, void* const data, u4t const sz) {
      mwg_check(check_overflow_add<u4t>(offset, sz), "overflow: offset=0x%" PRIx32 ", sz=0x%" PRIx32, offset, sz);
      mwg_check(offset + sz <= hdata.record.size, "out of range");
      heap_record const& record = hdata.record;
      mwtfile_block_chain const& chain = hdata.chain;

      int const level = get_heap_level(record.size);
      if (level < 0) {
        byte content[4];
        record.get_raw_data(content);
        std::copy(content + offset, content + offset + sz, reinterpret_cast<byte*>(data));
        return true;
      } else if(level < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level], record.address * (hcell_min_size << level) + offset);
        return m_head.read_data(data, 1, sz) == sz;
      } else {
        mwg_assert(level == number_of_heap_levels);
        return _bchain_generic_read(chain, offset, data, sz);
      }
    }
    bool hdata_write(hdata_handle& hdata, u4t const offset, void const* const data, u4t const sz) {
      hid_t const hid = hdata.hid;
      heap_record& record = hdata.record;
      mwtfile_block_chain& chain = hdata.chain;
      mwg_check(check_overflow_add<u4t>(offset, sz), "overflow: offset=0x%" PRIx32 ", sz=0x%" PRIx32, offset, sz);
      mwg_check(offset + sz <= record.size,
        "out of range. stream size is not automatically extended (offset + sz = %" PRId32 ", record.size = %" PRId32 ")",
        offset + sz, record.size);

      int const level = get_heap_level(record.size);
      if (level < 0) {
        byte content[4];
        record.get_raw_data(content);
        byte const* const bdata = reinterpret_cast<byte const*>(data);
        std::copy(bdata, bdata + sz, content + offset);
        record.set_raw_data(content);
        return _hrecord_write(hid, record);
      } else if(level < number_of_heap_levels) {
        _bchain_seek(heap_buffers[level], record.address * (hcell_min_size << level) + offset);
        return m_head.write_data(data, 1, sz) == sz;
      } else {
        mwg_assert(level == number_of_heap_levels);
        bool const is_empty = chain.blocks.empty();
        bool const ret = _bchain_generic_write(chain, offset, data, sz);
        if (is_empty && !chain.blocks.empty()) {
          record.address = chain.blocks[0];
          _hrecord_write(hid, record);
        }
        return ret;
      }
    }
  };

  template<typename Tape>
  class mwheap_tape {
    mwheap<Tape>* file;
    bool flag_write;
    bool flag_resize;

    mutable u4t m_pos;
    mutable u4t m_size;

    mutable hdata_handle hdata;
    mutable std::vector<byte> wbuff;

  public:
    mwg_constexpr mwheap_tape(mwheap<Tape>& file, hid_t hid):
      file(&file),
      flag_write(file.head().can_write()),
      flag_resize(flag_write),
      m_pos(0), m_size(0)
    {
      file.hdata_initialize(hdata, hid);
      m_size = hdata.record.size;
      wbuff.reserve(4 * 1024);
    }
    ~mwheap_tape() {flush();}

  public:
    mwg_constexpr bool can_read() const {return true;}
    mwg_constexpr bool can_write() const {return flag_write;}
    mwg_constexpr bool can_seek() const {return true;}
    mwg_constexpr bool can_trunc() const {return flag_resize;}

    int read(void* data, int unit, int n = 1) const {
      u4t const capacity = (m_size - m_pos) / unit;
      if (capacity < n) n = capacity;
      file->hdata_read(hdata, m_pos, data, n * unit);
      m_pos += n * unit;
      return n;
    }
    int write(const void* _data, int unit, int n = 1) const {
      mwg_check(can_write() && unit > 0);
      byte const* data = reinterpret_cast<byte const*>(_data);

      u4t const capacity = ((flag_resize? UINT32_MAX : m_size) - m_pos) / unit;
      if (capacity < n) n = capacity;

      u4t const wsize   = n * unit;
      u4t const newpos  = m_pos + wsize;
      u4t const newsize = newpos > m_size? newpos: m_size;
      if (wbuff.size() + wsize <= wbuff.capacity()) {
        wbuff.insert(wbuff.end(), data, data + wsize);
      } else if (wbuff.empty()) {
        if (hdata.record.size < newsize)
          file->reallocate(hdata, newsize);
        file->hdata_write(hdata, m_pos, data, wsize);
      } else if (wbuff.size() + wsize < 2 * wbuff.capacity()) {
        if (hdata.record.size < newsize)
          file->reallocate(hdata, newsize);
        u4t const buffered_pos = m_pos - wbuff.size();
        std::size_t const rest = wbuff.capacity() - wbuff.size();
        wbuff.insert(wbuff.end(), data, data + rest);
        file->hdata_write(hdata, buffered_pos, &wbuff[0], wbuff.size());
        wbuff.clear();
        wbuff.insert(wbuff.end(), data + rest, data + wsize);
      } else {
        mwg_assert(wbuff.size() <= m_pos, "|wbuff| = %zd, m_pos = %" PRId32, wbuff.size(), m_pos);
        if (hdata.record.size < newsize)
          file->reallocate(hdata, newsize);
        file->hdata_write(hdata, m_pos - wbuff.size(), &wbuff[0], wbuff.size());
        file->hdata_write(hdata, m_pos, data, wsize);
        wbuff.clear();
      }

      m_pos = newpos;
      m_size = newsize;
      return n;
    }
    int seek(i8t offset, int whence = SEEK_SET) const {
      i8t newpos = m_pos;
      switch (whence) {
      case SEEK_SET:
        newpos = offset;
        break;
      case SEEK_CUR:
        newpos = m_pos + offset;
        break;
      case SEEK_END:
        newpos = m_size + offset;
        break;
      default:
        mwg_check(false, "invalid argument whence");
        break;
      }

      if (newpos == m_pos) return 0;
      if (newpos < 0) return -1; // out of range

      if (newpos > m_size) {
        if (!flag_resize) return -1;
        file->reallocate(hdata, newpos);
        m_size = newpos;
      }
      this->flush();
      m_pos = newpos;
      return 0;
    }
    mwg_constexpr i8t tell() const {return m_pos;}
    mwg_constexpr u8t size() const {return m_size;}
    int trunc(u8t newsize) const {
      mwg_check(can_trunc());
      this->flush();
      file->reallocate(hdata, newsize);
    }
    int flush() const {
      if (wbuff.empty()) return 0;
      mwg_assert(wbuff.size() <= m_pos, "|wbuff| = %zd, m_pos = %" PRId32, wbuff.size(), m_pos);
      if (hdata.record.size < m_size)
        file->reallocate(hdata, m_size);
      bool const result = file->hdata_write(hdata, m_pos - wbuff.size(), &wbuff[0], wbuff.size());
      wbuff.clear();
      return result? 0: -1;
    }

    mwg_constexpr bool is_alive() const {return (bool) *file;}
    mwg_constexpr operator bool() const {return this->is_alive();}
  };

#pragma%x begin_test
typedef mwg::bio::mwtfile_detail::mwheap<mwg::bio::ftape const&> mwheap_t;
void test_heap_read_and_write(mwheap_t& mwt, mwg::bio::mwtfile_detail::hid_t hid, mwg::u4t cell_size) {
  namespace Mwt = mwg::bio::mwtfile_detail;

  Mwt::hdata_handle hdata;
  mwt.hdata_initialize(hdata, hid);
  if (!hdata) {
    hid = mwt.allocate(cell_size);
    mwt.hdata_initialize(hdata, hid);
  }

  mwg::i4t value;
  mwt.hdata_read(hdata, 0, &value, sizeof(value));
  value++;
  mwt.hdata_write(hdata, 0, &value, sizeof(value));
  std::printf("cell[%" PRIu32 "]=%" PRId32 "\n", cell_size, value);
}
void test() {
  namespace Mwt = mwg::bio::mwtfile_detail;

  std::string filename = outputdir + "/a.mwheap";
  mwg::bio::ftape file;
  if (!file.open(filename.c_str(), "r+"))
    file.open(filename.c_str(), "w+");
  mwg_check(file, "failed to open the file '%s'\n", filename.c_str());

  Mwt::mwheap<mwg::bio::ftape const&> mwt(file);
  if (!mwt) {
    std::fprintf(stderr, "a.mwheap is an invalid mwheap file.\n%s", mwt.message.c_str());
    mwg_check(mwt);
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

  // test hdata_read/hdata_write
  test_heap_read_and_write(mwt, 0x40, 4);
  test_heap_read_and_write(mwt, 0x41, 8);
  test_heap_read_and_write(mwt, 0x42, 512);
  test_heap_read_and_write(mwt, 0x43, 1000);
  test_heap_read_and_write(mwt, 0x44, 2000);
  {
    Mwt::hid_t hid_stream = 0x45;
    mwg_check(mwt.reallocate(hid_stream, 1));
    Mwt::mwheap_tape<mwg::bio::ftape const&> tape1(mwt, hid_stream);

    typedef Mwt::mwheap_tape<mwg::bio::ftape const&> tape1_t;
    mwg::bio::tape_head<tape1_t> head1(tape1);
    for (int i = 0; i < 1200; i++)
      head1.write<mwg::u4t>(i * (i + 1) % 9731);
    head1.seek(0);
    for (int i = 0; i < 1200; i++) {
      mwg::u4t value;
      head1.read<mwg::u4t>(value);
      mwg_check(value == i * (i + 1) % 9731);
    }
  }

  // mwt.debug_print_fat();
  // mwt.debug_print_master();
  mwt.debug_print_heap();
}
#pragma%x end_test

}
}
}

#endif
