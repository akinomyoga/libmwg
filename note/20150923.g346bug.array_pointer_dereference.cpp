// -*- mode:c++;coding:utf-8 -*-
//
// g++-3 で (T(&)[N])(*(T(*)[N])0) ができない。
// つまり、配列へのポインタを間接参照して配列への参照を構築できない。
//

// gcc-3 で以下がコンパイルできない
typedef const char (&return_type)[2];
return_type f1(const char (*p)[2]){
  return return_type(*p);
}

// 以下はコンパイルできる
return_type f2(const char (*p)[2]){
  return_type ret(*p);
  return ret;
}

void test1(){
  const char a[2]="a";
  f1(&a);
  f2(&a);
}

int main(){
  test1();
  return 0;
}
