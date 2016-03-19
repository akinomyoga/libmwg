// -*- mode:C++;coding:utf-8 -*-
#pragma%x begin_check
#include <vector>
#include <mwg/except.h>

class managed_test{
public:
  virtual void test()=0;
  managed_test(){testerList.push_back(this);}

private:
  static std::vector<managed_test*> testerList;
public:
  static void run_tests(){
    for(int i=0,iN=testerList.size();i<iN;i++){
      // mwg_printd("_test%d",i);
      testerList[i]->test();
    }
  }
};
std::vector<managed_test*> managed_test::testerList;

#pragma%[itest=0]
#pragma%m begin_test
#pragma%%x begin_check
#pragma%%x
class _test$"itest":managed_test{
#pragma%%end.i
#pragma%end
#pragma%m end_test
#pragma%%x
} _test$"itest"_instance;
#pragma%%end.i
#pragma%%x end_check
#pragma%%[itest++]
#pragma%end

// usage
// #pragma%x begin_check
// int main(){
//   managed_test::run_tests();
//   return 0;
// }
// #pragma%x end_check

#pragma%x end_check
