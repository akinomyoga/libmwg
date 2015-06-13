// -*- mode:C++;coding:utf-8 -*-
#ifndef MWG_BIO_TTX2_H
#define MWG_BIO_TTX2_H
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <mwg/std/memory>
#include <mwg/std/utility>
#include <mwg/cast.h>
#include <mwg/exp/utils.h>
#include <mwg/exp/iprint.h>
#include "defs.h"

# include <mwg/impl/warning_push.inl>
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  class ttx_attr;
  class ttx_node;

  namespace ttx2_detail{
    static std::string ttx_empty_string;

    void ttx_scan(ttx_node& root,const char* ttx);
    void ttx_scan(ttx_node& root,std::istream& ttx);
    void ttx_scan(ttx_node& root,FILE* ttx);
    void ttx_scan(ttx_node& root,const mwg::bio::itape& ttx);
    mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(const char* ttx);
    mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(std::istream& ttx);
    mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(FILE* ttx);
    mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(const mwg::bio::itape& ttx);

    bool getnode(const char* ttx,ttx_node& node,bool untilIsolatedClosingBrace=false);
    bool getnode(std::istream& ttx,ttx_node& node,bool untilIsolatedClosingBrace=false);
    bool getnode(FILE* ttx,ttx_node& node,bool untilIsolatedClosingBrace=false);
    bool getnode(mwg::bio::itape& ttx,ttx_node& node,bool untilIsolatedClosingBrace=false);

    // 属性操作用一時オブジェクト
    class _tmpobj__ttx_attr_manipulator{
      ttx_node* node;
      std::string const& attrname;
    private:
      friend class mwg::bio::ttx_node;
      _tmpobj__ttx_attr_manipulator(ttx_node* node,const std::string& attrname)
        :node(node),attrname(attrname){}
    public:
      operator bool() const;
      std::string const& get() const;
      void set(std::string const& value);

    public:
      bool empty() const{return !this->operator bool();}
      bool operator!() const{return !this->operator bool();}

      operator std::string const&() const{
        return this->get();
      }
      std::string const& str() const{
        return this->get();
      }
      const char* c_str() const{
        return this->get().c_str();
      }
      template<typename T>
      T get() const{
        return mwg::lexical_cast<T>(this->get());
      }

      _tmpobj__ttx_attr_manipulator& operator=(const std::string& value){
        this->set(value);
        return *this;
      }
    };
  }

  using ttx2_detail::getnode;

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  class ttx_attr{
  public:
    std::string name;
    std::string value;
  public:
    template<typename A1,typename A2>
    ttx_attr(A1 mwg_forward_rvalue name,A2 mwg_forward_rvalue value)
      :name(stdm::forward<A1>(name)),value(stdm::forward<A2>(value)){}
  };

  class ttx_node:public mwg::exp::iprint{
  private:
    std::string m_name;
    ttx_node*   m_parent;
    int         m_index;
  protected:
    std::vector<ttx_attr>  m_attrs;
    std::vector<ttx_node*> m_nodes;
  public:
    const std::string& name  () const{return this->m_name;}
    ttx_node*          parent() const{return this->m_parent;}
    int                index () const{return this->m_index;}
    typedef std::vector<ttx_attr>::const_iterator  attr_iterator;
    typedef std::vector<ttx_node*>::const_iterator node_iterator;
    const std::vector<ttx_attr>&  attrs() const{return this->m_attrs;}
    const std::vector<ttx_node*>& nodes() const{return this->m_nodes;}

  public:
    ttx_node()
      :m_parent(nullptr),m_index(-1),m_name("xml"){}
    explicit ttx_node(const char* _name)
      :m_parent(nullptr),m_index(-1),m_name(_name){}
    explicit ttx_node(const std::string& _name)
      :m_parent(nullptr),m_index(-1),m_name(_name){}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    explicit ttx_node(std::string&& _name)
      :m_parent(nullptr),m_index(-1),m_name(stdm::move(_name)){}
#endif
  private:
    void clear(){
      for(std::vector<ttx_node*>::iterator i=this->m_nodes.begin();i!=this->m_nodes.end();++i){
        (*i)->m_parent=nullptr;
        (*i)->m_index=-1;
        delete *i;
      }
      this->m_nodes.clear();
      this->m_attrs.clear();
    }
  public:
    ~ttx_node(){
      this->clear();
      if(this->m_parent!=nullptr)
        this->m_parent->release_node(this->m_index);
    }

  private:
    void addContentOf(ttx_node const& src){
      ttx_node& dst(*this);

      ttx_node::attr_iterator p=src.attrs().begin();
      ttx_node::attr_iterator pN=src.attrs().end();
      for(;p<pN;++p){
        dst.attr(p->name)=p->value;
      }

      ttx_node::node_iterator ps=src.nodes().begin();
      ttx_node::node_iterator psN=src.nodes().end();
      for(;ps<psN;++ps){
        dst.add_node(new ttx_node(**ps));
      }
    }
  public:
    ttx_node(ttx_node const& src)
      :m_parent(nullptr),m_index(-1),m_name(src.m_name)
    {
      this->addContentOf(src);
    }
  public:
    ttx_node& operator=(ttx_node const& src){
      this->clear();
      this->m_name=src.m_name;
      this->addContentOf(src);
      return *this;
    }

  //----------------------------------------------------------------------------
  // m_nodes
  //----------------------------------------------------------------------------
  public:
    ttx_node* release_node(int index){
      if(index<0||index>=m_nodes.size())return nullptr; // throw

      ttx_node* const _node=m_nodes[index];
      _node->m_parent=nullptr;
      _node->m_index=-1;

      m_nodes.erase(m_nodes.begin()+index);
      typedef std::vector<ttx_node*>::iterator it_t;
      for(it_t i=m_nodes.begin()+index,iN=m_nodes.end();i<iN;i++)
        (*i)->m_index=index++;

      return _node;
    }
    ttx_node* release_node(ttx_node* _node){
      if(_node->m_parent!=this)return nullptr; // throw
      return this->release_node(_node->m_index);
    }
    ttx_node* add_node(ttx_node* _node){
      if(_node->m_parent!=nullptr)
        _node->m_parent->release_node(_node);
      
      _node->m_parent=this;
      _node->m_index=m_nodes.size();
      m_nodes.push_back(_node);
      return _node;
    }
  public:
    ttx_node* create_node(const char* _name){
      return this->add_node(new ttx_node(_name));
    }
    ttx_node* create_node(const std::string& _name){
      return this->add_node(new ttx_node(_name));
    }
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    ttx_node* create_node(std::string&& _name){
      return this->add_node(new ttx_node(std::move(_name)));
    }
#endif
    void remove_node(int _index){
      delete this->release_node(_index);
    }
    void remove_node(ttx_node* _node){
      if(_node->m_parent!=this)return; // throw
      this->remove_node(_node->m_index);
    }
  public:
    void load(const char* ttx){
      mwg::bio::ttx2_detail::ttx_scan(*this,ttx);
    }
    void load(std::istream& ttx){
      mwg::bio::ttx2_detail::ttx_scan(*this,ttx);
    }
    static mwg::exp::enumerator<ttx_node*> load_and_enumerate(const char* ttx){
      return mwg::bio::ttx2_detail::ttx_scan_enumerate_node(ttx);
    }
    static mwg::exp::enumerator<ttx_node*> load_and_enumerate(std::istream& ttx){
      return mwg::bio::ttx2_detail::ttx_scan_enumerate_node(ttx);
    }
  //----------------------------------------------------------------------------
  // m_attrs
  //----------------------------------------------------------------------------
  private:
    friend class ttx2_detail::_tmpobj__ttx_attr_manipulator;
    const std::string& _internal_getAttr(const std::string& _name) const{
      std::vector<ttx_attr>::const_iterator i=m_attrs.begin(),iN=m_attrs.end();
      for(;i<iN;i++)
        if(i->name==_name)return i->value;
      return mwg::bio::ttx2_detail::ttx_empty_string;
    }
    bool _internal_hasAttr(const std::string& _name) const{
      std::vector<ttx_attr>::const_iterator i=m_attrs.begin(),iN=m_attrs.end();
      for(;i<iN;i++)
        if(i->name==_name)return true;
      return false;
    }
    void _internal_setAttr(const std::string& _name,const std::string& value){
      std::vector<ttx_attr>::iterator i=m_attrs.begin(),iN=m_attrs.end();
      for(;i<iN;i++){
        if(i->name==_name){
          i->value=value;
          return;
        }
      }
      m_attrs.push_back(ttx_attr(_name,value));
    }
  public:
    ttx2_detail::_tmpobj__ttx_attr_manipulator
    attr(std::string const& attrName){
      return ttx2_detail::_tmpobj__ttx_attr_manipulator(this,attrName);
    }
    ttx2_detail::_tmpobj__ttx_attr_manipulator const
    attr(std::string const& attrName) const{
      return ttx2_detail::_tmpobj__ttx_attr_manipulator(const_cast<ttx_node*>(this),attrName);
    }
  //----------------------------------------------------------------------------
  // enumeration
  //----------------------------------------------------------------------------
  public:
    mwg::exp::enumerator<ttx_node*> enumerate_nodes(bool recursive=false);
    mwg::exp::enumerator<ttx_node*> enumerate_nodes(const char* _name,bool recursive=false);
    mwg::exp::enumerator<ttx_node*> enumerate_nodes(const std::string& _name,bool recursive=false){
      return this->enumerate_nodes(_name.c_str(),recursive);
    }
    ttx_node* node(const char* _name,bool recursive=false) const{
      for(std::vector<ttx_node*>::const_iterator i=m_nodes.begin(),iN=m_nodes.end();i<iN;i++){
        if((*i)->m_name==_name)return *i;
        if(recursive){
          ttx_node* ret=(*i)->node(_name,recursive);
          if(ret)return ret;
        }
      }
      return nullptr;
    }
    mwg::exp::enumerator<ttx_attr&> enumerate_attrs(){
      return new mwg::exp::range_enumerator<std::vector<ttx_attr>::iterator>(m_attrs.begin(),m_attrs.end());
    }
    mwg::exp::enumerator<ttx_attr const&> enumerate_attrs() const{
      return new mwg::exp::range_enumerator<std::vector<ttx_attr>::const_iterator>(m_attrs.begin(),m_attrs.end());
    }
  //----------------------------------------------------------------------------
  // merge
  //----------------------------------------------------------------------------
  public:
    static ttx_node* clone(const ttx_node& n1){
      return new ttx_node(n1);
    }
    static ttx_node* merge(const ttx_node& n1,const ttx_node& n2);
    void add_merge(const ttx_node& n1,const ttx_node& n2){
      ttx_node* m=ttx_node::merge(n1,n2);
      if(m!=nullptr){
        this->add_node(m);
      }else{
        this->add_node(new ttx_node(n1));
        this->add_node(new ttx_node(n2));
      }
    }
  //----------------------------------------------------------------------------
  // iprint::print
  //----------------------------------------------------------------------------
  public:
    typedef iprint base;
    using base::print;
    void print(std::ostream& ostr,mwg::exp::sfmt::xml_t* tag) const;
    void print(std::ostream& ostr,mwg::exp::sfmt::ttx_t* tag) const;
    void print(std::ostream& ostr,mwg::exp::sfmt::json_t* tag) const;
    virtual void print(std::ostream& ostr,const mwg::exp::iformat& fmt) const{
      if(typeid(fmt)==typeid(mwg::exp::sfmt::xml_t))
        this->print(ostr,mwg::exp::sfmt::xml);
      else if(typeid(fmt)==typeid(mwg::exp::sfmt::ttx_t))
        this->print(ostr,mwg::exp::sfmt::ttx);
      else if(typeid(fmt)==typeid(mwg::exp::sfmt::json_t))
        this->print(ostr,mwg::exp::sfmt::json);
      else
        this->base::print(ostr,fmt);
    }
  };


  namespace ttx2_detail{

    // class _tmpobj__ttx_attr_manipulator;
    inline _tmpobj__ttx_attr_manipulator::operator bool() const{
      return this->node->_internal_hasAttr(this->attrname);
    }
    inline std::string const& _tmpobj__ttx_attr_manipulator::get() const{
      return this->node->_internal_getAttr(this->attrname);
    }
    inline void _tmpobj__ttx_attr_manipulator::set(std::string const& value){
      return this->node->_internal_setAttr(this->attrname,value);
    }
  }


  class ttx_root:public ttx_node{
  public:
    ttx_root():ttx_node("xml"){}
  //----------------------------------------------------------------------------
  // iprint::print
  //----------------------------------------------------------------------------
  public:
    using mwg::exp::iprint::print;
    void print(std::ostream& ostr,mwg::exp::sfmt::xml_t* tag) const{
      ostr<<"<?xml version=\"1.0\"?>\n";
      typedef std::vector<ttx_node*>::const_iterator node_it;
      for(node_it i=m_nodes.begin(),iN=m_nodes.end();i!=iN;++i){
        (*i)->print(ostr,tag);
      }
    }
    void print(std::ostream& ostr,mwg::exp::sfmt::ttx_t* tag) const{
      typedef std::vector<ttx_node*>::const_iterator node_it;
      for(node_it i=m_nodes.begin(),iN=m_nodes.end();i!=iN;++i)
        (*i)->print(ostr,tag);
    }
    virtual void print(std::ostream& ostr,const mwg::exp::iformat& fmt) const{
      if(typeid(fmt)==typeid(mwg::exp::sfmt::xml_t))
        this->print(ostr,mwg::exp::sfmt::xml);
      else if(typeid(fmt)==typeid(mwg::exp::sfmt::ttx_t))
        this->print(ostr,mwg::exp::sfmt::ttx);
      else
        this->base::print(ostr,fmt);
    }
  };
//==============================================================================

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
# include <mwg/impl/warning_pop.inl>
#endif
