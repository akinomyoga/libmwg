// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_ARGUMENTREADER
#define MWG_ARGUMENTREADER
#include <cstdio>
#include <cstdarg>
#include <cstring>
namespace mwg{

class ArgumentReader{
protected:
  const char* const progname;
private:
  int argc;
  char** argv;
  bool err;
  int i;
  int option;
  char* optionname;
public:
  ArgumentReader(const char* progname):progname(progname){}
  operator bool() const{return !err;}
protected:
  //void report_argerr(const char* message){
  //  std::fprintf(stderr,"%s:(arg%d)! %s (%s)\n",progname,i,message,argv[i]);
  //  this->err=true;
  //}
  void report_argerr(const char* format,...){
    std::fprintf(stderr,"%s:(arg%d)! ",progname,i);
    va_list va_args;
    va_start(va_args,format);
    std::vfprintf(stderr,format,va_args);
    va_end(va_args);
    std::fprintf(stderr," (%s)\n",argv[i]);
    this->err=true;
  }
public:
  //void report_error(const char* name,const char* message){
  //  std::fprintf(stderr,"%s:%s! %s\n",progname,name,message);
  //  this->err=true;
  //}
  void report_error(const char* name,const char* format,...){
    std::fprintf(stderr,"%s:%s! ",progname,name);
    va_list va_args;
    va_start(va_args,format);
    std::vfprintf(stderr,format,va_args);
    va_end(va_args);
    std::fprintf(stderr,"\n");
    this->err=true;
  }
protected:
  virtual void print_usage()=0;
  virtual bool is_func(int c){return false;}
  virtual void proc_func(int c,const char* arg){}
  virtual bool proc_flag(int c){return false;}
  virtual void proc_arg(const char* arg)=0;
  virtual int resolve_longname(const char* arg){return 0;}
  virtual void check_args(){}
private:
  // [return: 指定された引数の読み取りが終了したかどうか]
  bool read_option(char*& arg){
    int c=*arg++;
    if(c=='\0'){
      report_argerr("incomplete option");
      return true;
    }

    if(c=='-'&&*arg!='\0'){
      // --longname=param 分離
      char* p1=std::strchr(arg,'=');
      char* p2=std::strchr(arg,':');
      if(p1==NULL||p2!=NULL&&p2<p1)p1=p2;
      if(p1!=NULL)*p1++='\0';

      // longname
      c=resolve_longname(arg);
      if(c==0){
        report_argerr("unrecongnized longname option '%s'",arg);
        return true;
      }

      if(proc_flag(c)){
        if(p1)report_argerr("option '%s' requires no extra parameters",arg);
        return true;
      }

      if(is_func(c)){
        option=c;
        optionname=arg;
        if(!p1)return true;
        arg=p1;
        return false;
      }
    }else{
      // flag
      while(proc_flag(c)){
        if((c=*arg++)=='\0')return true;
      }

      // func
      if(is_func(c)){
        option=c;
        if(*arg=='\0')return true;
        if(*arg==':'||*arg=='=')arg++;
        return false;
      }
    }

    // ???
    report_argerr("unrecognized option '%c'",c);
    return true;
  }
public:
  bool read(int argc,char** argv){
    // init
    this->argc=argc;
    this->argv=argv;
    this->err=false;

    option=0;
    optionname=NULL;
    for(i=1;i<argc;i++){
      char* arg=argv[i];
      if(!option){
        if(arg[0]=='\0')continue;
        if(arg[0]=='-'){
          arg++;
          if(read_option(arg))continue;
        }
      }

      if(option){
        proc_func(option,arg);
      }else{
        proc_arg(arg);
      }

      option=0;
      optionname=NULL;
    }

    // check
    if(option!=0){
      if(optionname)
        report_argerr("missing extra-argument for option '%s'",optionname);
      else
        report_argerr("missing extra-argument for option '%c'",(char)option);
    }
    check_args();

    if(err)print_usage();
    return !err;
  }
};

}
#endif
