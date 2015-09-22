// -*- coding:cp932 -*-
#include <type_traits>
#include <utility>
#include <mwg/except.h>

namespace test1{
  void print_rvalue(const char* name,int&& arg){
    mwg_printd("%s=%p",name,&arg);
  }

  int&& pass_rvalue(int&& arg){return (int&&)arg;}
  int&& reinterpret_rvalue(int&& arg){return reinterpret_cast<int&&>(arg);}
  int&& static_rvalue(int&& arg){return static_cast<int&&>(arg);}

  template<typename T>
  T&& fwd(typename std::remove_reference<T>::type& value){
    return static_cast<T&&>(value);
  }
  template<typename T>
  T&& fwd(typename std::remove_reference<T>::type&& value){
    return static_cast<T&&>(value);
  }

  void test_bind1(int& a2){
    // 代入形式のコンストラクタ
    int&& a3=(int&&)(a2);                 mwg_printd("a3=%p",&a3); // NG
    int&& a3r=reinterpret_cast<int&&>(a2);mwg_printd("a3r=%p",&a3r); // NG
    int&& a3s=static_cast<int&&>(a2);     mwg_printd("a3s=%p",&a3s); // NG
    // int& a3p=*(int*)(&a2);
    // mwg_printd("a3p=%p",&a3p);
  }
  void test_bind2(int& a2){
    // 関数形式のコンストラクタ
    int&& a3((int&&)(a2));                 mwg_printd("a3=%p",&a3); // NG
    int&& a3r(reinterpret_cast<int&&>(a2));mwg_printd("a3r=%p",&a3r); // NG
    int&& a3s(static_cast<int&&>(a2));     mwg_printd("a3s=%p",&a3s); // NG
  }

  void test_argument(int& a2){
    // print_rvalue("a3",(int&&)(a2)); // OK
    // print_rvalue("a3r",reinterpret_cast<int&&>(a2)); // NG
    // print_rvalue("a3s",static_cast<int&&>(a2)); // OK
  }
  void test_return(int& a2){
    // print_rvalue("a3r",pass_rvalue(reinterpret_cast<int&&>(a2))); // NG
    // print_rvalue("a3r",static_rvalue(reinterpret_cast<int&&>(a2))); // NG
    // print_rvalue("a3r",reinterpret_rvalue(reinterpret_cast<int&&>(a2))); // NG

    // print_rvalue("a3" ,pass_rvalue((int&&)(a2))); // OK
    // print_rvalue("a3" ,static_rvalue((int&&)(a2))); // OK
    // print_rvalue("a3" ,reinterpret_rvalue((int&&)(a2))); // NG

    // print_rvalue("a3s",pass_rvalue(static_cast<int&&>(a2))); // OK
    // print_rvalue("a3s",static_rvalue(static_cast<int&&>(a2))); // OK
    // print_rvalue("a3s",reinterpret_rvalue(static_cast<int&&>(a2))); // NG

    print_rvalue("a3" ,fwd<int>(a2)); // OK
    print_rvalue("a3" ,fwd<int>((int&&)(a2))); // OK
    print_rvalue("a3s",fwd<int>(static_cast<int&&>(a2))); // OK

    // print_rvalue("a3p",(int)*(int*)(&a2)); // 何とコンパイルできない…
  }

  struct rvalue_holder{
    int&& rvalue;
    int&& get() const{return (int&&)this->rvalue;}

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4413)
#endif
    rvalue_holder(int&& _rvalue):rvalue((int&&)(_rvalue)){
      mwg_printd("&_rvalue=%p",&_rvalue);
      mwg_printd("&rvalue=%p",&rvalue);
    }
#ifdef _MSC_VER
# pragma warning(pop)
#endif
  };

  void print_rvalue(const char* name,rvalue_holder const& holder){
    mwg_printd("%s=%p",name,&holder.rvalue);
  }

  void test_struct(int& a2){
    print_rvalue("h1",rvalue_holder((int&&)a2));
  }

  namespace return_construct_detail{
    struct B{
      int *p,*q;
      B(const char* tag,int&& a,int&& b,int* pa,int* pb):p(&a),q(&b){
        if(!mwg_check_nothrow(&a==pa&&&b==pb,"tag=%s",tag)){
          mwg_printd("Caller(a=%p,b=%p)",pa,pb);
          mwg_printd("Callee(a=%p,b=%p)",&a,&b);
        }
      }
    };
    B f1(int&& a,int&& b){
      // return 式の内部でコンストラクタを呼び出すと右辺値参照はすり替わる。
      return B("f1",fwd<int>(a),fwd<int>(b),&a,&b);
    }
    B f2(int&& a,int&& b){
      // 一旦ローカル変数でコンストラクタを呼び出せばOK
      B ret("f2",fwd<int>(a),fwd<int>(b),&a,&b);
      return std::move(ret);
    }

    // コンストラクタではなくて通常の関数の呼出では何も起こらない
    int hoge(const char* tag,int&& a,int&& b,int* pa,int* pb){
      if(!mwg_check_nothrow(&a==pa&&&b==pb,"tag=%s",tag)){
        mwg_printd("Caller(a=%p,b=%p)",pa,pb);
        mwg_printd("Callee(a=%p,b=%p)",&a,&b);
      }
      return 123;
    }
    int g1(int&& a,int&& b){
      return hoge("g1",fwd<int>(a),fwd<int>(b),&a,&b);
    }
    int g2(int&& a,int&& b){
      int ret=hoge("g2",fwd<int>(a),fwd<int>(b),&a,&b);
      return ret;
    }

    struct A{
      int *p;
      A(const char* tag,int&& a,int* pa):p(&a){
        this->check(tag,pa);
      }
      A(int&& a):p(&a){}
      void check(const char* tag,int* pa) const{
        if(!mwg_check_nothrow(p==pa,"tag=%s",tag)){
          mwg_printd("Caller(a=%p)",pa);
          mwg_printd("Callee(a=%p)",p);
        }
      }
      int value() const{return *p;}
    };

    // 単一の引数のコンストラクタだと大丈夫な様だ。
    A h1(int&& a){
      return A(fwd<int>(a));
    }
    A h2(int&& a){
      A ret(fwd<int>(a));
      return std::move(ret);
    }
    // 右辺値参照が一個でも、複数の引数がある場合には駄目なようだ。
    A h1a(int&& a){
      return A("h1a",fwd<int>(a),&a);
    }
    A h2a(int&& a){
      A ret("h2a",fwd<int>(a),&a);
      return std::move(ret);
    }

    // 何と return の内部になかったとしても空中で A() の様に構築すると駄目な様だ!
    int h1s(int&& a){
      return A("h1s",fwd<int>(a),&a).value()+1;
    }
    int h2s(int&& a){
      int ret=A("h2s",fwd<int>(a),&a).value()+1;
      return ret;
    }
  }
  void test_return_construct(){
    using namespace return_construct_detail;
    f1(1,2);f2(1,2);
    g1(1,2);g2(1,2);

    int a;
    h1(std::move(a)).check("h1",&a);
    h2(std::move(a)).check("h2",&a);
    h1a(1);h2a(1);
    h1s(1);h2s(1);
  }

  void test(){
    int a1=1234;mwg_printd("a1=%p",&a1);
    int& a2=a1; mwg_printd("a2=%p",&a2);

    // vc では↓でアドレスが変わる (新しい場所にコピーされる)
    // 正しく forward できない / もしくは (int&&) としただけで move されて中身が無くなる。
    // 何れにしても std::vector<int> v{1,2,3};std::move(v); 等とした時に
    // 何が起こるかなど想像すると現実に問題が発生する様に思うので、最新版では修正されていると思いたい。
    //

    // test_bind1(a2);
    // test_bind2(a2);
    // test_argument(a2);
    // test_return(a2);
    // test_struct(a2);
    test_return_construct();
  }
}

