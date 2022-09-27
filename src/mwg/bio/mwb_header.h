// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_MWB_HEADER_H
#define MWG_BIO_MWB_HEADER_H
#include <cstdlib>
#include <cstring>
#include <complex>
#include <mwg/defs.h>
#include <mwg/stat/errored.h>
#include "tape.h"
namespace mwg {
namespace bio {
namespace mwb1 {
  // mwb leaf
  static const u4t MAGIC    = 0xFBB373DE;
  static const u4t MAGIC_OE = 0xDE73B3FB;
  static const u4t MWBVER   = 0x00000001;

  struct FMTC {
    static const byte I1 = 'h'; //!< ~ higher
    static const byte I2 = 'i';
    static const byte I4 = 'j';
    static const byte I8 = 'k';
    static const byte U1 = 'l'; //!< ~ lower
    static const byte U2 = 'm';
    static const byte U4 = 'n';
    static const byte U8 = 'o'; //!< *o*ctal

    static const byte FLT1 = 'f'; //!< single-precision *f*loating-point number
    static const byte FLT2 = 'd'; //!< *d*ouble-precision fp
    static const byte FLTX = 'e'; //!< *e*xtended double-precision fp

    static const byte CPX1 = 'a'; //!< std::complex<float>
    static const byte CPX2 = 'c'; //!< std::complex<double>            *c*omplex number
    static const byte CPXX = 'b'; //!< std::complex<long double>

    static const byte ERR1 = 'y'; //!< mwg::stat::errored<float>
    static const byte ERR2 = 'x'; //!< mwg::stat::errored<double>      X (probabilistic variable)
    static const byte ERRX = 'z'; //!< mwg::stat::errored<long double>

    static const byte STRL = 's'; //!< char[n]    *s*tring with length
    static const byte STR0 = 't'; //!< char*      null-*t*erminated string
    static const byte WSTL = 'u'; //!< wchar_t[n] *u*nicode string with length         (sizeof(wchar_t)==2)
    static const byte WST0 = 'v'; //!< wchar_t*   null-terminated unicode string

    // not used: g pqr w A-Z

    // functions
    static const byte TYPEDEF = '=';  //!< defines named type
    static const byte NTYPE   = '\''; //!< named type

    static const byte DICT    = ':';  //!< dict  (reserved)
    static const byte MARK    = '<';  //!< tuple start mark
    static const byte TUPLE   = '>';  //!< tuple
    static const byte ARRAY   = '*';  //!< array
    static const byte ATTR    = '@';  //!< attributes
    static const byte NAME    = '"';  //!< name/id
    static const byte COMMENT = '#';  //!< comment
    static const byte FILTER  = '|';  //!< filter
    static const byte PTR4    = '&';  //!< dword pointer with size
    static const byte PTR8    = '%';  //!< qword pointer with size


    // special
    static const byte TERM   = (byte) '\0';   //!< terminator
    static const byte QUOTE  = (byte) '\x80'; //!< quote
    static const byte QUOTEL = (byte) '\xFF'; //!< quote
    // '\x80-\xFE' : QUOTE | length
    static const byte PREFIX = (byte) '\x01'; //!< prefix (reserved)
    // '\x01-\x1F' : prefix
  };

  struct MwbHeader {
    u4t magic;      //!< MAGIC
    u4t mwbver;     //!< MWBVER
    u8t size;       //!< header+data size
    char format[1];
  };

  //TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  struct MwbFormatWriter;
  template<typename T>
  struct MwbFormatWriter_WriteType {};

  //---------------------------------------------------------------------------

  struct MwbFormatWriter {
    mwg::bio::itape const& tape;
    mwg::bio::tape_head<> head;
  public:
    MwbFormatWriter(mwg::bio::itape const& tape): tape(tape), head(tape) {}
    ~MwbFormatWriter() {
      head.write<byte>((byte) FMTC::TERM);
      head.align_fill(8);
    }

