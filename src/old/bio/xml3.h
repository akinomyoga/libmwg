// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_IO_XML_H
#define MWG_IO_XML_H
#include <map>
#include <string>
#include <vector>
#include "mwg/exp/utils.h"
#include "mwg/std/memory"
namespace mwg{
namespace bio{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  struct XmlNodeType:mwg::exp::enum_class_base<XmlNodeType>{
    enum enum_t{
      Document,
      Element,
      Attribute,
      Text,
    };

    typedef mwg::exp::enum_class_base<XmlNodeType> base;
    typedef base::underlying_type underlying_type;
    explicit XmlNodeType(const underlying_type& i):base(i){}
    XmlNodeType(const enum_t& e):base((underlying_type)e){}
    operator enum_t() const{return (enum_t)this->base::value;}
  };

//==============================================================================
  static std::string empty_string;

  struct XmlNode;
  struct XmlAttribute;
  struct XmlText;
  struct XmlElement;

  class XmlNode{
  protected:
    XmlElement* parent;
    int index;
    friend class XmlElement;
  public:
    XmlNode():parent(nullptr),index(0){}
    XmlElement* get_parent() const{return this->parent;}
    virtual XmlNodeType nodeType() const=0;
    virtual ~XmlNode(){}
    virtual void to_str(std::ostream& ostr) const=0;
  };

  struct XmlAttribute:public XmlNode{
    std::string name;
    std::string value;
  public:
    XmlAttribute(){}
    explicit XmlAttribute(const std::string& name,const std::string& value):name(name),value(value){}
    virtual XmlNodeType nodeType() const{return XmlNodeType::Attribute;}
  public:
    virtual void to_str(std::ostream& ostr) const{
      ostr<<this->name<<"=\""<<this->value<<"\""; // TODO: escape
    }
  };

  struct XmlText:public XmlNode{
    std::string data;
    virtual XmlNodeType nodeType() const{return XmlNodeType::Text;}
    XmlText(){}
    XmlText(const std::string& data):data(data){}
  public:
    virtual void to_str(std::ostream& ostr) const{
      ostr<<this->data; // TODO: espace
    }
  };

