// -*- mode:C++;coding:utf-8 -*-
//?mconf H -t'"xz Library"' -oMWGCONF_LIBRARY_XZ lzma.h
#include <mwg_config.h>
#ifdef MWGCONF_LIBRARY_XZ
#include <lzma.h>
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/ext/xz.h>
#include <mwg/bio/filter.inl>
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace mwg{
namespace bio{
namespace{
  static_assert(LZMA_OK==0,"LZMA_OK is assumed to be 0, or redefinition of the error codes is required.");

  struct XzEncoder{
    int ret;
    lzma_stream zstr;
    XzEncoder(){
      {lzma_stream zstr_tmp=LZMA_STREAM_INIT;zstr=zstr_tmp;}
      ret=::lzma_easy_encoder(&zstr,6,LZMA_CHECK_CRC64); // 0-9 [6]
    }
    ~XzEncoder(){
      ::lzma_end(&zstr);
    }
    int consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN){
      zstr. next_in =src0;
      zstr.avail_in =srcN-src0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::lzma_code(&zstr,LZMA_RUN);
      if(ret!=LZMA_OK&&ret!=LZMA_STREAM_END)return ret;

      src0=zstr.next_in;
      dst0=dstN-zstr.avail_out;
      return 0;
    }
    int flush(byte*& dst0,byte*const dstN){
      zstr. next_in =nullptr;
      zstr.avail_in =0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::lzma_code(&zstr,LZMA_FINISH);
      if(ret!=LZMA_OK&&ret!=LZMA_STREAM_END)return ret;

      dst0=dstN-zstr.avail_out;
      return 0;
    }
  };

  struct XzDecoder{
    int ret;
    lzma_stream zstr;
    XzDecoder(){
      const qword memlimit=128<<20; // 128MiB
      {lzma_stream zstr_tmp=LZMA_STREAM_INIT;zstr=zstr_tmp;}
      ret=::lzma_stream_decoder(&zstr,memlimit,0); // 0-9 [6]
    }
    ~XzDecoder(){
      ::lzma_end(&zstr);
    }
    int consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN){
      zstr. next_in =src0;
      zstr.avail_in =srcN-src0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::lzma_code(&zstr,LZMA_RUN);
      if(ret!=LZMA_OK&&ret!=LZMA_STREAM_END)return ret;

      src0=zstr.next_in;
      dst0=dstN-zstr.avail_out;
      return 0;
    }
    int flush(byte*& dst0,byte*const dstN){
      zstr. next_in =nullptr;
      zstr.avail_in =0;
      zstr. next_out=dst0;
      zstr.avail_out=dstN-dst0;
      ret=::lzma_code(&zstr,LZMA_FINISH);
      if(ret!=LZMA_OK&&ret!=LZMA_STREAM_END)return ret;

      dst0=dstN-zstr.avail_out;
      return 0;
    }
  };
}

  int xz_encode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    return filter_with_encoder<XzEncoder>(src0,srcN,dst0,dstN,state_);
  }

  int xz_decode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    return filter_with_encoder<XzDecoder>(src0,srcN,dst0,dstN,state_);
  }

}
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
#endif /* MWGCONF_LIBRARY_XZ */
