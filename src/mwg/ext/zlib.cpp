// -*- mode: c++; coding: utf-8 -*-
//?mconf H -t'"zlib Library"' -oMWGCONF_LIBRARY_ZLIB zlib.h
#include <mwg_config.h>
#ifdef MWGCONF_LIBRARY_ZLIB
#include <zlib.h>
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/ext/zlib.h>
#include <mwg/bio/filter.inl>
//#define DBG20111219
#ifdef DBG20111219
# include <cstdio>
#endif
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace mwg{
namespace bio{
namespace{
  static_assert(Z_OK==0,"Z_OK is assumed to be 0, or redefinition of the error codes is required.");

  struct ZlibEncodeData{
    int ret;
    z_stream zstr;
    ZlibEncodeData(){
      zstr.zalloc=Z_NULL;
      zstr.zfree =Z_NULL;
      zstr.opaque=Z_NULL;
      ret=::deflateInit(&zstr,Z_DEFAULT_COMPRESSION); // 0-9 [6]
    }
    ~ZlibEncodeData(){
      ::deflateEnd(&zstr);
    }
    int consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN){
      zstr. next_in =const_cast<byte*>(src0);
      zstr.avail_in =srcN-src0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::deflate(&zstr,Z_NO_FLUSH);
      if(ret!=Z_OK&&ret!=Z_STREAM_END)return ret;

      src0=zstr.next_in;
      dst0=dstN-zstr.avail_out;
      return 0;
    }
    int flush(byte*& dst0,byte*const dstN){
      zstr. next_in =nullptr;
      zstr.avail_in =0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::deflate(&zstr,Z_FINISH);
      if(ret!=Z_OK&&ret!=Z_STREAM_END)return ret;

      dst0=dstN-zstr.avail_out;
      return 0;
    }
  };

  struct ZlibDecodeData{
    int ret;
    z_stream zstr;
    ZlibDecodeData(){
      zstr.zalloc=Z_NULL;
      zstr.zfree =Z_NULL;
      zstr.opaque=Z_NULL;
      ret=::inflateInit(&zstr);
    }
    ~ZlibDecodeData(){
      ::inflateEnd(&zstr);
    }
    int consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN){
      zstr. next_in =const_cast<byte*>(src0);
      zstr.avail_in =srcN-src0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
#ifdef DBG20111219
      std::fprintf(stderr, "--- input %p %d / output %p %d\n",    zstr.next_in,zstr.avail_in,zstr.next_out,zstr.avail_out);
#endif
      ret=::inflate(&zstr,Z_NO_FLUSH);
#ifdef DBG20111219
      std::fprintf(stderr,"r=%d input %p %d / output %p %d\n",ret,zstr.next_in,zstr.avail_in,zstr.next_out,zstr.avail_out);
#endif
      if(ret!=Z_OK&&ret!=Z_STREAM_END)return ret;

      src0=zstr.next_in;
      dst0=dstN-zstr.avail_out;
      return 0;
    }
    int flush(byte*& dst0,byte*const dstN){
      zstr. next_in =nullptr;
      zstr.avail_in =0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::inflate(&zstr,Z_FINISH);
      if(ret!=Z_OK&&ret!=Z_STREAM_END)return ret;

      dst0=dstN-zstr.avail_out;
      return 0;
    }
  };
}

  int zlib_encode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    return filter_with_encoder<ZlibEncodeData>(src0,srcN,dst0,dstN,state_);
  }

  int zlib_decode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    return filter_with_encoder<ZlibDecodeData>(src0,srcN,dst0,dstN,state_);
  }

}
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
#endif /* MWGCONF_LIBRARY_ZLIB */
