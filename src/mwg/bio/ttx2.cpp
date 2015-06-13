// -*- mode:C++;coding:utf-8 -*-
#define TTX_CPP__EnableComment

#include <mwg/std/utility>
#include <mwg/bio/ttx2.h>
#include <mwg/bio/tape.h>

#include <mwg/impl/warning_push.inl>
namespace mwg{
namespace bio{
namespace ttx2_detail{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
//  TTX Parser
//-----------------------------------------------------------------------------
struct ttx_charcat_t{
  static bool isnamef(char c){
    return 'a'<=c&&c<='z'||'A'<=c&&c<='Z'
      ||c=='-'||c=='_'||c==':'||c=='.'||c=='+';
    // dame: </>&="'`\ ()[]{} *?!#$%@;,|
  }
  static bool isname(char c){
    return isnamef(c)||'0'<=c&&c<='9';
  }
};

//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
template<typename S>
struct ttx_source{};

template<>
struct ttx_source<const char*>{
  const char* ptr;
public:
  typedef const char* source_init_type;
  ttx_source(const char* ptr):ptr(ptr){}
  ttx_source():ptr(nullptr){}
public:
  operator bool() const{return ptr!=nullptr&&*ptr!='\0';}
  char operator*() const{return *ptr;}
  ttx_source operator++(int){return ttx_source(this->ptr++);}
  ttx_source& operator++(){this->ptr++;return *this;}
};

template<>
struct ttx_source<std::istream>{
  std::istream* istr;
  int c;
public:
  typedef std::istream& source_init_type;
  ttx_source(std::istream& istr):istr(&istr){c=this->istr->get();}
  ttx_source():istr(nullptr),c(EOF){}
  operator bool() const{return c!=EOF;}
  char operator*() const{return c;}
  ttx_source& operator++(){
    this->c=istr->get();
    return *this;
  }
public:
  struct charptr_t{
    int c;
  public:
    charptr_t(char c):c(c){}
    char operator*() const{return c;}
  };
  charptr_t operator++(int){
    char ret=this->c;
    this->operator++();
    return ret;
  }
};

template<>
struct ttx_source<FILE*>{
  FILE*const f;
  int c;
public:
  typedef FILE* source_init_type;
  ttx_source(FILE* f):f(f){
    this->operator++();
  }
  ttx_source():f(nullptr),c(EOF){}
  operator bool() const{return c!=EOF;}
  char operator*() const{return c;}
  ttx_source& operator++(){
    if(!std::fread(&c,1,1,f))c=EOF;
    return *this;
  }
public:
  struct charptr_t{
    int c;
  public:
    charptr_t(char c):c(c){}
    char operator*() const{return c;}
  };
  charptr_t operator++(int){
    char ret=this->c;
    this->operator++();
    return ret;
  }
};

template<>
struct ttx_source<mwg::bio::itape>{
  const mwg::bio::itape* tape;
  int c;
public:
  typedef const mwg::bio::itape& source_init_type;
  ttx_source(const mwg::bio::itape& tape):tape(&tape){
    this->operator++();
  }
  ttx_source():tape(nullptr),c(EOF){}
  operator bool() const{return c!=EOF;}
  char operator*() const{return c;}
  ttx_source& operator++(){
    if(!tape->read(&c,1))c=EOF;
    return *this;
  }
public:
  struct charptr_t{
    int c;
  public:
    charptr_t(char c):c(c){}
    char operator*() const{return c;}
  };
  charptr_t operator++(int){
    char ret=this->c;
    this->operator++();
    return ret;
  }
};


//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  implementation of ttx_scan
//-----------------------------------------------------------------------------
/**\struct ttx_scanner
 * \tparam[a] S 文字源。const char*, std::FILE*, std::istream, mwg::bio::itape など。
 */
template<typename S>
struct ttx_scanner{
  enum WT{
    WT_NUL=0,
    WT_KEY, // elem
    WT_VAL, // "value"
    WT_OP,  // {
    WT_CL,  // }
    WT_EQ,  // = NOT_USED
    WT_SEP, // , NOT_USED
  };

