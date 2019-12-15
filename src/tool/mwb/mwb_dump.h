// -*- mode:C++;coding:shift_jis -*-
#ifndef MWG_BIO_MWB_DUMP_H
#define MWG_BIO_MWB_DUMP_H
#include <cstdio>
#include <mwg/bio/tape.h>
namespace mwg{
namespace bio{
namespace mwb1{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  struct dump_mwb_flags{
    enum enum_t{
      Simple,
      Verbose,
      WithDef,
      Definition,
    };
  };
  void dump_mwb(FILE* fDst,mwg::bio::itape const& mwbtape,int flags);
  void dump_mwb(FILE* fDst,const char* mwbFilename,int flags);
  void dump_mwb(FILE* fDst,FILE* fMwb,int flags);

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif
