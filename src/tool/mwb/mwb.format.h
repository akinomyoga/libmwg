// -*- mode:C++;coding:shift_jis -*-
#ifndef MWG_BIO_MWB_FORMAT_H
#define MWG_BIO_MWB_FORMAT_H
#include <cstdlib>
#include <string>
#include <vector>
#include <mwg/std/utility>
#include <mwg/std/memory>
#include <mwg/bio/defs.h>
#include <mwg/bio/tape.h>
#include <mwg/bio/mwb_header.h>
namespace mwg{
namespace bio{
namespace mwb1{
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
  class IFormatNode;

  class AtomTypeFormatNode;
  class ArrayTypeFormatNode;
  class TupleTypeFormatNode;

  class MarkFormatNode;
  class QuotedDataFormatNode;

//-----------------------------------------------------------------------------
  class IFormatNode{
  public:
    std::string name;
    virtual byte GetFormatNodeType() const=0;
    virtual IFormatNode* Clone() const=0;
    virtual ~IFormatNode(){}
  public:
    static bool isatom(IFormatNode* node){
      byte c=node->GetFormatNodeType();
      return 'a'<=c&&c<='z';
    }
    static bool istype(IFormatNode* node){
      byte c=node->GetFormatNodeType();
      return 'a'<=c&&c<='z'||c==FMTC::ARRAY||c==FMTC::TUPLE;
    }
    typedef struct{} *clone_tag;
  };

  class AtomTypeFormatNode:public IFormatNode{
    byte fmtc;
  public:
    AtomTypeFormatNode(byte fmtc):fmtc(fmtc){}
    virtual byte GetFormatNodeType() const{return fmtc;}
  protected:
    AtomTypeFormatNode(AtomTypeFormatNode const& node,IFormatNode::clone_tag)
      :IFormatNode(node),fmtc(node.fmtc){}
  public:
    virtual IFormatNode* Clone() const{
      return new AtomTypeFormatNode(*this,IFormatNode::clone_tag());
    }
  };

  class ArrayTypeFormatNode:public IFormatNode{
    std::size_t m_len;
    stdm::unique_ptr<IFormatNode> m_element;
  public:
    ArrayTypeFormatNode(std::size_t len,IFormatNode* elem)
      :m_len(len),m_element(stdm::move(elem)){}
    ArrayTypeFormatNode(std::size_t len,stdm::unique_ptr<IFormatNode>&& elem)
      :m_len(len),m_element(stdm::move(elem)){}
    virtual byte GetFormatNodeType() const{return FMTC::ARRAY;}
  protected:
    ArrayTypeFormatNode(ArrayTypeFormatNode const& node,IFormatNode::clone_tag)
      :IFormatNode(node),m_len(node.m_len),m_element(node.m_element->Clone()){}
  public:
    virtual IFormatNode* Clone() const{
      return new ArrayTypeFormatNode(*this,IFormatNode::clone_tag());
    }
  public:
    std::size_t GetLength() const{return this->m_len;}
    IFormatNode* GetElementType() const{return this->m_element.get();}
  };

  class MarkFormatNode:public IFormatNode{
  public:
    MarkFormatNode(){}
    virtual byte GetFormatNodeType() const{return FMTC::MARK;}
  protected:
    MarkFormatNode(MarkFormatNode const& node,IFormatNode::clone_tag)
      :IFormatNode(node){}
  public:
    virtual IFormatNode* Clone() const{
      return new MarkFormatNode;
    }
  };

  class TupleTypeFormatNode:public IFormatNode{
    std::vector<stdm::unique_ptr<IFormatNode> > m_members;
  public:
    TupleTypeFormatNode(){}
    virtual byte GetFormatNodeType() const{return FMTC::TUPLE;}
  protected:
    TupleTypeFormatNode(TupleTypeFormatNode const& node,IFormatNode::clone_tag)
      :IFormatNode(node)
    {
      typedef std::vector<stdm::unique_ptr<IFormatNode> >::const_iterator iterator;
      for(iterator i=node.m_members.begin(),iN=node.m_members.end();i!=iN;++i)
        this->AddMember((*i)->Clone());
    }
  public:
    virtual IFormatNode* Clone() const{
      return new TupleTypeFormatNode(*this,IFormatNode::clone_tag());
    }
  public:
    void AddMember(IFormatNode* node){
      m_members.push_back(stdm::unique_ptr<IFormatNode>(node));
    }
    void Reverse(){
      // for parsing
      std::reverse(m_members.begin(),m_members.end());
    }
    std::size_t GetLength() const{return this->m_members.size();}
    IFormatNode* GetElementType(int index) const{return this->m_members[index].get();}
  };

  class QuotedDataFormatNode:public IFormatNode{
    std::vector<byte> m_data;
  public:
    QuotedDataFormatNode(std::vector<byte>&& data):m_data(stdm::move(data)){}
    QuotedDataFormatNode(std::vector<byte> const& data):m_data(data){}
    virtual byte GetFormatNodeType() const{return FMTC::QUOTE;}
  protected:
    QuotedDataFormatNode(QuotedDataFormatNode const& node,IFormatNode::clone_tag)
      :IFormatNode(node),m_data(node.m_data){}
  public:
    virtual IFormatNode* Clone() const{
      return new QuotedDataFormatNode(*this,IFormatNode::clone_tag());
    }

    const byte* data() const{return &this->m_data[0];}
    std::size_t size() const{return this->m_data.size();}
  };

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
  stdm::unique_ptr<IFormatNode> read_format(mwg::bio::itape const& tape);

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif
