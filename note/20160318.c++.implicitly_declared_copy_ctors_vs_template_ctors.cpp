#include <cstdio>

/* テンプレートのコンストラクタを定義した時、
 * copy/move コンストラクタはコンパイラが生成する物が使用されるのか、
 * テンプレートのコンストラクタが使用されるのか。
 */

template<typename T>
struct klass{
  const char* m_str;
  klass():m_str("<default>"){}
  klass(const char* str):m_str(str){}

  template<typename U>
  klass(klass<U> const& src):m_str(src.m_str){
    std::printf("%s: template copy-ctor is called.\n",m_str);
  }

  template<typename U>
  klass& operator=(klass<U> const& src){
    this->m_str=src.m_str;
    std::printf("%s: template copy-assign is called.\n",m_str);
    return *this;
  }

#if defined(__GXX_EXPERIMENTAL_CXX0X__)||__cplusplus>=201103L
  template<typename U>
  klass(klass<U>&& src):m_str(src.m_str){
    std::printf("%s: template move-ctor is called.\n",m_str);
  }

  template<typename U>
  klass& operator=(klass<U>&& src){
    this->m_str=src.m_str;
    std::printf("%s: template move-assign is called.\n",m_str);
    return *this;
  }
#endif
};

int main(){
  klass<int> a1("b1");
  klass<int> a2("b2");
  klass<int>  b1(a1); // template copy ctor/copy ctor の両方呼べる?
  klass<char> b2(a2); // template copy ctor だけしか呼べない。
  klass<int>  c1((klass<int>("c1"))); // template move ctor/move ctor の両方呼べる?
  klass<char> c2((klass<int>("c2"))); // template move ctor だけしか呼べない。
  // ※注意: 二重括弧はプロトタイプ宣言と思われないため。

  b1=a1; // template copy assign/copy assign の両方呼べる?
  b2=a2; // template copy assign だけしか呼べない。
  c1=klass<int>("c1"); // template move assign/move assign の両方呼べる?
  c2=klass<int>("c2"); // template move assign だけしか呼べない。

  /* 結果 (clang++-3.7.0 g++-5.3.1 g++-2.95.3 vc9(2008))
   * b1: copy ctor/assign
   * b2: template copy ctor/assign
   * c1: move ctor/assign
   * c2: template move ctor/assign
   *
   * 従って、テンプレートのコンストラクタよりも
   * コンパイラが自動生成するコンストラクタの方が優先されるという事である。
   *
   * 結果 (icc-14.0)
   * b1: copy ctor/assign
   * b2: template copy ctor/assign
   * c1: move ctor & template move assign ★ なんと template の方が呼び出された。
   * c2: template move ctor/assign
   *
   * Intel C++ Compiler (icpc) が別の結果を出した。これはどうするか。
   * (最新版でも直らないのか。)
   *
   */

  std::printf("completed\n");
  return 0;
}
