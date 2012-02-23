// -*- mode:C++;coding:shift_jis -*-
#include <cstdlib>
#include <cstdio>
#include <map>
#include <stack>
#include <vector>
#include <algorithm>
#include <mwg/std/memory>
#include <mwg/bio/mwb.h>
#include "mwb.format.h"

namespace mwg{
namespace bio{
namespace mwb1{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//-----------------------------------------------------------------------------
//  Basic traits for dump

  template<byte FMTC> struct DumpTraits{
    static const char* get_typename(){return nullptr;}
  };

  template<> struct DumpTraits<FMTC::FLT1>{
    typedef float element_type;
    static const int element_number=1;
    static const char* get_typename(){return "float";}
  };
  template<> struct DumpTraits<FMTC::FLT2>{
    typedef double element_type;
    static const int element_number=1;
    static const char* get_typename(){return "double";}
  };
  template<> struct DumpTraits<FMTC::FLTX>{
    typedef long double element_type;
    static const int element_number=1;
    static const char* get_typename(){return "long double";}
  };
  template<> struct DumpTraits<FMTC::CPX1>{
    typedef float element_type;
    static const int element_number=2;
    static const char* get_typename(){return "complex<float>";}
  };
  template<> struct DumpTraits<FMTC::CPX2>{
    typedef double element_type;
    static const int element_number=2;
    static const char* get_typename(){return "complex<double>";}
  };
  template<> struct DumpTraits<FMTC::CPXX>{
    typedef long double element_type;
    static const int element_number=2;
    static const char* get_typename(){return "complex<long double>";}
  };
  template<> struct DumpTraits<FMTC::ERR1>{
    typedef float element_type;
    static const int element_number=2;
    static const char* get_typename(){return "errored<float>";}
  };
  template<> struct DumpTraits<FMTC::ERR2>{
    typedef double element_type;
    static const int element_number=2;
    static const char* get_typename(){return "errored<double>";}
  };
  template<> struct DumpTraits<FMTC::ERRX>{
    typedef long double element_type;
    static const int element_number=2;
    static const char* get_typename(){return "errored<long double>";}
  };

  template<> struct DumpTraits<FMTC::I1>{
    typedef i1t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "char";}
  };
  template<> struct DumpTraits<FMTC::I2>{
    typedef i2t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "short";}
  };
  template<> struct DumpTraits<FMTC::I4>{
    typedef i4t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "int";}
  };
  template<> struct DumpTraits<FMTC::I8>{
    typedef i8t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "long";}
  };
  template<> struct DumpTraits<FMTC::U1>{
    typedef u1t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "byte";}
  };
  template<> struct DumpTraits<FMTC::U2>{
    typedef u2t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "word";}
  };
  template<> struct DumpTraits<FMTC::U4>{
    typedef u4t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "dword";}
  };
  template<> struct DumpTraits<FMTC::U8>{
    typedef u8t element_type;
    static const int element_number=1;
    static const char* get_typename(){return "qword";}
  };

  template<typename T> struct printf_format{static const char* fmt(){return nullptr;}};
  template<> struct printf_format<int>{static const char* fmt(){return "%d";}};
  template<> struct printf_format<long>{static const char* fmt(){return "%ld";}};
  template<> struct printf_format<long long>{static const char* fmt(){return "%lld";}};
  template<> struct printf_format<unsigned int>{static const char* fmt(){return "%u";}};
  template<> struct printf_format<unsigned long>{static const char* fmt(){return "%lu";}};
  template<> struct printf_format<unsigned long long>{static const char* fmt(){return "%llu";}};
  template<> struct printf_format<signed char>:printf_format<int>{};
  template<> struct printf_format<signed short>:printf_format<int>{};
  template<> struct printf_format<unsigned char>:printf_format<unsigned int>{};
  template<> struct printf_format<unsigned short>:printf_format<unsigned int>{};
  template<> struct printf_format<float>{static const char* fmt(){return "%.8g";}};
  template<> struct printf_format<double>{static const char* fmt(){return "%.15lg";}};
  template<> struct printf_format<long double>{static const char* fmt(){return "%.19llg";}};
  template<> struct printf_format<char>:printf_format<unsigned int>{};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//  struct DumpExec;
//-----------------------------------------------------------------------------
  struct DumpExec{
    mwg::bio::tape_head<mwg::bio::itape,mwg::bio::little_endian_flag> h;
    FILE* fout;

    int indent_level;
    bool fIndent;
  private:
    void newline(){
      std::fputc('\n',this->fout);
      fIndent=true;
    }
    void before_print(){
      if(fIndent){
        for(int i=0;i<indent_level;i++)fputc(' ',this->fout);
        fIndent=false;
      }
    }
    void print(const char* txt){
      this->before_print();
      std::fprintf(this->fout,"%s",txt);
    }
  public:
    DumpExec(mwg::bio::itape const& tape,FILE* fout):h(tape),fout(fout),indent_level(0),fIndent(false){}
//-----------------------------------------------------------------------------
  private:
    template<int fmtc>
    bool dumpA(){
      typedef typename DumpTraits<fmtc>::element_type element_type;
      const char* fmt=printf_format<element_type>::fmt();
      this->before_print();
      if(DumpTraits<fmtc>::element_number==1){
        element_type value;
        if(!h.read(value))return false;
        std::fprintf(this->fout,fmt,value);
        std::fputc(',',this->fout);
        std::fputc(' ',this->fout);
      }else if(DumpTraits<fmtc>::element_number==2){
        element_type value;
        std::fprintf(this->fout,"%s(",DumpTraits<fmtc>::get_typename());

        if(!h.read(value))return false;
        std::fprintf(this->fout,fmt,value);

        std::fputc(',',this->fout);
        if(!h.read(value))return false;
        std::fprintf(this->fout,fmt,value);
      
        std::fprintf(this->fout,"), ");
      }else{
        element_type value;
        std::fprintf(this->fout,"%s(",DumpTraits<fmtc>::get_typename());

        if(!h.read(value))return false;
        std::fprintf(this->fout,fmt,value);
        for(int i=1;i<DumpTraits<fmtc>::element_number;i++){
          std::fputc(',',this->fout);
          if(!h.read(value))return false;
          std::fprintf(this->fout,fmt,value);
        }
      
        std::fprintf(this->fout,"), ");
      }
      return true;
    }
    bool dumpA_array(ArrayTypeFormatNode* anode){
      IFormatNode* const node=anode->GetElementType();
      const int iN=anode->GetLength();

      bool const isatom=IFormatNode::isatom(node);
      this->print("{");
      this->indent_level+=2;
      if(!isatom){
        this->newline();
      }

      if(iN){
        for(int i=0;i<iN;i++)
          if(!this->dumpA(node))
            return false;
      }else{
        for(;;)
          if(!this->dumpA(node))break;
      }

      this->indent_level-=2;
      this->print("},");
      this->newline();
      return true;
    }
    bool readA_tuple(TupleTypeFormatNode* tnode){
      const int iN=tnode->GetLength();

      bool fAtom=true;
      this->print("{");
      this->indent_level+=2;
      for(int i=0;i<iN;i++){
        IFormatNode* const node=tnode->GetElementType(i);
        bool const isatom=IFormatNode::isatom(node);
        if(fAtom&&!isatom)
          this->newline();

        fAtom=isatom;

        if(!this->dumpA(node))
          return false;
      }
      this->indent_level-=2;
      this->print("},");
      this->newline();
      return true;
    }
  public:
    bool dumpA(IFormatNode* node){
      switch(node->GetFormatNodeType()){
      case FMTC::FLT1:return dumpA<FMTC::FLT1>();
      case FMTC::FLT2:return dumpA<FMTC::FLT2>();
      case FMTC::FLTX:return dumpA<FMTC::FLTX>();
      case FMTC::I1:return dumpA<FMTC::I1>();
      case FMTC::I2:return dumpA<FMTC::I2>();
      case FMTC::I4:return dumpA<FMTC::I4>();
      case FMTC::I8:return dumpA<FMTC::I8>();
      case FMTC::U1:return dumpA<FMTC::U1>();
      case FMTC::U2:return dumpA<FMTC::U2>();
      case FMTC::U4:return dumpA<FMTC::U4>();
      case FMTC::U8:return dumpA<FMTC::U8>();
      case FMTC::CPX1:return dumpA<FMTC::CPX1>();
      case FMTC::CPX2:return dumpA<FMTC::CPX2>();
      case FMTC::CPXX:return dumpA<FMTC::CPXX>();
      case FMTC::ERR1:return dumpA<FMTC::ERR1>();
      case FMTC::ERR2:return dumpA<FMTC::ERR2>();
      case FMTC::ERRX:return dumpA<FMTC::ERRX>();
      case FMTC::ARRAY:return dumpA_array(static_cast<ArrayTypeFormatNode*>(node));
      case FMTC::TUPLE:return readA_tuple(static_cast<TupleTypeFormatNode*>(node));
      default:
        return false;
      }
    }
//-----------------------------------------------------------------------------
  private:
    template<int fmtc>
    bool dumpB(){
      typedef typename DumpTraits<fmtc>::element_type element_type;
      std::fprintf(this->fout,"%s( ",DumpTraits<fmtc>::get_typename());

      const char* fmt=printf_format<element_type>::fmt();
      this->before_print();
      element_type value;
      if(!h.read(value))return false;
      std::fprintf(this->fout,fmt,value);

      if(DumpTraits<fmtc>::element_number==2){
        std::fprintf(this->fout,", ");
        if(!h.read(value))return false;
        std::fprintf(this->fout,fmt,value);
      }

      std::fprintf(this->fout," ), ");
      this->newline();
      return true;
    }
    bool dumpB_array(ArrayTypeFormatNode* anode){
      IFormatNode* const node=anode->GetElementType();
      const int iN=anode->GetLength();

      this->print("{");
      this->newline();
      this->indent_level+=2;

      if(iN){
        for(int i=0;i<iN;i++){
          this->before_print();
          std::fprintf(this->fout,"/* %d */ ",i);
          if(!this->dumpB(node))
            return false;
        }
      }else{
        for(mwg::i8t i=0;;i++){
          this->before_print();
          std::fprintf(this->fout,"/* %d */ ",i);
          if(!this->dumpB(node))break;
        }
      }

      this->indent_level-=2;
      this->print("},");
      this->newline();
      return true;
    }
    bool dumpB_tuple(TupleTypeFormatNode* tnode){
      const int iN=tnode->GetLength();

      bool fAtom=true;
      this->print("{");
      this->newline();
      this->indent_level+=2;
      for(int i=0;i<iN;i++){
        IFormatNode* const node=tnode->GetElementType(i);

        if(!this->dumpB(node))
          return false;
      }
      this->indent_level-=2;
      this->print("},");
      this->newline();
      return true;
    }
    void dumpB_printName(IFormatNode* node){
      const char* name=node->name.c_str();
      if(*name){
        this->print(name);
        std::fputc(':',this->fout);
        std::fputc(' ',this->fout);
      }
    }
  public:
    bool dumpB(IFormatNode* node){
      dumpB_printName(node);

      switch(node->GetFormatNodeType()){
      case FMTC::FLT1:return dumpB<FMTC::FLT1>();
      case FMTC::FLT2:return dumpB<FMTC::FLT2>();
      case FMTC::FLTX:return dumpB<FMTC::FLTX>();
      case FMTC::I1:return dumpB<FMTC::I1>();
      case FMTC::I2:return dumpB<FMTC::I2>();
      case FMTC::I4:return dumpB<FMTC::I4>();
      case FMTC::I8:return dumpB<FMTC::I8>();
      case FMTC::U1:return dumpB<FMTC::U1>();
      case FMTC::U2:return dumpB<FMTC::U2>();
      case FMTC::U4:return dumpB<FMTC::U4>();
      case FMTC::U8:return dumpB<FMTC::U8>();
      case FMTC::CPX1:return dumpB<FMTC::CPX1>();
      case FMTC::CPX2:return dumpB<FMTC::CPX2>();
      case FMTC::CPXX:return dumpB<FMTC::CPXX>();
      case FMTC::ERR1:return dumpB<FMTC::ERR1>();
      case FMTC::ERR2:return dumpB<FMTC::ERR2>();
      case FMTC::ERRX:return dumpB<FMTC::ERRX>();
      case FMTC::ARRAY:return dumpB_array(static_cast<ArrayTypeFormatNode*>(node));
      case FMTC::TUPLE:return dumpB_tuple(static_cast<TupleTypeFormatNode*>(node));
      default:
        return false;
      }
    }
//-----------------------------------------------------------------------------
  private:
    bool dumpF_typename_pref(IFormatNode* node){
      mwg::byte c=node->GetFormatNodeType();
      if('a'<=c&&c<='z'){
        const char* tname;
        switch(c){
        case FMTC::FLT1:tname=DumpTraits<FMTC::FLT1>::get_typename();break;
        case FMTC::FLT2:tname="double";break;
        case FMTC::FLTX:tname="long double";break;
        case FMTC::I1:tname="char";break;
        case FMTC::I2:tname="short";break;
        case FMTC::I4:tname="int";break;
        case FMTC::I8:tname="long";break;
        case FMTC::U1:tname="byte";break;
        case FMTC::U2:tname="word";break;
        case FMTC::U4:tname="dword";break;
        case FMTC::U8:tname="qword";break;
        case FMTC::CPX1:tname="complex<float>";break;
        case FMTC::CPX2:tname="complex<double>";break;
        case FMTC::CPXX:tname="complex<long double>";break;
        case FMTC::ERR1:tname="errored<float>";break;
        case FMTC::ERR2:tname="errored<double>";break;
        case FMTC::ERRX:tname="errored<long double>";break;
        default:tname="__unknown_type__";break;
        }
        this->print(tname);
        return true;
      }else{
        // std::fprintf(stderr,"dbg: dumpF_typename_pref: c='%c'\n",c);
        // std::fflush(stderr);
        switch(c){
        case FMTC::ARRAY:
          {
            ArrayTypeFormatNode* anode=static_cast<ArrayTypeFormatNode*>(node);
            dumpF_typename_pref(anode->GetElementType());
            return true;
          }
        case FMTC::TUPLE:
          {
            this->print("struct{");
            this->newline();
            this->indent_level+=2;

            TupleTypeFormatNode* tnode=static_cast<TupleTypeFormatNode*>(node);
            for(int i=0,iN=tnode->GetLength();i<iN;i++){
              IFormatNode* node2=tnode->GetElementType(i);

              dumpF_typename_pref(node2);
              dumpF_typename_suff(node2);
              std::fputc(';',this->fout);
              this->newline();
            }

            this->indent_level-=2;
            this->print("}");
            return true;
          }
        default:
          this->before_print();
          std::fprintf(this->fout,"__unknown_type__");
          return true;
        }
      }
    }
    bool dumpF_typename_suff(IFormatNode* node,bool suppressName=false){
      const char* memberName=node->name.c_str();
      if(*memberName=='\0'){
        if(!suppressName)
          std::fprintf(this->fout," _");
      }else{
        std::fprintf(this->fout,suppressName?" /* %s */":" %s",memberName);
      }

      mwg::byte c=node->GetFormatNodeType();
      if(c==FMTC::ARRAY){
        ArrayTypeFormatNode* anode=static_cast<ArrayTypeFormatNode*>(node);
        mwg::u8t const len=anode->GetLength();
        if(len>0)
          std::fprintf(this->fout,"[%d]",len);
        else
          std::fprintf(this->fout,"[]");

        dumpF_typename_suff(anode->GetElementType(),true);
      }

      return true;
    }
  public:
    bool dumpF(IFormatNode* node){
      dumpF_typename_pref(node);
      dumpF_typename_suff(node);
      std::fputc(';',this->fout);
      this->newline();
      return true;
    }
//-----------------------------------------------------------------------------
  public:
    bool dumpAF(IFormatNode* node){
      dumpF_typename_pref(node);
      dumpF_typename_suff(node);
      std::fputc('=',this->fout);
      return this->dumpA(node);
    }
  };
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
 
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}

