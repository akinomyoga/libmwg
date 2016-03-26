
struct MyClass{
  template<typename T> void f() const{}
};

// case1
template<typename T>
static void g(MyClass const& obj){
  obj.f<int>();

  // 回避方法
  //
  // obj.template f<int>();
}

void case1(){
  g<int>(MyClass());
}

// case2
template<typename T>
struct C2{
  static void g(MyClass const& obj){
    obj.f<int>();

    // 回避方法
    // obj.template f<int>();
  }
};

void case2(){
  C2<int>::g(MyClass());
}

int main(){
  case1();
  case2();
  return 0;
}
