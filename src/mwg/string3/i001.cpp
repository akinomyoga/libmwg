#include <mwg/except.h>
#include <mwg/functor.h>
#include <mwg/string.h>

void test(){
  mwg_assert( (mwg::stradp<char>("hello")=="hello"));
  mwg_assert(!(mwg::stradp<char>("hello")!="hello"));
  mwg_assert(!(mwg::stradp<char>("hello")=="world"));
  mwg_assert( (mwg::stradp<char>("hello")!="world"));
  mwg_assert(!(mwg::stradp<char>("hello")=="hell"));
  mwg_assert( (mwg::stradp<char>("hello")!="hell"));

  mwg_assert(!(mwg::stradp<char>("hello")<"hello"));
  mwg_assert(!(mwg::stradp<char>("hello")>"hello"));
  mwg_assert( (mwg::stradp<char>("hello")<"world"));
  mwg_assert(!(mwg::stradp<char>("hello")>"world"));
  mwg_assert(!(mwg::stradp<char>("hello")<"hell"));
  mwg_assert( (mwg::stradp<char>("hello")>"hell"));

  mwg_assert( (mwg::stradp<char>("hello")<="hello"));
  mwg_assert( (mwg::stradp<char>("hello")>="hello"));
  mwg_assert( (mwg::stradp<char>("hello")<="world"));
  mwg_assert(!(mwg::stradp<char>("hello")>="world"));
  mwg_assert(!(mwg::stradp<char>("hello")<="hell"));
  mwg_assert( (mwg::stradp<char>("hello")>="hell"));

  mwg::string<char> s1;
  mwg_assert( (s1==""));
  mwg::string<char> s2="012345";
  mwg_assert( (s2=="012345"));
  s1="21345";
  mwg_assert( (s1=="21345"));

  typedef mwg::stradp<char> _a;
  mwg_assert( (_a("hello").toupper()=="HELLO"));
  mwg_assert( (_a("hello").toupper(2)=="heLLO"));
  mwg_assert( (_a("hello").toupper(1,-1)=="hELLo"));
  mwg_assert( (_a("HELLO").tolower()=="hello"));
  mwg_assert( (_a("HELLO").tolower(2)=="HEllo"));
  mwg_assert( (_a("HELLO").tolower(1,-1)=="HellO"));
}