//-----------------------------------------------------------------------------

namespace mwb=mwg::bio::mwb1;

void test1(){
  mwb::TupleTypeFormatNode* flows=new mwb::TupleTypeFormatNode;
  flows->AddMember(new mwb::ArrayTypeFormatNode(12,new mwb::AtomTypeFormatNode(mwb::FMTC::CPX1)));
  flows->AddMember(new mwb::AtomTypeFormatNode(mwb::FMTC::FLT1));
  flows->Reverse();
  mwb::ArrayTypeFormatNode* etas=new mwb::ArrayTypeFormatNode(18,flows);
  mwb::ArrayTypeFormatNode* evts=new mwb::ArrayTypeFormatNode(1,etas);

  mwg::bio::ftape tape("F:/data/hydrojet/mckln_lhc/harm.etaA.000+001k.mwb","rb");
  tape.seek(0x40);
  if(!tape){
    std::fprintf(stderr,"failed to open file!\n");
    std::exit(EXIT_FAILURE);
  }
  mwb::DumpExec(tape,stdout).dumpB(evts);

  delete evts;
  //mwb::IFormatNode* type=new mwb::ArrayTypeFormatNode(0,mwg::stdm::unique_ptr<mwb::IFormatNode>(new mwb::AtomTypeFormatNode('d')));
}

