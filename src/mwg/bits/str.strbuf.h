// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BITS_STR_STRBUFF_H
#define MWG_BITS_STR_STRBUFF_H
#include <mwg/defs.h>
#include <mwg/str.h>
#include <string>

namespace mwg{
  namespace str_detail{
    template<typename XCH>
    struct strbuf_policy;
    template<typename XCH>
    class strbuf;
  }
  using str_detail::strbuf;
}

namespace mwg{
namespace str_detail{

  template<typename XCH>
  struct strbuf_policy{
    typedef strbuf_policy    policy_type;
    typedef XCH              char_type;
    typedef const char_type& char_reference;
    static const bool has_get_ptr=true;

    typedef char_type const* const_iterator;

    typedef char_traits<char_type> char_traits_type;

    class buffer_type{
      std::basic_string<XCH> m_str;

    public:
      buffer_type(){}

    public:
      template<typename StrP>
      buffer_type(strbase<StrP> const& s){
        this->reset(s);
      }

      template<typename StrP>
      void reset(strbase<StrP> const& s){
        this->m_str.reserve(s.length);
        this->m_str.clear();
        this->m_str.append(s.begin(),s.end());
      }

    public:
      buffer_type(std::basic_string<XCH> const& source):m_str(source){}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
      buffer_type(std::basic_string<XCH>&& source):m_str(stdm::move(source)){}
#endif

    public:
      template<typename Iter>
      buffer_type(Iter const& begin,Iter const& end):m_str(begin,end){}

    public:
      char_reference operator[](std::size_t index) const{
        return m_str[index];
      }
      std::size_t length() const{
        return m_str.size();
      }
      const_iterator begin   ()                     const{return const_iterator(&m_str[0]);}
      const_iterator end     ()                     const{return const_iterator(this->begin()+m_str.size());}
      const_iterator begin_at(std::ptrdiff_t index) const{return const_iterator(this->begin()+index);}

      const char_type* get_ptr() const{
        return &m_str[0];
      }

    public:
      void reserve(std::size_t capacity){this->m_str.reserve(capacity);}
      std::size_t capacity() const{return this->m_str.capacity();}
      void clear(){this->m_str.clear();}

      void append(char_type ch){this->m_str+=ch;}

      template<typename Iter>
      void append(Iter const& begin,Iter const& end){this->m_str.append(begin,end);}

      template<typename Str>
      typename mwg::as_str<Str,XCH>::template enable<void>::type
      append(Str const& _str){
        typename mwg::as_str<Str,XCH>::adapter str(_str);
        this->m_str.append(_str.begin(),_str.end());
      }
    };
  };

  template<typename XCH>
  class strbuf:public strbase<strbuf_policy<XCH> >{
    typedef strbase<strbuf_policy<XCH> > base;

  public:
    typedef typename base::char_type char_type;

  public:
    strbuf(){}

    strbuf(strbuf const& s):base(s.data){}
    strbuf& operator=(strbuf const& rhs){
      this->data=rhs.data;
      return *this;
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    strbuf(strbuf&& s):base(mwg::stdm::move(s.data)){}
    strbuf& operator=(strbuf&& rhs){
      this->data=mwg::stdm::move(rhs.data);
      return *this;
    }
#endif

    template<typename YStr>
    strbuf(YStr const& src,typename as_str<YStr,char_type>::template enable<mwg::invalid_type*>::type=0)
      :base(mwg::str(src)){}
    template<typename YStr>
    typename as_str<YStr,char_type>::template enable<strbuf&>::type
    operator=(YStr const& rhs){
      this->data.reset(mwg::str(rhs));
      return *this;
    }

  public:
    void reserve(std::size_t capacity){this->data.reserve(capacity);}
    std::size_t capacity() const{return this->data.capacity();}
    void clear(){this->data.clear();}

    strbuf& append(char_type ch){
      this->data.append(ch);
      return *this;
    }

    template<typename Iter>
    strbuf& append(Iter const& begin,Iter const& end){
      this->data.append(begin,end);
      return *this;
    }

    template<typename Str>
    typename mwg::as_str<Str,XCH>::template enable<strbuf&>::type
    append(Str const& _str){
      this->data.append(_str);
      return *this;
    }
  };
}
}
#endif