    void write_code(byte value) const {
      head.write(value);
    }

    // stack: [write_cnum(n)] n
    void write_cnum(unsigned value) const {
      int c = 0;
      for (unsigned v = value; v; v >>= 8) c++;

      head.write(byte(FMTC::QUOTE | c));
      for (int i = 0; i < c; i++) {
        head.write(byte(value));
        value >>= 8;
      }
    }

    // stack: [write_cstr(s)] s
    void write_cstr(const char* str) const {
      std::size_t len = std::strlen(str);
      if (len < 0x7F) {
        head.write(byte(FMTC::QUOTE | len));
      } else {
        write_cnum(len);
        head.write(byte(FMTC::QUOTEL));
      }
      head.write_data(str, len);
    }

    // stack: element_type [write_array(n)] array_type
    void write_array(unsigned length = 0) const {
      if (length != 0)
        write_cnum(length);
      head.write(byte(FMTC::ARRAY));
    }
    // stack: type [write_name(name)] type_with_name
    void write_name(const char* name) const {
      write_cstr(name);
      head.write(byte(FMTC::NAME));
    }

    // stack: [write_type<T>()] T
    template<typename T>
    void write_type() const {
      MwbFormatWriter_WriteType<T>::write_type(*this);
    }
    // stack: [write_type(type_name)] named_type
    void write_type(const char* name) const {
      write_cstr(name);
      head.write(byte(FMTC::NTYPE));
    }
    // stack: type [write_typedef(type_name)]
    void write_typedef(const char* name) const {
      write_cstr(name);
      head.write(byte(FMTC::TYPEDEF));
    }
  };

  //---------------------------------------------------------------------------
  template<char C>
  struct MwbFormatWriter_WriteTypeImplC {
    static void write_type(MwbFormatWriter const& writer) {
      writer.write_code(C);
    }
  };

  template<typename T, std::size_t N>
  struct MwbFormatWriter_WriteType<T[N]> {
    static void write_type(MwbFormatWriter const& writer) {
      writer.mwg_gcc336bug20160326_template write_type<T>();
      writer.write_array(N);
    }
  };
  template<> struct MwbFormatWriter_WriteType<i1t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::I1> {};
  template<> struct MwbFormatWriter_WriteType<u1t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::U1> {};
  template<> struct MwbFormatWriter_WriteType<i2t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::I2> {};
  template<> struct MwbFormatWriter_WriteType<u2t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::U2> {};
  template<> struct MwbFormatWriter_WriteType<i4t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::I4> {};
  template<> struct MwbFormatWriter_WriteType<u4t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::U4> {};
#ifdef MWGCONF_HAS_64BIT_INTEGER
  template<> struct MwbFormatWriter_WriteType<i8t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::I8> {};
  template<> struct MwbFormatWriter_WriteType<u8t>:
    MwbFormatWriter_WriteTypeImplC<FMTC::U8> {};
#endif
  template<> struct MwbFormatWriter_WriteType<float>:
    MwbFormatWriter_WriteTypeImplC<FMTC::FLT1> {};
  template<> struct MwbFormatWriter_WriteType<double>:
    MwbFormatWriter_WriteTypeImplC<FMTC::FLT2> {};
  template<> struct MwbFormatWriter_WriteType<long double>:
    MwbFormatWriter_WriteTypeImplC<FMTC::FLTX> {};
  template<> struct MwbFormatWriter_WriteType<std::complex<float> >:
    MwbFormatWriter_WriteTypeImplC<FMTC::CPX1> {};
  template<> struct MwbFormatWriter_WriteType<std::complex<double> >:
    MwbFormatWriter_WriteTypeImplC<FMTC::CPX2> {};
  template<> struct MwbFormatWriter_WriteType<std::complex<long double> >:
    MwbFormatWriter_WriteTypeImplC<FMTC::CPXX> {};
  //---------------------------------------------------------------------------

}
}
}
#endif
