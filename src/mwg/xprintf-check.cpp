#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <mwg/except.h>
#include <mwg/std/cmath>
#include "xprintf2.h"

void dbg_enumerate_fmtspec(const char* fmt){
  for(;*fmt;){
    if(*fmt=='%'){
      mwg::xprintf_detail::fmtspec spec;
      mwg::xprintf_detail::read_fmtspec(spec,fmt);
      std::printf(
        "(flg=%02o w=%d p=%d conv=%c)",
        spec.flags,
        spec.width,
        spec.precision,
        spec.conv?spec.conv:'$');
    }else{
      std::putchar(*fmt);
      fmt++;
    }
  }

}

template<typename... Args>
void test_printf(const char* expectedResult,const char* fmt,Args&&... args){
  std::string buff;
  int n=mwg::vxprintf(buff,fmt,mwg::vararg::va_forward<Args...>(args...));
  mwg_assert(
    buff==expectedResult,
    "fmt=%s: result=(%s:%d) expected=(%s:%d)",
    fmt,buff.c_str(),buff.size(),
    expectedResult,std::strlen(expectedResult));
  mwg_assert((std::size_t)n==buff.size(),"fmt=%s: len: calculated=%d actual=%d (%s)",fmt,n,buff.size(),buff.c_str());
}

void test1(){
  std::ostringstream ss;
  mwg::xprintf(ss,"(%.6d,%6o,%#-5x,%d)\n",100,100,100,"a");
  std::cout<<ss.str()<<std::flush;

  mwg::xprintf(stdout,"(%'*d)\n",30,123456789);

  std::string buff;
  mwg::xprintf(buff,"(%'*d)\n",30,123456789);

  mwg::xprintf(stdout,"%x %#x\n",123,123);
  mwg::xprintf(stdout,"%o %#o %o %#o\n",0,0,0644,0644);

  // test of continue '%'
  mwg::xprintf(
    stdout,
    "# hello world\n%",
    "%d %d %d\n%",1,2,3,
    "%d %d %d %1$d\n",-4,-3,-2
  );

  // test of default size
  mwg::xprintf(
    stdout,
    "d=%1$d u=%1$u x=%1$x X=%1$X o=%1$o\n"
    "d=%2$d u=%2$u x=%2$x X=%2$X o=%2$o\n"
    "d=%3$d u=%3$u x=%3$x X=%3$X o=%3$o\n"
    "d=%4$d u=%4$u x=%4$x X=%4$X o=%4$o\n%5$",
    mwg::i1t(-1),mwg::i2t(-1),mwg::i4t(-1),mwg::i8t(-1),
    "d=%1$d u=%1$u x=%1$x X=%1$X o=%1$o\n"
    "d=%2$d u=%2$u x=%2$x X=%2$X o=%2$o\n"
    "d=%3$d u=%3$u x=%3$x X=%3$X o=%3$o\n"
    "d=%4$d u=%4$u x=%4$x X=%4$X o=%4$o\n",
    mwg::i1t(-1234567890),mwg::i2t(-1234567890),mwg::i4t(-1234567890),mwg::i8t(-1234567890)
  );

  // test of size spec
  mwg::xprintf(
    stdout,
    "hhd=%1$hhd hd=%1$hd ld=%1$ld lld=%1$lld\n"
    "hhx=%1$hhx hx=%1$hx lx=%1$lx llx=%1$llx\n",
    mwg::i8t(-1234567890)
  );

  mwg::xprintf(stdout,"pi=%f 1=%f\n",M_PI,1.0);
  mwg::xprintf(stdout,"pi=%.20f\n",M_PI);

  mwg::xprintf(stdout,"0.1=%.100f\n",0.1);
  mwg::xprintf(stdout,"0.01=%.100f\n",0.01);

  mwg::xprintf(stdout,"bool: %s %S\n",true,false);
}