void test2(){
  const char* mwbfmt="<f\x83mul\"a\x81\x0C*\x84""flow\">\x85""flows=\x85""flows'\x81\x12*\x87""eta_bin\"*";
  //const char* mwbfmt="f\x81\x05*\x81\x02*\x85""hello\"";
  //const char* mwbfmt="<f\x81""a\"f>";
  mwg::bio::memory_tape t(mwbfmt,std::strlen(mwbfmt)+1);

  mwg::stdm::unique_ptr<mwb::IFormatNode> node(mwb::read_format(t));
  if(node.get()==nullptr){
    std::fprintf(stderr,"failed to read format specs.\n");
    std::exit(EXIT_FAILURE);
  }

  mwb::DumpExec(t,stdout).dumpF(node.get());
}

void test3(){
  mwg::bio::ftape tape("F:/data/hydrojet/mckln_lhc/harm.etaA.000+001k.mwb","rb");
  if(!tape){
    std::fprintf(stderr,"failed to open file!\n");
    std::exit(EXIT_FAILURE);
  }

  mwg::bio::tape_head<mwg::bio::ftape> head(tape);
  mwg::u4t magic;
  if(!head.read(magic)||magic!=mwb::MAGIC){
    std::fprintf(stderr,"the file is not a valid .mwb file!\n");
    std::exit(EXIT_FAILURE);
  }

  mwg::u4t version;
  if(!head.read(version)||version!=mwb::MWBVER){
    std::fprintf(stderr,"the .mwb version is incorrect!\n");
    std::exit(EXIT_FAILURE);
  }
  
  mwg::u8t size;
  if(!head.read(size)){
    std::fprintf(stderr,"failed to read the size of mwb structure!\n");
    std::exit(EXIT_FAILURE);
  }

  mwg::stdm::unique_ptr<mwb::IFormatNode> node(mwb::read_format(tape));
  if(node.get()==nullptr){
    std::fprintf(stderr,"failed to read format specs.\n");
    std::exit(EXIT_FAILURE);
  }

// std::fprintf(stderr,"dbg: offset = %lld\n",tape.tell());
// std::fflush(stderr);

  //mwb::DumpExec(tape,stdout).dumpAF(node.get());
  mwb::DumpExec(tape,stdout).dumpB(node.get());
}

int main(){
  test3();
  return 0;
}