  struct XmlElement:public XmlNode{
    //typedef stdm::shared_ptr<XmlNode> XmlNode;
  public:
    std::string name;
    std::vector<XmlAttribute> attributes;
    std::vector<XmlNode*> childNodes;
    virtual XmlNodeType nodeType() const{return XmlNodeType::Element;}
  //----------------------------------------------------------------------------
  // ctor/dtor
  public:
    XmlElement(const char* tagName):name(tagName){}
    XmlElement(const std::string& tagName):name(tagName){}
#ifdef MWGCONF_STD_RVALUE_REFERENCES
    XmlElement(std::string&& tagName)
      :name(stdm::forward<std::string&&>(tagName)){}
#endif
    ~XmlElement(){
      typedef std::vector<XmlNode*>::iterator it_t;
      for(it_t i=childNodes.begin(),iN=childNodes.end();i<iN;i++)delete *i;
    }
  //----------------------------------------------------------------------------
  // childNodes
  //----------------------------------------------------------------------------
  public:
    XmlNode* release_child(int index){
      if(index<0||index>=childNodes.size())return nullptr; // throw

      XmlNode* const node=childNodes[index];
      node->parent=nullptr;
      node->index=-1;

      childNodes.erase(childNodes.begin()+index);
      typedef std::vector<XmlNode*>::iterator it_t;
      for(it_t i=childNodes.begin()+index,iN=childNodes.end();i<iN;i++)
        (*i)->index=index++;

      return node;
    }
    XmlNode* release_child(XmlNode* node){
      if(node->parent!=this)return nullptr; // throw
      return this->release_child(node->index);
    }
    void remove_child(int index){
      delete this->release_child(index);
    }
    void remove_child(XmlNode* node){
      if(node->parent!=this)return; // throw
      this->remove_child(node->index);
    }
    XmlNode* add_child(XmlNode* node){
      if(node->parent!=nullptr)
        node->parent->release_child(node);

      node->parent=this;
      node->index=childNodes.size();
      childNodes.push_back(node);
      return node;
    }
  public:
    XmlElement* add_element(const char* tagName){
      XmlElement* ret=new XmlElement(tagName);
      add_child(ret);
      return ret;
    }
    XmlText* add_text(const char* text){
      XmlText* ret=new XmlText(text);
      this->add_child(ret);
      return ret;
    }
  //----------------------------------------------------------------------------
  // tostr
  public:
    virtual void to_str(std::ostream& ostr) const{
      ostr<<"<"<<this->name;

      typedef std::vector<XmlAttribute>::const_iterator attr_it;
      for(attr_it i=attributes.begin(),iN=attributes.end();i!=iN;++i){
        ostr<<" ";
        i->to_str(ostr);
      }

      if(this->childNodes.size()==0){
        ostr<<" />\n";
      }else{
        ostr<<">\n";
        typedef std::vector<XmlNode*>::const_iterator node_it;
        for(node_it i=childNodes.begin(),iN=childNodes.end();i!=iN;++i){
          (*i)->to_str(ostr);
        }
        ostr<<"</"<<this->name<<">\n";
      }
    }
  //----------------------------------------------------------------------------
  // enumeration
  public:
    mwg::exp::enumerator<XmlElement*> enumElements(bool recursive=false){
      typedef std::vector<XmlNode*> list_type;
      typedef list_type::iterator it_t;

      struct impl_t:mwg::exp::ienumerator<XmlElement*,impl_t>{
        list_type& childNodes;
        bool recursive;
      public:
        impl_t(list_type& childNodes,bool recursive)
          :childNodes(childNodes),recursive(recursive)
          ,next_flag(0){this->next();}
        impl_t(mwg::exp::ienumerator_end_tag)
          :childNodes(*reinterpret_cast<list_type*>(0)),next_flag(-1){}
      public:
        virtual XmlElement* operator*() const{return next_value;}
        virtual operator bool() const{return next_flag>=0;}
      //-----------------------------------------------------------------------
      // next() continuous
      private:
        int         next_flag;
        XmlElement* next_value;
        it_t        i;
        it_t        iN;
        XmlElement* elem;
        mwg::exp::enumerator<XmlElement*> mago;
      public:
        virtual void next(){
          mwg_yield_start_with(next_flag,next_value,XmlElement*);

          for(i=childNodes.begin(),iN=childNodes.end();i<iN;i++){
            if((*i)->nodeType()!=XmlNodeType::Element)continue;
            elem=static_cast<XmlElement*>(*i);
            mwg_yield_return(elem);
            if(recursive){
              mago=elem->enumElements(true);
              mwg_yield_allval(mago);
            }
          }

          mwg_yield_end;
        }
      };

      return new impl_t(this->childNodes,recursive);
    }
    //ienumerator<stdm::shared_ptr<XmlElement> >* enumElementsByTagName(const std::string& tagName,bool recursive=false){
    //  return nullptr;
    //  // ienumerator<XmlElement^>^
    //}
  //----------------------------------------------------------------------------
  // attributes
  //----------------------------------------------------------------------------
  public:
    const std::string& get_attribute(const std::string& name) const{
      std::vector<XmlAttribute>::const_iterator i=attributes.begin(),iN=attributes.end();
      for(;i<iN;i++)
        if(i->name==name)return i->value;
      return empty_string;
    }
    bool has_attribute(const std::string& name) const{
      std::vector<XmlAttribute>::const_iterator i=attributes.begin(),iN=attributes.end();
      for(;i<iN;i++)
        if(i->name==name)return true;
      return false;
    }
    void add_attribute(const std::string& name,const std::string& value){
      std::vector<XmlAttribute>::iterator i=attributes.begin(),iN=attributes.end();
      for(;i<iN;i++){
        if(i->name==name){
          i->value=value;
          return;
        }
      }
      attributes.push_back(XmlAttribute(name,value));
    }
  };
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#endif