void test2(){
  // %d
  test_printf("0 -1","%d %d",0,-1);
  test_printf("12345 -12345" ,"%d %d",12345,-12345);
  test_printf("+12345 -12345","%+d %+d",12345,-12345);
  test_printf(" 12345 -12345","% d % d",12345,-12345);
  test_printf("   12345","%8d",12345);
  test_printf("12345   ","%-8d",12345);
  test_printf("00012345","%08d",12345);
  test_printf("00012345","%.8d",12345);
  test_printf("  012345","%8.6d",12345);
  test_printf("012345  ","%-8.6d",12345);
  test_printf("12,345"  ,"%'d",12345);
  test_printf("  012345","%*.*d",8,6,12345);

  // %x %o
  test_printf("1e240 1E240" ,"%x %1$X",123456);
  test_printf("0x1e240 0X1E240" ,"%#x %1$#X",123456);
  test_printf("361100 0361100" ,"%o %1$#o",123456);
  test_printf("0 0" ,"%o %1$#o",0);
  test_printf(" 0  0" ,"%2o %1$#2o",0);
  test_printf("00 00" ,"%02o %1$02o",0);

  // %f %e
  test_printf("1.234568","%f",1.2345678);
  test_printf("12.345678","%f",12.345678);
  test_printf("123.456780","%f",123.45678);
  test_printf("1234567.800000","%f",1234567.8);
  test_printf("12345678.000000","%f",12345678);

  // -carry
  test_printf("0.999999 1.000000 9.999994e-001 9.999996e-001"  ,"%f %f %1$e %2$e",0.9999994,0.9999996);
  test_printf("0.991999 0.992000 9.919994e-001 9.919996e-001"  ,"%f %f %1$e %2$e",0.9919994,0.9919996);
  test_printf("0.000000 0.000001 4.000000e-007 5.000000e-007"  ,"%f %f %1$e %2$e",0.0000004,0.0000005);
  test_printf("1.000000 2.000000 1.000000e+000 2.000000e+000"  ,"%f %f %1$e %2$e",1.0000000,2.0000000);
  test_printf("0.100000 0.200000 1.000000e-001 2.000000e-001"  ,"%f %f %1$e %2$e",0.1000000,0.2000000);
  test_printf("10.000000 20.000000 1.000000e+001 2.000000e+001","%f %f %1$e %2$e",10.0,20.0);
  test_printf("1.000000","%f",0.9999999);
  test_printf("0.999999","%f",0.9999991);
  test_printf("0.000244","%f",0.0002436);
  test_printf("0.244","%.3f",0.2436912);

  // -grouping
  test_printf("1.234568","%'f",1.2345678);
  test_printf("12.345678","%'f",12.345678);
  test_printf("123.456780","%'f",123.45678);
  test_printf("1,234,567.800000","%'f",1234567.8);
  test_printf("12,345,678.000000","%'f",12345678);

  // %g
  test_printf("1.23456","%g",1.2345612345);
  test_printf("0.000243612","%g",0.000243612);
  test_printf("243612","%g",243612.0);
  test_printf("2.43612e+006","%g",2436120.0);
  test_printf("2.43612E-005","%G",0.0000243612);
  test_printf("1","%g",0.9999999);
  test_printf("0.999999","%g",0.9999991);
  test_printf("99.9999","%g",99.9999);
  test_printf("99.9999","%#g",99.9999);

  test_printf("1.00000e+006 1e+006","%#g %1$g",999999.9);
  test_printf("100000. 100000","%#g %1$g",99999.99);
  test_printf("10000.0 10000" ,"%#g %1$g",9999.999);
  test_printf("100.000 100"   ,"%#g %1$g",99.99999);
  test_printf("10.0000 10"    ,"%#g %1$g",9.999999);
  test_printf("1.00000 1"     ,"%#g %1$g",0.9999999);
  test_printf("0.100000 0.1"  ,"%#g %1$g",0.09999999);
  test_printf("0.0100000 0.01","%#g %1$g",0.009999999);

  test_printf("999999. 999999"       ,"%#g %1$g",999999.0);
  test_printf("99999.9 99999.9"      ,"%#g %1$g",99999.9);
  test_printf("9999.99 9999.99"      ,"%#g %1$g",9999.99);
  test_printf("99.9999 99.9999"      ,"%#g %1$g",99.9999);
  test_printf("9.99999 9.99999"      ,"%#g %1$g",9.99999);
  test_printf("0.999999 0.999999"    ,"%#g %1$g",0.999999);
  test_printf("0.0999999 0.0999999"  ,"%#g %1$g",0.0999999);
  test_printf("0.00999999 0.00999999","%#g %1$g",0.00999999);

  // zero
  test_printf("0.000000 0.000000e+000","%f %e",0.0,0.0);
  test_printf("0.00000 0","%#g %1$g",0.0);

  // nan/inf
  test_printf("inf INF +inf -INF","%f %1$F %1$+f %F",1.0/0.0,-1.0/0.0);
  test_printf("inf INF +inf -INF","%e %1$E %1$+e %E",1.0/0.0,-1.0/0.0);
  test_printf("inf INF +inf -INF","%g %1$G %1$+g %G",1.0/0.0,-1.0/0.0);
  test_printf("inf INF +inf -INF","%a %1$A %1$+a %A",1.0/0.0,-1.0/0.0);

  double nan=std::sqrt(-1.0);
  test_printf("nan NAN +nan NAN","%f %1$F %1$+f %F",nan,-nan);
  test_printf("nan NAN +nan NAN","%e %1$E %1$+e %E",nan,-nan);
  test_printf("nan NAN +nan NAN","%g %1$G %1$+g %G",nan,-nan);
  test_printf("nan NAN +nan NAN","%a %1$A %1$+a %A",nan,-nan);

  mwg_printd("completed");
}

int main(){
  dbg_enumerate_fmtspec("aiueo kakikukeko %s %10.12s %#010.123f\n");
  test1();
  test2();
  return 0;
}
