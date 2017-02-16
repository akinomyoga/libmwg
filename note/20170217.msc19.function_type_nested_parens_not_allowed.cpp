#include <cassert>

template<typename A, typename B> struct is_same {static const bool value = false;};
template<typename A> struct is_same<A, A> {static const bool value = true;};

//
// gcc は int((int)) の形式の関数型を受け付けるが、msc はエラーになる。
// 規格的にはどの様な扱いになっているのだろう。
// そして、括弧を許す様な型名を渡すにはどの様にすれば良いのだろうか。
// → <concept.h> mwg::concept_detail::quote によって囲む事にした。
//

int main() {
  assert((is_same<int(), int()>::value));
  assert((is_same<int(int), int(int)>::value));
  assert((is_same<int(int), int((int))>::value));
  return 0;
}
