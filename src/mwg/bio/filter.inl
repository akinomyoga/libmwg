// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_BIO_FILTER_INL
#define MWG_BIO_FILTER_INL
#include <mwg/defs.h>
namespace mwg{
namespace bio{

  // Coder 要件
  //   int Coder::consume(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN);
  //   int Coder::flush(byte*& dst0,byte*const dstN);

  template<typename Coder>
  int filter_with_encoder(const byte*& src0,const byte*const srcN,byte*& dst0,byte*const dstN,void*& state_){
    Coder*& state=reinterpret_cast<Coder*&>(state_);

    if(src0<srcN){
      if(state==nullptr){
        state=new Coder;
        if(state->ret){
          delete state;
          state=nullptr;
          return state->ret; // error
        }
      }

      return state->consume(src0,srcN,dst0,dstN);
    }else{
      if(state!=nullptr){
        if(dst0!=dstN){
          byte* const dst0_=dst0;
          if(state->flush(dst0,dstN))
            return state->ret;
          else if(dst0==dst0_){
            delete state;
            state=nullptr;
          }
        }else{
          delete state;
          state=nullptr;
        }
      }
    }

    return 0;
  }
}
}
#endif
