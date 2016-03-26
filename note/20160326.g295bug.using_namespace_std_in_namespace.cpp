// 20160326.g295bug.using_namespace_std_in_namespace.cpp

// 注釈
//
// namespace std 以外ならば OK
// #define std ns1

// 回避方法
//
// 使う可能性のある物を全て一つ一つ using するしかない?

namespace std{void mwg_test_f(const char* str){}}
namespace ns2{using namespace std;}
int main(){
  // using namespace std; している筈なのに見えない。
  ns2::mwg_test_f("hello");
  return 0;
}
