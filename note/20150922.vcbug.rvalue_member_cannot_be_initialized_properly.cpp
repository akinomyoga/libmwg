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
    // ����`���̃R���X�g���N�^
    int&& a3=(int&&)(a2);                 mwg_printd("a3=%p",&a3); // NG
    int&& a3r=reinterpret_cast<int&&>(a2);mwg_printd("a3r=%p",&a3r); // NG
    int&& a3s=static_cast<int&&>(a2);     mwg_printd("a3s=%p",&a3s); // NG
    // int& a3p=*(int*)(&a2);
    // mwg_printd("a3p=%p",&a3p);
  }
  void test_bind2(int& a2){
    // �֐��`���̃R���X�g���N�^
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

    // print_rvalue("a3p",(int)*(int*)(&a2)); // ���ƃR���p�C���ł��Ȃ��c
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
      // return ���̓����ŃR���X�g���N�^���Ăяo���ƉE�Ӓl�Q�Ƃ͂���ւ��B
      return B("f1",fwd<int>(a),fwd<int>(b),&a,&b);
    }
    B f2(int&& a,int&& b){
      // ��U���[�J���ϐ��ŃR���X�g���N�^���Ăяo����OK
      B ret("f2",fwd<int>(a),fwd<int>(b),&a,&b);
      return std::move(ret);
    }

    // �R���X�g���N�^�ł͂Ȃ��Ēʏ�̊֐��̌ďo�ł͉����N����Ȃ�
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

    // �P��̈����̃R���X�g���N�^���Ƒ��v�ȗl���B
    A h1(int&& a){
      return A(fwd<int>(a));
    }
    A h2(int&& a){
      A ret(fwd<int>(a));
      return std::move(ret);
    }
    // �E�Ӓl�Q�Ƃ���ł��A�����̈���������ꍇ�ɂ͑ʖڂȂ悤���B
    A h1a(int&& a){
      return A("h1a",fwd<int>(a),&a);
    }
    A h2a(int&& a){
      A ret("h2a",fwd<int>(a),&a);
      return std::move(ret);
    }

    // ���� return �̓����ɂȂ������Ƃ��Ă��󒆂� A() �̗l�ɍ\�z����Ƒʖڂȗl��!
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

    // vc �ł́��ŃA�h���X���ς�� (�V�����ꏊ�ɃR�s�[�����)
    // ������ forward �ł��Ȃ� / �������� (int&&) �Ƃ��������� move ����Ē��g�������Ȃ�B
    // ����ɂ��Ă� std::vector<int> v{1,2,3};std::move(v); ���Ƃ�������
    // �����N���邩�ȂǑz������ƌ����ɖ�肪��������l�Ɏv���̂ŁA�ŐV�łł͏C������Ă���Ǝv�������B
    //

    // test_bind1(a2);
    // test_bind2(a2);
    // test_argument(a2);
    // test_return(a2);
    // test_struct(a2);
    test_return_construct();
  }
}

// �܂Ƃ� @ VC++ 2010
//
//   - �ϐ��� bind ����ƃA�h���X���ω�����B
//   - reinterpret_cast<T&&>() ����ƃA�h���X���ω�����B
//   - �E�Ӓl�Q�ƃ����o������������ƃA�h���X���ω�����B
//   - (1) �����̈��������� (2) �R���X�g���N�^�� (3) �ϐ��𔺂�Ȃ����������� �Ăяo���ƈ����̃A�h���X���ω�����B
//
//     ��L�O�������S�đ��������ɃA�h���X���ω����Ă��܂��l�ł���B
//     �Ⴆ�� return MyClass(1); ���Ƃ� int a=MyClass(1).count(); ���ł���B
//     ��������̃R���X�g���N�^�Ȃ�Α��v�ȗl�ł��� (�E�Ӓl�Q�ƂȈ����̐��ł͂Ȃ��đS���̈����̐�)�B
//     �R���X�g���N�^�ȊO�̊֐��̌ďo�Ȃ���v�ȗl�ł���B
//     �ϐ��̏������Ƃ��Ă̌ďo�Ȃ���v�ȗl�ł���B�Ⴆ�� MyClass ret(1); ���͖��͋N����Ȃ��B
//
//   �⑫
//   - (��L return ���ȊO�Ȃ�) �����ɓn���̂� OK�B
//   - �߂�l�ɓn���̂� OK�B
//   - static_cast<T&&>() �� (T&&)() �� OK
//   - �R���X�g���N�^�̈����Ŏ󂯎��̂� OK �ł���B
//   - �ȒP�� forward ���������Ă݂� (��L fwd) ������͐����������Ă���l�Ɍ�����B
//
//   �ȉ��œ��l�̖��ɂ��ďq�ׂ��Ă���:
//   http://boost.2283326.n4.nabble.com/VC10-config-some-regressions-in-several-libraries-td2662089.html
//
//   �����o�̏��������ł��Ȃ��Ƃ����̂͒v���I�ł���B
//   �Ⴆ�� std::forward_as_tuple(1) �̖߂�l�̌^�� tuple<int&&> �ł��邪�A�߂�l�̏������Ɏ��s����B
//   ���݂ɒ��ׂĂ݂��� std::forward_as_tuple �� VC 2010 �ł͗p�ӂ���Ă��Ȃ��l�ł���B
//
// ���̍ŏ���
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
// �������@?
//
//   ��L�R�[�h�� C++ �I�ɂ͐��������Ȃ̂Ő݌v��ύX���鎖�Ȃ��A
//   VC �̎������ŏ����̏C���œ��삷��l�ɕύX�������B
//   reference_wrapper �őΏ�����Ƃ���������邪�A
//   std::tuple, std::pair �Ȃǂɂ�������Ȃ���΂Ȃ�Ȃ������l����ƁA
//
//   �����I�� rvalue_reference_wrapper �I�Ȃ��̂��g���͖̂ʓ|�ł���B
//   �܂� reference_wrapper �ɂ���ꍇ�A�E�Ӓl�Q�Ƃ��R�s�[�ł��Ȃ��Ȃǂ�
//   �ׂ�������ɂ��Ă��͕킷��K�v������B
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