// まとめ @ VC++ 2010
//
//   - 変数に bind するとアドレスが変化する。
//   - reinterpret_cast<T&&>() するとアドレスが変化する。
//   - 右辺値参照メンバを初期化するとアドレスが変化する。
//   - (1) 複数の引数を持つ (2) コンストラクタを (3) 変数を伴わない部分式から 呼び出すと引数のアドレスが変化する。
//
//     上記三条件が全て揃った時にアドレスが変化してしまう様である。
//     例えば return MyClass(1); だとか int a=MyClass(1).count(); 等である。
//     引数が一個のコンストラクタならば大丈夫な様である (右辺値参照な引数の数ではなくて全部の引数の数)。
//     コンストラクタ以外の関数の呼出なら大丈夫な様である。
//     変数の初期化としての呼出なら大丈夫な様である。例えば MyClass ret(1); 等は問題は起こらない。
//
//   補足
//   - (上記 return 式以外なら) 引数に渡すのは OK。
//   - 戻り値に渡すのも OK。
//   - static_cast<T&&>() や (T&&)() は OK
//   - コンストラクタの引数で受け取るのも OK である。
//   - 簡単な forward を実装してみた (上記 fwd) がそれは正しく動いている様に見える。
//
//   以下で同様の問題について述べられている:
//   http://boost.2283326.n4.nabble.com/VC10-config-some-regressions-in-several-libraries-td2662089.html
//
//   メンバの初期化ができないというのは致命的である。
//   例えば std::forward_as_tuple(1) の戻り値の型は tuple<int&&> であるが、戻り値の初期化に失敗する。
//   因みに調べてみた所 std::forward_as_tuple は VC 2010 では用意されていない様である。
//
// 問題の最小化
//

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4413)
#endif

struct A{
  int&& value;
  A(int&& _value):value(static_cast<int&&>(_value)){
    mwg_printd("&_value=%p",&_value);
    mwg_printd("&value=%p",&value);
  }
};

#ifdef _MSC_VER
# pragma warning(pop)
#endif

//
// 解決方法?
//
//   上記コードは C++ 的には正しい筈なので設計を変更する事なく、
//   VC の時だけ最小限の修正で動作する様に変更したい。
//   reference_wrapper で対処するという手もあるが、
//   std::tuple, std::pair などにも手を入れなければならない事を考えると、
//
//   内部的に rvalue_reference_wrapper 的なものを使うのは面倒である。
//   また reference_wrapper にする場合、右辺値参照がコピーできないなどの
//   細かい動作についても模倣する必要がある。
//

namespace test2{

  template<typename T>
  struct rvalue_reference_wrapper{
    T* rvalue;
    rvalue_reference_wrapper(T&& _value)
      :rvalue(&_value)
    {
      mwg_printd("&_value=%p",&_value);
      mwg_printd("rvalue=%p",rvalue);
    }

    T* operator&() const{return this->rvalue;}
    T&& get() const{return static_cast<T&&>(*this->rvalue);}
    operator T&&() const{return static_cast<T&&>(*this->rvalue);}
  };

  struct A{
    rvalue_reference_wrapper<int> value;
    A(int&& _value):value(static_cast<int&&>(_value)){
      mwg_printd("&_value=%p",&_value);
      mwg_printd("&value=%p",&this->value);
    }
  };

  void test(){
    A a(1234);
  }
}


int main(){
  //A a(123);

  test1::test();
  //test2::test();

  return 0;
}