  ttx_source<S> p;
  int wt;
  std::string w;
public:
  ttx_scanner(typename ttx_source<S>::source_init_type p):p(p){}
  ttx_scanner():p(){}
  const std::string& getw() const{
    return w;
  }
  int gett() const{
    return wt;
  }
//-----------------------------------------------------------------------------
// scanning
private:
  static int xdigit_to_number(char c){
    if(isdigit(c))
      return c-'0';
    else if(islower(c))
      return c-('a'-10);
    else if(isupper(c))
      return c-('A'-10);
    else
      return 0;
  }
#ifdef TTX_CPP__EnableComment
  void skip_comment(){
    //mwg_assert(*p=='#');
    while(p&&*p!='\n'&&*p!='\r')++p;

    char c=*p++;
    if(c=='\r'&&*p=='\n')p++;
  }
#endif
  void skip_space(){
#ifdef TTX_CPP__EnableComment
    while(p)
      if(isspace(*p)){
        p++;
      }else if(*p=='#'){
        this->skip_comment();
      }else break;
#else
    while(p&&isspace(*p))++p;
#endif
  }
  void skip_space_or_sep(){
#ifdef TTX_CPP__EnableComment
    while(p)
      if(isspace(*p)||*p==','){
        p++;
      }else if(*p=='#'){
        this->skip_comment();
      }else break;
#else
    while(p&&(isspace(*p)||*p==','))++p;
#endif
  }
  void read_quoted_escaped_char(){
    //[:p:]
    byte c=*p++;
    if('0'<=c&&c<='7'){
      // \ooo
      int v=c-'0';
      if(p&&'0'<=*p&&*p<='7'){
        c=c*8+*p++-'0';
        if(p&&'0'<=*p&&*p<='7')
          c=c*8+*p++-'0';
      }
      w+=byte(v);
    }else{
      switch(c){
      case 'a':w+='\a';break;
      case 'b':w+='\b';break;
      case 't':w+='\t';break;
      case 'n':w+='\n';break;
      case 'v':w+='\v';break;
      case 'f':w+='\f';break;
      case 'r':w+='\r';break;
      case 'e':w+='\x1b';break;
      case 'x': // \xhh
        {
          int v=0;
          if(p&&isxdigit(*p)){
            v=v*16+xdigit_to_number(*p++);
            if(p&&isxdigit(*p))
              v=v*16+xdigit_to_number(*p++);
          }
          w+=byte(v);
        }
        //TODO \uhhhh, encoding
        //TODO \Uhhhhhhhh, encoding
        break;
      default:
        w+=c;
      }
    }
  }
  void read_quoted(){
    char c=*p++;
    while(p){
      if(*p==c){
        ++p;
        return;
      }else if(*p=='\\'){
        if(!++p)return;
        read_quoted_escaped_char();
      }else{
        w+=*p++;
      }
    }
  }
public:
  bool next_val(){
    wt=WT_VAL;
    w.erase();
    int lenM=0;

    // head space
    this->skip_space();
    if(*p=='='){
      ++p;
      this->skip_space();
    }

    // body
    while(p){
      if(*p=='{'){
        if(w.size()==0){
          wt=WT_OP;
          ++p;
          return true;
        }
        break;
      }else if(*p==','||*p=='}'){
        break;
      }else if(*p=='"'||*p=='\''){
        this->read_quoted();
        lenM=w.size();
#ifdef TTX_CPP__EnableComment
      }else if(*p=='#'){
        this->skip_comment();
#endif
      }else{
        w+=*p++;
      }
    }

    // tail space
    int len=w.size();
    while(len>lenM&&isspace(w[len-1]))len--;
    if(len!=w.size())
      w.erase(len);
    return true;
  }
private:
  void read_key(){
    if(ttx_charcat_t::isname(*p)){
      w+=*p++;
      while(p&&ttx_charcat_t::isname(*p))
        w+=*p++;
    }else if(*p=='"'||*p=='\''){
      this->read_quoted();
    }else{
      w+=*p++;
    }
  }
public:
  bool next(){
    wt=WT_NUL;
    w.erase();
    this->skip_space_or_sep();
    if(!p)return false;

    if(*p=='{'){
      wt=WT_OP;
      ++p;
      return true;
    }else if(*p=='}'){
      wt=WT_CL;
      ++p;
      return true;
    }else if(*p=='='){
      wt=WT_KEY;
      w="data";
      return true;
    }else{
      wt=WT_KEY;
      this->read_key();
      return true;
    }
  }
public:
  bool peek_isCL(){
    this->skip_space_or_sep();
    return p&&*p=='}';
  }
};
//*****************************************************************************
//  ttx_scan: scanner 末端まで全ての要素を解析
//-----------------------------------------------------------------------------
template<typename S>
void ttx_scan(ttx_node& root,ttx_scanner<S>& s){
  typedef ttx_scanner<S> scanner_t;
  ttx_node* elem=&root;
  while(s.next()){
    if(s.gett()==scanner_t::WT_OP){
      elem=elem->create_node("xml");
    }else if(s.gett()==scanner_t::WT_CL){
      ttx_node* parent=elem->parent();
      if(parent!=nullptr)elem=parent;
    }else{
      std::string key=s.getw();
      if(!s.next_val()){
        elem->attr(key)="";
      }else if(s.gett()==scanner_t::WT_OP){
        elem=elem->create_node(key.c_str());
      }else{
        // TODO: key=="#" -> text element
        elem->attr(key)=s.getw();
      }
    }
    //std::printf("dbg: word: wt='%d' w='%s'\n",s.gett(),s.getw().c_str());
  }
}

void ttx_scan(ttx_node& root,const char* ttx){
  ttx_scanner<const char*> s(ttx);
  ttx_scan(root,s);
}
void ttx_scan(ttx_node& root,std::istream& ttx){
  ttx_scanner<std::istream> s(ttx);
  ttx_scan(root,s);
}
void ttx_scan(ttx_node& root,FILE* ttx){
  ttx_scanner<FILE*> s(ttx);
  ttx_scan(root,s);
}
void ttx_scan(ttx_node& root,const mwg::bio::itape& ttx){
  ttx_scanner<mwg::bio::itape> s(ttx);
  ttx_scan(root,s);
}
//*****************************************************************************
//  ttx_scan_enumerate_node: 一つ要素を解析する毎に列挙
//-----------------------------------------------------------------------------
template<typename S>
struct ttx_scan_enumerate_node_impl_t
  :public mwg::exp::ienumerator<ttx_node*,ttx_scan_enumerate_node_impl_t<S> >
{
  typedef ttx_scanner<S> scanner_t;
  ttx_scanner<S> s;
public:
  ttx_scan_enumerate_node_impl_t(S& s)
    :s(s),next_flag(0)
    ,elem(nullptr)
  {this->next();}
  ttx_scan_enumerate_node_impl_t(mwg::exp::ienumerator_end_tag)
    :s(),next_flag(-1)
    ,elem(nullptr)
  {}
  ~ttx_scan_enumerate_node_impl_t(){
    if(elem!=nullptr){
      delete elem;
      elem=nullptr;
    }
  }
public:
  virtual ttx_node* operator*() const{return next_value;}
  virtual operator bool() const{return next_flag>=0;}
//-------------------------------------------------------------------
// next() continuous
private:
  int         next_flag;
  ttx_node*   next_value;
  ttx_node*   elem;
public:
  virtual void next(){
    mwg_yield_start_with(next_flag,next_value,ttx_node*);

    elem=nullptr;
    while(s.next()){
      if(s.gett()==scanner_t::WT_OP){
        if(elem!=nullptr)
          elem=elem->create_node("xml");
        else
          elem=new ttx_node("xml");
      }else if(s.gett()==scanner_t::WT_CL){
        if(elem!=nullptr){
          if(elem->parent()!=nullptr){
            elem=elem->parent();
          }else{
            //std::cout<<"hello!"<<std::endl;
            mwg_yield_return(elem);
            //std::cout<<"world!"<<std::endl;
            delete elem;
            elem=nullptr;
          }
        }
      }else{
        std::string key=s.getw();
        if(!s.next_val()){
          if(elem!=nullptr)
            elem->attr(key)="";
        }else if(s.gett()==scanner_t::WT_OP){
          if(elem!=nullptr)
            elem=elem->create_node(key.c_str());
          else
            elem=new ttx_node(key.c_str());
        }else{
          if(elem!=nullptr)
            elem->attr(key)=s.getw();
        }
      }
      //std::printf("dbg: word: wt='%d' w='%s'\n",s.gett(),s.getw().c_str());
    }
    if(elem!=nullptr){
      delete elem;
      elem=nullptr;
    }

    mwg_yield_end;
  }
};
template<typename S>
mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(S& s){
  return new ttx_scan_enumerate_node_impl_t<S>(s);
}
//-----------------------------------------------------------------------------

mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(const char* ttx){
  return ttx_scan_enumerate_node<const char*>(ttx);
}
mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(std::istream& ttx){
  return ttx_scan_enumerate_node<std::istream>(ttx);
}
mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(FILE* ttx){
  return ttx_scan_enumerate_node<FILE*>(ttx);
}
mwg::exp::enumerator<ttx_node*> ttx_scan_enumerate_node(mwg::bio::itape& ttx){
  return ttx_scan_enumerate_node<mwg::bio::itape>(ttx);
}

//*****************************************************************************
//  ttx_scan__getnode: 要素を一つだけ読み取り
//-----------------------------------------------------------------------------

template<typename S>
bool ttx_scan__getnode(S& source,ttx_node& node,bool untilIsolatedClosingBrace=false){
  ttx_scanner<S> s(source);

  ttx_node* elem=nullptr;
  for(;;){
    if(s.peek_isCL()){
      if(elem==nullptr){
        if(untilIsolatedClosingBrace)
          return false;
        else{
          s.next(); // 孤立 '}' を無視
          continue;
        }
      }else if(elem==&node)
        return true;

      elem=elem->parent();
      s.next();
      continue;
    }

    if(!s.next())
      return elem!=nullptr;

    mwg_assert(s.gett()!=ttx_scanner<S>::WT_CL); // if(s.peek_isCL()) で処理されている筈

    if(s.gett()==ttx_scanner<S>::WT_OP){
      if(elem!=nullptr)
        elem=elem->create_node("xml");
      else{
        node=ttx_node("xml");
        elem=&node;
      }
    }else{
      std::string key=s.getw();
      if(!s.next_val()){
        // ttx: key=,

        if(elem!=nullptr)
          elem->attr(key)="";
        else{
          // 孤立属性は無視
        }
      }else if(s.gett()==ttx_scanner<S>::WT_OP){
        // ttx: key={

        if(elem!=nullptr)
          elem=elem->create_node(stdm::move(key));
        else{
          node=ttx_node(stdm::move(key));
          elem=&node;
        }
      }else{
        // ttx: key=value,

        if(elem!=nullptr)
          elem->attr(key)=s.getw();
        else{
          // 孤立属性は無視
        }
      }
    }
  }
}

bool getnode(const char* ttx,ttx_node& node,bool untilIsolatedClosingBrace){
  return ttx_scan__getnode<const char*>(ttx,node,untilIsolatedClosingBrace);
}
bool getnode(std::istream& ttx,ttx_node& node,bool untilIsolatedClosingBrace){
  return ttx_scan__getnode<std::istream>(ttx,node,untilIsolatedClosingBrace);
}
bool getnode(FILE* ttx,ttx_node& node,bool untilIsolatedClosingBrace){
  return ttx_scan__getnode<FILE*>(ttx,node,untilIsolatedClosingBrace);
}
bool getnode(mwg::bio::itape& ttx,ttx_node& node,bool untilIsolatedClosingBrace){
  return ttx_scan__getnode<mwg::bio::itape>(ttx,node,untilIsolatedClosingBrace);
}

//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//  implementation of xml_scan
//-----------------------------------------------------------------------------

// TODO (計画)
//  <?xml ?>
//  <!DOCTYPE root SYSTEM "URI">
//  <!DOCTYPE root PUBLIC "DTD" "URI">
//  <!DOCTYPE root [ ..., <!HOGE >, %param; ]>
//  <![CDATA[ data ]]> # ref <a href="http://bakera.jp/yomoyama/markedsection">マーク区間とは | 鳩丸よもやま話</a>
//  <element attr="value"></element>
//  &entity; 
//  <!-- comment -->
//
// xml_scan    : entity, CDATA など全て解決する。DTD など省略。
// xmldom_scan : entity なども全て構造として保ちつつ読み取る。
// html_scan   : xml_scan + 終了タグの省略などに対応
// htmldom_scan: xmldom_scan + 終了タグの省略などに対応
//

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
} /* end of ttx2_detail */
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

ttx_node* ttx_node::merge(const ttx_node& n1,const ttx_node& n2){
  // check
  {
    if(n1.name()!=n2.name())return nullptr;
    ttx_node::attr_iterator p=n2.attrs().begin();
    ttx_node::attr_iterator pN=n2.attrs().end();
    for(;p<pN;++p){
      if(n1.attr(p->name))
        if(p->value!=n1.attr(p->name).get())return nullptr;
    }
  }

  ttx_node* node=new ttx_node(n1.name());

  ttx_node::attr_iterator a1=n1.attrs().begin();
  ttx_node::attr_iterator a1N=n1.attrs().end();
  for(;a1<a1N;++a1)
    node->attr(a1->name)=a1->value;

  ttx_node::attr_iterator a2=n2.attrs().begin();
  ttx_node::attr_iterator a2N=n2.attrs().end();
  for(;a2<a2N;++a2){
    if(!node->attr(a2->name))
      node->attr(a2->name)=a2->value;
  }

  ttx_node::node_iterator pd=n1.nodes().begin();
  ttx_node::node_iterator pdN=n1.nodes().end();
  ttx_node::node_iterator ps=n2.nodes().begin();
  ttx_node::node_iterator psN=n2.nodes().end();
  for(;pd<pdN&&ps<psN;++ps,++pd)
    node->add_merge(**pd,**ps);

  if(pd<pdN){
    for(;pd<pdN;++pd)
      node->add_node(new ttx_node(**pd));
  }else{
    for(;ps<psN;++ps)
      node->add_node(new ttx_node(**ps));
  }
  return node;
}

//-----------------------------------------------------------------------------
class ttx_node_enumNodes_impl_t
  :public mwg::exp::ienumerator<ttx_node*,ttx_node_enumNodes_impl_t>
{
  typedef std::vector<ttx_node*> list_type;
  typedef list_type::iterator it_t;
  list_type* children;
  bool recursive;
public:
  ttx_node_enumNodes_impl_t(list_type& children,bool recursive)
    :children(&children),recursive(recursive)
    ,next_flag(0){this->next();}
  ttx_node_enumNodes_impl_t(mwg::exp::ienumerator_end_tag)
    :children(nullptr),next_flag(-1){}
public:
  virtual ttx_node* operator*() const{return next_value;}
  virtual operator bool() const{return next_flag>=0;}
//-------------------------------------------------------------------
// next() continuous
private:
  int         next_flag;
  ttx_node*   next_value;
  it_t        i;
  it_t        iN;
  ttx_node*   elem;
  mwg::exp::enumerator<ttx_node*> mago;
public:
  virtual void next(){
    mwg_yield_start_with(next_flag,next_value,ttx_node*);

    for(i=children->begin(),iN=children->end();i<iN;i++){
      mwg_yield_return(*i);
      if(recursive){
        mago=(*i)->enumerate_nodes(true);
        mwg_yield_allval(mago);
      }
    }

    mwg_yield_end;
  }
};
mwg::exp::enumerator<ttx_node*> ttx_node::enumerate_nodes(bool recursive){
  return new ttx_node_enumNodes_impl_t(this->m_nodes,recursive);
}
//-----------------------------------------------------------------------------
class ttx_node_enumNodesByName_impl_t
  :public mwg::exp::ienumerator<ttx_node*,ttx_node_enumNodesByName_impl_t>
{
  typedef std::vector<ttx_node*> list_type;
  typedef list_type::iterator it_t;
  list_type* children;
  const char*const name;
  bool const recursive;
public:
  ttx_node_enumNodesByName_impl_t(list_type& children,const char* _name,bool recursive)
    :children(&children),name(_name),recursive(recursive)
    ,next_flag(0){this->next();}
  ttx_node_enumNodesByName_impl_t(mwg::exp::ienumerator_end_tag)
    :children(nullptr)
    ,name(nullptr),recursive(false)
    ,next_flag(-1){}
public:
  virtual ttx_node* operator*() const{return next_value;}
  virtual operator bool() const{return next_flag>=0;}
//-------------------------------------------------------------------
// next() continuous
private:
  int         next_flag;
  ttx_node*   next_value;
  it_t        i;
  it_t        iN;
  ttx_node*   elem;
  mwg::exp::enumerator<ttx_node*> mago;
public:
  virtual void next(){
    mwg_yield_start_with(next_flag,next_value,ttx_node*);

    for(i=children->begin(),iN=children->end();i<iN;i++){
      if((*i)->name()==this->name)
        mwg_yield_return(*i);
      if(recursive){
        mago=(*i)->enumerate_nodes(name,true);
        mwg_yield_allval(mago);
      }
    }

    mwg_yield_end;
  }
};
mwg::exp::enumerator<ttx_node*> ttx_node::enumerate_nodes(const char* _name,bool recursive){
  return new ttx_node_enumNodesByName_impl_t(this->m_nodes,_name,recursive);
}
//-----------------------------------------------------------------------------

void ttx_node::print(std::ostream& ostr,mwg::exp::sfmt::xml_t* tag) const{
  ostr<<"<"<<this->m_name;

  typedef std::vector<ttx_attr>::const_iterator attr_it;
  for(attr_it i=m_attrs.begin(),iN=m_attrs.end();i!=iN;++i){
    ostr<<" "<<i->name<<"=\"";
    const std::string& v(i->value);
    const char* p =v.c_str();
    const char* pN=p+v.size();
    for(;p<pN;p++)switch(*p){
      case '"':ostr<<"&quot;";break;
      case '&':ostr<<"&amp;";break;
      default:ostr.put(*p);
    }
    ostr<<"\"";
  }

  if(this->m_nodes.size()==0){
    ostr<<" />\n";
  }else{
    ostr<<">\n";
    typedef std::vector<ttx_node*>::const_iterator node_it;
    for(node_it i=m_nodes.begin(),iN=m_nodes.end();i!=iN;++i){
      (*i)->print(ostr,tag);
    }
    ostr<<"</"<<this->m_name<<">\n";
  }
}

void ttx_node::print(std::ostream& ostr,mwg::exp::sfmt::ttx_t* tag) const{
  bool hasChild=this->m_nodes.size()>0;
  ostr<<this->m_name<<(hasChild?"{\n":"{");

  // m_attrs
  if(m_attrs.size()>0){
    typedef std::vector<ttx_attr>::const_iterator attr_it;
    for(attr_it i=m_attrs.begin(),iN=m_attrs.end();i!=iN;++i){
      ostr<<i->name<<"=\"";
      const std::string& v(i->value);
      const char* p =v.c_str();
      const char* pN=p+v.size();
      for(;p<pN;p++)switch(*p){
        case '"': ostr<<"\\\"";break;
        case '\\':ostr<<"\\\\";break;
        default:  ostr.put(*p);
      }
      ostr<<"\",";
    }
    if(hasChild)ostr<<"\n";
  }

  // m_nodes
  if(hasChild){
    typedef std::vector<ttx_node*>::const_iterator node_it;
    for(node_it i=m_nodes.begin(),iN=m_nodes.end();i!=iN;++i)
      (*i)->print(ostr,tag);
  }

  ostr<<"},\n";
}

static void ttx_node__print__quoted(std::ostream& ostr,std::string const& value){
  ostr<<"\"";
  const char* p =value.c_str();
  const char* pN=p+value.size();
  for(;p<pN;p++)
    switch(*p){
    case '"': ostr<<"\\\"";break;
    case '\\':ostr<<"\\\\";break;
    case '\r':ostr<<"\\r";break;
    case '\n':ostr<<"\\n";break;
    case '\f':ostr<<"\\f";break;
    case '\t':ostr<<"\\t";break;
    case '\v':ostr<<"\\v";break;
    case '\0':ostr<<"\\0";break;
    default:  ostr.put(*p);
    }
  ostr<<"\"";
}

void ttx_node::print(std::ostream& ostr,mwg::exp::sfmt::json_t* tag) const{
  bool hasChild=this->m_nodes.size()>0;
  ttx_node__print__quoted(ostr,this->m_name);
  ostr<<(hasChild?":{\n":":{");

  // m_attrs
  if(m_attrs.size()>0){
    typedef std::vector<ttx_attr>::const_iterator attr_it;
    for(attr_it i=m_attrs.begin(),iN=m_attrs.end();i!=iN;++i){
      if(i!=m_attrs.begin())ostr<<",";

      ttx_node__print__quoted(ostr,i->name);
      ostr<<":";
      ttx_node__print__quoted(ostr,i->value);
    }
    if(hasChild)ostr<<",\n";
  }

  // m_nodes
  if(hasChild){
    typedef std::vector<ttx_node*>::const_iterator node_it;
    for(node_it i=m_nodes.begin(),iN=m_nodes.end();i!=iN;++i){
      if(i!=m_nodes.begin())ostr<<",\n";
      (*i)->print(ostr,tag);
    }
    ostr<<"\n";
  }

  ostr<<"}";
}

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
#include <mwg/impl/warning_pop.inl>
