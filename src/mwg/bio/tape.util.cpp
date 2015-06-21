// -*- mode:C++;coding:utf-8 -*-
#include <mwg/bio/tape.h>
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
namespace mwg{
namespace bio{
namespace{

int HexEncode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
  mwg_unused(state_);

  static const char* const TABLE="0123456789abcdef";
  const byte* src=src0;
  byte*       dst=dst0;

  int n=std::min(srcN-src0,(dstN-dst0)/2);
  for(int i=0;i<n;i++,src++){
    *dst++=TABLE[*src>>4];
    *dst++=TABLE[0xF&*src];
  }

  src0=src;
  dst0=dst;
  return 0;
}
//-----------------------------------------------------------------------------

struct hexdecode_state{
  byte f; // leading byte flag
  byte b;
};
int HexDecode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
  hexdecode_state& state(reinterpret_cast<hexdecode_state&>(state_));

  const byte* src=src0;
  byte*       dst=dst0;

  while(src<srcN){
    byte b=*src++;
    if('0'<=b&&b<='9')
      b-='0';
    else if('a'<=b&&b<='f')
      b=b-'a'+10;
    else if('A'<=b&&b<='F')
      b=b-'A'+10;
    else continue;

    if(state.f){
      *dst++=state.b<<4|b;
      state.f=0;
      if(dst>=dstN)break;
    }else{
      state.f=1;
      state.b=b;
    }
  }

  src0=src;
  dst0=dst;
  return 0;
}
//-----------------------------------------------------------------------------

//#define MWG_BIO_TAPE_UTIL_CPP__Base64EncodeImpl1
#define MWG_BIO_TAPE_UTIL_CPP__Base64EncodeImpl2

#ifdef MWG_BIO_TAPE_UTIL_CPP__Base64EncodeImpl1
struct base64encode_state{
  byte count;
  byte c[3];
};
int Base64Encode(const byte*& src0,const byte* srcN,byte*& dst0,byte* dstN,void*& state_){
  //                                    0123456789_123456789_123456789_123456789_123456789_123456789_123456789
  static const char* const BASE64TABLE="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  base64encode_state* state=reinterpret_cast<base64encode_state*>(state_);
  const byte* src=src0;
  byte* dst=dst0;

  mwg_assert(dst+4<=dstN);

  if(src<srcN){
    if(state==nullptr){
      //std::fprintf(stderr,"dbg: base64encode: state alloc\n");
      state_=state=new base64encode_state;
      state->count=0;
    }

    //std::printf("dbg: state->count=%d\n",state->count);
    mwg_assert(0<=state->count&&state->count<3);
    do{
      state->c[state->count++]=*src++;
      if(state->count==3){
        //mwg_assert(!(( state->c[0]>>2 )&~0x3F));
        //mwg_assert(!(( (state->c[0]&0x3)<<4|(state->c[1]&0xF0)>>4 )&~0x3F));
        //mwg_assert(!(( (state->c[1]&0xF)<<2|(state->c[2]&0xC0)>>6 )&~0x3F));
        //mwg_assert(!(( state->c[2]&0x3F )&~0x3F));
        //mwg_assert(dst+3<dstN);
        *dst++=BASE64TABLE[state->c[0]>>2];
        *dst++=BASE64TABLE[(state->c[0]&0x3)<<4|(state->c[1]&0xF0)>>4];
        *dst++=BASE64TABLE[(state->c[1]&0xF)<<2|(state->c[2]&0xC0)>>6];
        *dst++=BASE64TABLE[state->c[2]&0x3F];
        state->count=0;
        if(dst+4>dstN)break;
      }
    }while(src<srcN);
  }else{
    // 終端
    if(state!=nullptr){
      //std::fprintf(stderr,"dbg: base64encode: term\n");
      if(state->count==1){
        *dst++=BASE64TABLE[state->c[0]>>2];
        *dst++=BASE64TABLE[(state->c[0]&3)<<4];
        *dst++='=';
        *dst++='=';
      }else if(state->count==2){
        *dst++=BASE64TABLE[state->c[0]>>2];
        *dst++=BASE64TABLE[(state->c[0]&3)<<4|(state->c[1]&0xF0)>>4];
        *dst++=BASE64TABLE[(state->c[1]&0xF)<<2];
        *dst++='=';
      }
      delete state;
      state_=nullptr;
    }
  }

  src0=src;
  dst0=dst;
  return 0;
}
#endif

#ifdef MWG_BIO_TAPE_UTIL_CPP__Base64EncodeImpl2
//                                           "0123456789_123456789_123456789_123456789_123456789_123456789_123"
  static const char* const Base64EncodeTable="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  int Base64Encode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    static_assert(sizeof(void*)>=2,"the size of the void* pointer is assumed to be larger than 2 bytes.");
    const byte* src=src0;
    byte* dst=dst0;
    byte& buff=(&reinterpret_cast<byte&>(state_))[0];
    byte& stat=(&reinterpret_cast<byte&>(state_))[1];

    byte s;
#define consume_readbyte                        \
    do{if(src>=srcN)goto end;s=*src++;}while(0)
#define consume_checkdst                        \
    do if(dst>=dstN)goto end;while(0)

    if(src0<srcN){
      switch(stat)for(;;){
        default:
        case 0:
          consume_checkdst;
          consume_readbyte;

          *dst++=Base64EncodeTable[s>>2];
          buff=s<<4;
          stat=1;
        case 1:
          consume_checkdst;
          consume_readbyte;

          *dst++=Base64EncodeTable[0x3F&(buff|s>>4)];
          buff=s<<2;
          stat=2;
        case 2:
          consume_checkdst;
          consume_readbyte;

          *dst++=Base64EncodeTable[0x3F&(buff|s>>6)];
          buff=s;
          stat=3;
        case 3:
          consume_checkdst;

          *dst++=Base64EncodeTable[0x3F&buff];
          stat=0;
        }
    }else{
      if(dst0==dstN){
        state_=nullptr;
        return 0;
      }

      switch(stat){
      default:
      case 0:return 0;
      case 1:
        consume_checkdst;

        *dst++=Base64EncodeTable[0x3F&buff];
        goto trail2;
      case 2:
        consume_checkdst;

        *dst++=Base64EncodeTable[0x3F&buff];
        goto trail1;
      case 3:
        consume_checkdst;

        *dst++=Base64EncodeTable[0x3F&buff];
        stat=0;
        goto end;
      trail2:stat=4;
      case 4:
        consume_checkdst;
        *dst++='=';
      trail1:stat=5;
      case 5:
        consume_checkdst;
        *dst++='=';
        stat=0;
        goto end;
      }
    }
  end:
    src0=src;
    dst0=dst;
    return 0;
#undef consume_readbyte
#undef consume_checkdst
  }
#endif

//-----------------------------------------------------------------------------
//  base64 decode

  static const byte Base64DecodeTable[128]={
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3E,0xFF,0xFF,0xFF,0x3F,
    0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
    0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
    0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF,
  };

  // base64*4 (24bit) -> byte*3 (24bit)
  struct Base64Decoder{
    byte buff;
    byte stat;
    int consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN){
      const byte* src=src0;
      byte* dst=dst0;

      byte s;
#define consume_readbyte                                          \
      do if(src>=srcN)goto end;                                   \
      while((s=*src++)>0x7F||(s=Base64DecodeTable[s])==0xFF) /**/
#define consume_checkdst                        \
      do if(dst>=dstN)goto end;while(0)

      switch(stat)for(;;){
        default:
        case 0:
          consume_readbyte;
          buff=s<<2;
          stat=1;
        case 1:
          consume_checkdst;
          consume_readbyte;
          *dst++=buff|s>>4;
          buff=s<<4;
          stat=2;
        case 2:
          consume_checkdst;
          consume_readbyte;
          *dst++=buff|s>>2;
          buff=s<<6;
          stat=3;
        case 3:
          consume_checkdst;
          consume_readbyte;
          *dst++=buff|s;
          stat=0;
        }
#undef consume_readbyte
#undef consume_checkdst
    end:
      src0=src;
      dst0=dst;
      return 0;
    }
    int finish(byte*& dst0,byte*const dstN){
      mwg_unused(dst0);
      mwg_unused(dstN);
      return 0;
    }
  };

  int Base64Decode(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    static_assert(sizeof(void*)>=sizeof(Base64Decoder),"the size of the void* pointer is assumed to be larger than class Base64Decoder.");
    Base64Decoder& state=reinterpret_cast<Base64Decoder&>(state_);

    if(src0<srcN){
      return state.consume(src0,srcN,dst0,dstN);
    }else{
      if(state_!=nullptr){
        if(dst0!=dstN){
          return state.finish(dst0,dstN);
        }else{
          state_=nullptr;
        }
      }
    }

    return 0;
  }
}

int hex_encode(const byte*& src0,const byte* srcN,byte*& dst0,byte* dstN,void*& state_){
  return HexEncode(src0,srcN,dst0,dstN,state_);
}
int hex_decode(const byte*& src0,const byte* srcN,byte*& dst0,byte* dstN,void*& state_){
  return HexDecode(src0,srcN,dst0,dstN,state_);
}
int base64_encode(const byte*& src0,const byte* srcN,byte*& dst0,byte* dstN,void*& state_){
  return Base64Encode(src0,srcN,dst0,dstN,state_);
}
int base64_decode(const byte*& src0,const byte* srcN,byte*& dst0,byte* dstN,void*& state_){
  return Base64Decode(src0,srcN,dst0,dstN,state_);
}

}
}
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