void test_concat(){
  typedef mwg::stradp<char> _a;
  mwg_assert((_a("hello")+_a(" world")=="hello world"));
  mwg_assert((_a("hello")+" world"=="hello world"));
  mwg_assert(("hello"+_a(" world")+"!"=="hello world!"));
}
void test_slice(){
  typedef mwg::stradp<char> _a;
  mwg_assert((mwg::stradp<char>("hello").slice(2,4)=="ll"));
  mwg_assert((mwg::stradp<char>("hello").slice(1,-2)=="el"));
  mwg_assert((mwg::stradp<char>("hello").slice(3)=="lo"));

  mwg_assert((mwg::stradp<char>("0123456789").slice(-6,-3)=="456"));
  mwg_assert((mwg::stradp<char>("0123456789").slice(-3)=="789"));
  mwg_assert((mwg::stradp<char>("0123456789").slice(6,4)==""));
  mwg_assert((mwg::stradp<char>("0123456789").slice(6,-6)==""));

  mwg_assert((_a("hello").remove(3)=="hel"));
  mwg_assert((_a("hello").remove(1,-2)=="hlo"));
  mwg_assert((_a("hello").remove(mwg::make_range(-4,-1))=="ho"));
}
void test_trim(){
  typedef mwg::stradp<char> _a;
  mwg_assert((_a("  hello   ").trim()=="hello"));
  mwg_assert((_a("  hello   ").ltrim()=="hello   "));
  mwg_assert((_a("  hello   ").rtrim()=="  hello"));
  mwg_assert((_a("012343210").trim("012")=="343"));
  mwg_assert((_a("012343210").ltrim("012")=="343210"));
  mwg_assert((_a("012343210").rtrim("012")=="012343"));
  mwg_assert((_a("012343210").trim(_a("012"))=="343"));
  mwg_assert((_a("012343210").ltrim(_a("012"))=="343210"));
  mwg_assert((_a("012343210").rtrim(_a("012"))=="012343"));

  mwg_assert((_a("hello").pad(1)=="hello"));
  mwg_assert((_a("hello").lpad(1)=="hello"));
  mwg_assert((_a("hello").rpad(1)=="hello"));
  mwg_assert((_a("hello").pad(10)=="  hello   "));
  mwg_assert((_a("hello").lpad(10)=="     hello"));
  mwg_assert((_a("hello").rpad(10)=="hello     "));
  mwg_assert((_a("hello").pad(10,'-')=="--hello---"));
  mwg_assert((_a("hello").lpad(10,'-')=="-----hello"));
  mwg_assert((_a("hello").rpad(10,'-')=="hello-----"));
}
void test_starts(){
  typedef mwg::stradp<char> _a;
  mwg_assert( (_a("hello world").starts("hel")));
  mwg_assert( (_a("hello world").starts(_a("hel"))));
  mwg_assert(!(_a("hello world").starts("hal")));
  mwg_assert(!(_a("hello world").starts(_a("hal"))));
  mwg_assert(!(_a("hello world").starts("hello world!")));
  mwg_assert(!(_a("hello world").starts(_a("hello world!"))));
  mwg_assert( (_a("hello world").ends("orld")));
  mwg_assert( (_a("hello world").ends(_a("orld"))));
  mwg_assert(!(_a("hello world").ends("olrd")));
  mwg_assert(!(_a("hello world").ends(_a("olrd"))));
  mwg_assert(!(_a("hello world").ends("+hello world")));
  mwg_assert(!(_a("hello world").ends(_a("+hello world"))));
}
void test_find(){
  typedef mwg::stradp<char> _a;
  mwg_assert((_a("0123401234").find("012")==0));
  mwg_assert((_a("0123401234").find("234")==2));
  mwg_assert((_a("0123401234").find("021")<0));
  mwg_assert((_a("0123401234").find("012",1)==5));
  mwg_assert((_a("0123401234").find("012",1,8)==5));
  mwg_assert((_a("0123401234").find("012",1,7)<0));
  mwg_assert((_a("0123401234").rfind("012")==5));
  mwg_assert((_a("0123401234").rfind("234")==7));
  mwg_assert((_a("0123401234").rfind("021")<0));
  mwg_assert((_a("0123401234").rfind("012",1)==5));
  mwg_assert((_a("0123401234").rfind("012",1,8)==5));
  mwg_assert((_a("0123401234").rfind("012",1,7)<0));
  mwg_assert((_a("0123401234").rfind("012",6)<0));
}
void test_replace(){
  typedef mwg::stradp<char> _a;
  mwg_assert((_a("hello").replace('l','c')=="hecco"));
  mwg_assert((_a("hello").replace('l','p').replace('e','i')=="hippo"));
  mwg_assert((_a("hello").replace('l','p',-2)=="helpo"));
  mwg_assert((_a("hello").replace('l','r',0,3)=="herlo"));

  mwg_assert((_a("hello").replace(1,-3,"icon")=="hiconllo"));
  mwg_assert((_a("hello").replace(1,-3,_a("icon"))=="hiconllo"));
  mwg_assert((_a("hello").replace(mwg::make_range(1,-3),"icon")=="hiconllo"));
  mwg_assert((_a("hello").insert(1,"icon")=="hiconello"));
  mwg_assert((_a("hello").insert(1,_a("icon"))=="hiconello"));
}
// namespace string_bench{
//   int test_compare1();
//   void test(){
//     for(mwg::i8t i=0;i<10000000LL;i++)
//       string_bench::test_compare1();
//   }
// }

int main(){
  test();
  test_concat();
  test_slice();
  test_trim();
  test_starts();
  test_find();
  test_replace();

  return 0;
}
