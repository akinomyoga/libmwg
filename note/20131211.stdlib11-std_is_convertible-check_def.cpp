// -*- mode: c++; coding: cp932 -*-
#include <mwg/std/type_traits>
#include <cassert>

#ifdef MWGCONF_STD_RVALUE_REFERENCES
template<typename From>
typename mwg::stdm::add_rvalue_reference<From>::type create();
#else
template<typename From>
From create();
#endif


template<typename From, typename To>
To test() {return create<From>();}

void test_is_convertible_to_integer() {
  static_assert(__is_convertible_to(int, signed char), "int -> signed_char");
}

void test_is_convertible_to_reference() {
  //
  // int   ,int   &  ---
  // int   ,int c &  VGI
  // int   ,int  v&  ---
  // int   ,int cv&  V--
  // int c ,int   &  ---
  // int c ,int c &  VGI
  // int c ,int  v&  ---
  // int c ,int cv&  V--
  // int  v,int   &  ---
  // int  v,int c &  --I
  // int  v,int  v&  ---
  // int  v,int cv&  V--
  // int cv,int   &  ---
  // int cv,int c &  --I
  // int cv,int  v&  ---
  // int cv,int cv&  V--

  // MSC ... is_convertible<F,T&>::value = is_const<T>::value&&(is_volatile<T>::value||!is_volatile<F>::value);
  // ICC ... is_convertible<F,T&>::value = is_const<T>::value&&(!is_volatile<T>::value);
  // GCC ... is_convertible<F,T&>::value = is_const<T>::value&&(!is_volatile<T>::value&&!is_volatile<F>::value);

#if defined(_MSC_VER) && (1500 <= _MSC_VER && _MSC_VER <= 1800)
  // msc15 (_MSC_VER 1500) (VS2008)
  // msc16 (_MSC_VER 1600) (VS2010)
  // msc17 Microsoft(R) C/C++ Optimizing Compiler Version 17.00.60610.1 for x86 (VS2012)
  // msc18 Microsoft(R) C/C++ Optimizing Compiler Version 18.00.40629 for x86   (VS2013)
  //
  //    | & | c&| v&|cv&|
  // ---+---+-----------+
  // -- | x | o | x | o |
  // c- | x | o | x | o |
  // cv | x | x | x | o |
  // -v | x | x | x | o |
  //

  //test<int,int&>();//NG
  test<int,const int&>();
  //test<int,volatile int&>(); // NG
  test<int,const volatile int&>();

  // test<const int,int&>(); // NG
  test<const int,const int&>();
  // test<const int,volatile int&>(); //NG
  test<const int,const volatile int&>();

  // test<volatile int,int&>(); // NG
  // test<volatile int,const int&>(); // NG
  // test<volatile int,volatile int&>(); // NG
  test<volatile int,const volatile int&>();

  // test<const volatile int,int&>(); // NG
  // test<const volatile int,const int&>(); // NG
  // test<const volatile int,volatile int&>(); // NG
  test<const volatile int,const volatile int&>();

  static_assert(!(mwg::stdm::is_convertible<int, int&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int const, int&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int const, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int const, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int const, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int volatile, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int volatile const, int const volatile&>::value), "");
#elif defined(_MSC_VER) && (_MSC_VER == 1900)
  // msc19 Microsoft(R) C/C++ Optimizing Compiler Version 19.00.24213.1 for x86 (VS2016)
  //
  // 実際に変換できるかどうかのパターンはこれまでの msc15-msc18 と同様である。
  // しかし msc19 で std::is_convertible の返す値だけが変なパターンに変化した。
  //
  // 実際に変換できるか     std::is_convertible
  //
  //    | & | c&| v&|cv&|      | & | c&| v&|cv&|
  // ---+---+-----------+   ---+---+-----------+
  // -- | x | o | x | o |   -- | x | o | x | x |
  // c- | x | o | x | o |   c- | x | o | x | x |
  // cv | x | x | x | o |   cv | x | x | x | x |
  // -v | x | x | x | o |   -v | x | x | x | x |

  //test<int,int&>();//NG
  test<int,const int&>();
  //test<int,volatile int&>(); // NG
  test<int,const volatile int&>();
  // test<const int,int&>(); // NG
  test<const int,const int&>();
  // test<const int,volatile int&>(); //NG
  test<const int,const volatile int&>();
  // test<volatile int,int&>(); // NG
  // test<volatile int,const int&>(); // NG
  // test<volatile int,volatile int&>(); // NG
  test<volatile int,const volatile int&>();
  // test<const volatile int,int&>(); // NG
  // test<const volatile int,const int&>(); // NG
  // test<const volatile int,volatile int&>(); // NG
  test<const volatile int,const volatile int&>();

  static_assert(!(std::is_convertible<int, int const volatile&>::value), ""); // !!!
  static_assert(!(std::is_convertible<int const, int const volatile&>::value), ""); // !!!
  static_assert(!(std::is_convertible<int volatile, int const volatile&>::value), ""); // !!!
  static_assert(!(std::is_convertible<int volatile const, int const volatile&>::value), ""); // !!!

  static_assert(!(mwg::stdm::is_convertible<int, int&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int const, int&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int const, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int const, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int const, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int volatile, int const volatile&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int const&>::value), "");
  static_assert(!(mwg::stdm::is_convertible<int volatile const, int volatile&>::value), "");
  static_assert( (mwg::stdm::is_convertible<int volatile const, int const volatile&>::value), "");
#else
  // const& には変換できるけれども
  // 他には変換できないというパターンである。
  // 変換元の CV qualifier は関係ない。
  //
  //    | & | c&| v&|cv&|
  // ---+---+-----------+
  // -- | x | o | x | x |
  // c- | x | o | x | x |
  // cv | x | o | x | x |
  // -v | x | o | x | x |
  //

  //test<int,int&>();
  test<int,const int&>();
  //test<int,volatile int&>();
  //test<int,const volatile int&>();
  //test<const int,int&>();
  test<const int,const int&>();
  //test<const int,volatile int&>();
  //test<const int,const volatile int&>();
  //test<volatile int,int&>();
  test<volatile int,const int&>();
  //test<volatile int,volatile int&>();
  //test<volatile int,const volatile int&>();
  //test<const volatile int,int&>();
  test<const volatile int,const int&>();
  //test<const volatile int,volatile int&>();
  //test<const volatile int,const volatile int&>();

#endif
}
