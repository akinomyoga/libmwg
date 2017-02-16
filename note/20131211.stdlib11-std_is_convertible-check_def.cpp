#include <mwg/std/type_traits>

#ifdef MWGCONF_STD_RVALUE_REFERENCES
template<typename From>
typename mwg::stdm::add_rvalue_reference<From>::type create();
#else
template<typename From>
From create();
#endif

template<typename From,typename To>
To test(){return create<From>();}

void test_is_convertible_to_reference(){

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

#ifdef _MSC_VER
  // vc2008, vc2010 共に以下の結果であった

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
#else
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
