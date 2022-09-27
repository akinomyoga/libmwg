// -*- mode: c++; coding: utf-8 -*-
#include <stack>
#include <map>
#include <mwg/bio/tape.h>
#include <mwg/bio/mwb_header.h>
#include <mwg/bio/mwb_format.h>

namespace mwg {
namespace bio {
namespace mwb1 {
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
// フォーマット読み取り関数実装
  stdm::unique_ptr<IFormatNode> read_format(mwg::bio::itape const& tape);
//-----------------------------------------------------------------------------
  class FormatReader {
    mwg::bio::tape_head<mwg::bio::itape> h;
    std::stack<IFormatNode*> stk;
    std::map<std::string,IFormatNode*> types;
  private:
    IFormatNode* stk_pop() {
      IFormatNode* ret = stk.top();
      stk.pop();
      return ret;
    }
  public:
    FormatReader(mwg::bio::itape const& tape): h(tape) {}
    ~FormatReader() {
      while (!stk.empty())
        delete stk_pop();

      typedef std::map<std::string,IFormatNode*> map_t;
      for (map_t::iterator i = types.begin(), iN = types.end(); i != iN; ++i)
        delete i->second;
    }
  private:
    bool read_number(u8t& ret) {
      if (stk.empty() || stk.top()->GetFormatNodeType() != FMTC::QUOTE) return false;
      QuotedDataFormatNode* quoted = static_cast<QuotedDataFormatNode*>(stk_pop());
      {
        ret = 0;
        byte const* const data = quoted->data();
        std::size_t const size = quoted->size();
        for (std::size_t i = 0; i < size; i++) ret |= data[i] << i * 8;
      }
      delete quoted;
      return true;
    }
    bool read_string(std::string& ret) {
#ifdef DBG20120222
      std::fprintf(stderr, "dbg: read_string: stk.top()->GetFormatNodeType() == '%d', QUOTE == '%d'\n", (byte) stk.top()->GetFormatNodeType(), (byte) FMTC::QUOTE);
      std::fflush(stderr);
#endif
      if (stk.empty() || stk.top()->GetFormatNodeType() != FMTC::QUOTE) return false;
      QuotedDataFormatNode* quoted = static_cast<QuotedDataFormatNode*>(stk_pop());
      {
        ret.assign(reinterpret_cast<const char*>(quoted->data()), quoted->size());
      }
      delete quoted;
      return true;
    }
    bool read_quoted(int size) {
#ifdef DBG20120222
      std::fprintf(stderr, "dbg: read_quoted\n");
      std::fflush(stderr);
#endif
      std::vector<byte> data;
      for (int i = 0; i < size; i++) {
        byte b;
        if (!h.read(b)) return false;
        data.push_back(b);
      }
      stk.push(new QuotedDataFormatNode(stdm::move(data)));
      return true;
    }
  public:
    stdm::unique_ptr<IFormatNode> read_format() {
      byte fmtc;
      while (h.read(fmtc)) {
        if ('a' <= fmtc && fmtc <= 'z') {
          stk.push(new AtomTypeFormatNode(fmtc));
        } else if (FMTC::QUOTE <= fmtc && fmtc < FMTC::QUOTEL) {
          this->read_quoted(fmtc - FMTC::QUOTE);
        } else switch(fmtc) {
        case FMTC::TERM:
          if (stk.empty())
            goto invalid_format;
          else {
#ifdef DBG20120222
            std::fprintf(stderr, "dbg: TERM: node /* %p */ ->GetFormatNodeType() = '%c'\n", stk.top(), stk.top()->GetFormatNodeType());
            std::fflush(stderr);
#endif
            h.align(8);
            return stdm::unique_ptr<IFormatNode>(stk_pop());
          }
        case FMTC::ARRAY:
          {
            u8t dim; if (!this->read_number(dim)) dim = 0;
#ifdef DBG20120222
            std::fprintf(stderr, "dbg: ARRAY: dim=%d\n", dim);
            std::fflush(stderr);
#endif

            if (!stk.empty() && IFormatNode::istype(stk.top())) {
              stk.push(new ArrayTypeFormatNode(dim,stk_pop()));
#ifdef DBG20120222
              std::fprintf(stderr, "dbg: ARRAY: anode /* %p */ ->GetFormatNodeType() = '%c'\n", stk.top(), stk.top()->GetFormatNodeType());
              std::fflush(stderr);
#endif
              break;
            } else {
              goto invalid_format; // error: invalid format specs
            }
          }
        case FMTC::MARK:
          stk.push(new MarkFormatNode);
          break;
        case FMTC::TUPLE:
          {
            TupleTypeFormatNode* tnode = new TupleTypeFormatNode;
            for (;;) {
              if (stk.empty()) {
                goto invalid;
              } else if (IFormatNode::istype(stk.top())) {
                tnode->AddMember(stk_pop());
                continue;
              } else if (stk.top()->GetFormatNodeType() == FMTC::MARK) {
                tnode->Reverse();
#ifdef DBG20120222
                std::fprintf(stderr,"dbg: make_tuple (tnode /* %p */ ->GetFormatNodeType() = '%c')\n",tnode,tnode->GetFormatNodeType());
                std::fflush(stderr);
#endif
                stk.push(tnode);
                break;
              }
            invalid:
              delete tnode;
              goto invalid_format; // error: invalid format specs
            }
            break;
          }
        case FMTC::QUOTEL:
          {
            u8t len; if (!this->read_number(len)) len = 0;
            this->read_quoted(len);
            break;
          }
        case FMTC::NAME:
          {
#ifdef DBG20120222
            std::fprintf(stderr, "dbg: NAME\n");
            std::fflush(stderr);
#endif
            std::string name; if (!this->read_string(name)) goto invalid_format;
#ifdef DBG20120222
            std::fprintf(stderr, "dbg: NAME: name = '%.*s'\n", name.size(), name.c_str());
            std::fflush(stderr);
#endif
            if (stk.empty() || stk.top()->name.size() != 0) goto invalid_format; // double naming
            stk.top()->name=name;
            break;
          }
        case FMTC::TYPEDEF:
          {
            std::string name; if (!this->read_string(name)) goto invalid_format;
            if (stk.empty() || !IFormatNode::istype(stk.top())) goto invalid_format;
            types.insert(std::pair<std::string,IFormatNode*>(name,this->stk_pop()));
            break;
          }
        case FMTC::NTYPE:
          {
            std::string name; if (!this->read_string(name)) goto invalid_format;

            typedef std::map<std::string, IFormatNode*> map_t;
            map_t::iterator i = types.find(name);
            stk.push(i->second->Clone());
            break;
          }
        }
      }
    invalid_format:
      return stdm::unique_ptr<IFormatNode>(nullptr);
    }
  };

  stdm::unique_ptr<IFormatNode> read_format(mwg::bio::itape const& tape) {
    return FormatReader(tape).read_format();
  }
//FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
}
}
}
