// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_BIO_FILE_MWT_IMPL2
#define MWG_BIO_FILE_MWT_IMPL2
# include <mwg/std/memory>
# include "defs.h"
# include "tape.h"
# ifdef _MSC_VER
#   pragma warning(push)
#   pragma warning(disable:4355) /* `this' in member initializers */
// GCC4.2.1 での抑制は: http://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
# endif

namespace mwg{
namespace bio{
namespace mwt_detail{


  template<typename T>
  class iprop{
  public:
    virtual void get_value(T& value) const=0;
    virtual void set_value(const T& value) const=0;
    virtual ~iprop(){}

    T get_value() const{
      T ret;
      get_value(ret);
      return ret;
    }
  };

  //---------------------------------------------------------------------------
  struct BID{
  public:
    static bool isSpecial(i4t bid){
      return (bid&0xFF)==0;
    }
  };

#define MWT_FOURCC(a,b,c,d) (a|b<<8|c<<16|d<<24)
  //const i4t MAGIC        =MWT_FOURCC('M','W','T','1');
  const i4t MAGICR       =MWT_FOURCC('M','W','T','R');

  const i4t BID_NOTUSED  =0x0000; // 特別 (必ず 0)
  const i4t BID_LAST     =MWT_FOURCC(0,'-','$','-');
  const i4t BID_ROOT     =MWT_FOURCC(0,'M','W','T');
#undef MWT_FOURCC

  struct DataPtr{
    i4t offset; // ブロック番号
    i4t length;
  };
  template<typename ITape>
  class PtrBPropertyDescriptor:public iprop<DataPtr>{
    tape_head<ITape>& head;
    i8t               offset;
  public:
    PtrBPropertyDescriptor(tape_head<ITape>& head,i8t offset)
      :head(head),offset(offset){}
    virtual void get_value(DataPtr& value) const{
      head.seek(offset);
      head.read(value);
    }
    virtual void set_value(const DataPtr& value) const{
      head.seek(offset);
      head.write(value);
    }
  };

  //---------------------------------------------------------------------------
  const int BlockSize   =0x0400; // 1kB
  struct RootBlock{
    i4t magic;   // mwt 形式の保証
    i4t padding;

    DataPtr Fat256;
    DataPtr Fat64;
    DataPtr Fat16;

    DataPtr listNodeIndex;
    DataPtr listTagIndex;

    i4t HeapFreeNodeCount;
    i4t HeapFreeNodeLength;
    DataPtr HeapList; // "heap 表"
    DataPtr HeapData; // "heap ブロック番号リスト"

    DataPtr RootDir; // "ルートディレクトリの中身"

    // 作成日時
    // 更新日時
    // 読取日時
    // 属性 (rwx 読取日時記録 プロテクト quota etc.)
    // 著作権 etc.
    int reserved[238];
  };
  static const int OffsetOfFat256=BlockSize+((int)(iPt)(byte*)&reinterpret_cast<RootBlock*>(0)->Fat256);
  static const int OffsetOfFat64 =BlockSize+((int)(iPt)(byte*)&reinterpret_cast<RootBlock*>(0)->Fat64);

  //---------------------------------------------------------------------------
  //  Nodes
  //---------------------------------------------------------------------------
  static const byte NPT_DATA='D'; // DATA: ファイル内容
  static const byte NPT_FILT='F'; // FILT: ファイル内容に対するフィルタ
  static const byte NPT_MIME='M'; // MIME: ファイル内容の形式を指定する MIME type
  static const byte NPT_HASH='H'; // HASH: ファイル内容の checksum, md5hash, etc.

  static const byte NPT_LIST='>'; // LIST: 子ノード配列
  static const byte NPT_TIME='T'; // TIME: タイムスタンプ
  static const byte NPT_FLAG='A'; // FLAG: フラグ rwx ash
  static const byte NPT_PROP='@'; // PROP: 一般属性・ストリーム
  static const byte NPT_TAGS='K'; // TAGS: タグリスト

  typedef i4t NID;
  struct NodeData{
    NID parent;
    char name[1]; // null terminated
    //NodeProp properties; // 複数続く
  };

//*****************************************************************************
//
//    実装
//
//*****************************************************************************
  template<typename ITape=itape,int CellSize=0x400>
  struct mwtfile_cellbath;
  template<typename ITape=itape,int CellSize=0x400>
  class mwtfile_celltape;
  template<typename ITape=itape>
  class mwtfile_autotape;
  template<typename ITape=itape>
  struct mwtfile_manager;
  //iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
  //  interfaces
  //---------------------------------------------------------------------------
  struct iptrtape:itape{
    virtual void get_dataptr(DataPtr& value) const=0;
  };
  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  //  ブロックの確保・解放
  //---------------------------------------------------------------------------
  template<typename ITape,int CellSize>
  struct mwtfile_cellbath{
  //---------------------------------------------------------------------------
  //  Block Metric Definitions
  //---------------------------------------------------------------------------
    static const int NCellInPlane=CellSize/4;
    static const int PlaneSize   =CellSize*NCellInPlane;
    static u4t GetRequiredCellCount(u4t dataSize){
      return (dataSize+u4t(CellSize-1))/u4t(CellSize);
    }
    static u4t GetRequiredPlaneCount(u4t dataSize){
      return (dataSize+u4t(PlaneSize-1))/u4t(PlaneSize);
    }
  //---------------------------------------------------------------------------
    tape_head<ITape>& head;
  public:
    mwtfile_cellbath(tape_head<ITape>& head):head(head){}
    ~mwtfile_cellbath(){this->full_truncate();}
  public:
    /// <summary>新しいブロックを割り当てます</summary>
    /// <param name="bidStart">空きブロックの検索開始位置を指定します。</param>
    i4t cell_alloc(i4t bidStart){
      int iplane=bidStart/NCellInPlane;
      int iblock=bidStart%NCellInPlane;
      if(iblock<1)iblock=1;
      //std::printf("bath%d:new cell\n",CellSize);
      for(;;iplane++,iblock=1){
        // 新しい plane を作成
        if(head.size()<=iplane*(i8t)PlaneSize){
          //mwg_assert(BID_NOTUSED==0);
          head.seek(iplane*(i8t)PlaneSize);
          head.template write<byte>(1);
          head.seek(iplane*(i8t)PlaneSize+sizeof(i4t));
          head.fill_n<byte>(0,sizeof(i4t)*(NCellInPlane-1));
          return iplane*NCellInPlane+1;
        }

        // plane 内の使用ブロック数を確認
        byte c;
        head.seek(iplane*(i8t)PlaneSize);
        head.read(c);
        if(c==NCellInPlane-1)continue;

        head.seek(iplane*(i8t)PlaneSize+sizeof(i4t)*iblock);
        for(;iblock<NCellInPlane;iblock++){
          i4t bid;
          head.read(bid,FMT_LE);
          if(bid!=BID_NOTUSED)continue;

          // plane 空きブロック数を inc
          c++;
          head.seek(iplane*(i8t)PlaneSize);
          head.write(c);
          return iplane*NCellInPlane+iblock;
        }
      }
    }
    i4t cell_getNext(i4t bid){
      int iplane=bid/NCellInPlane;
      int iblock=bid%NCellInPlane;
      i4t ret;
      head.seek(iplane*(i8t)PlaneSize+sizeof(i4t)*iblock);
      head.read(ret,FMT_LE);
      return ret;
    }
    void cell_setNext(i4t bid,i4t next){
      int iplane=bid/NCellInPlane;
      int iblock=bid%NCellInPlane;
      head.seek(iplane*(i8t)PlaneSize+sizeof(i4t)*iblock);
      head.write(next,FMT_LE);
    }
    void cell_free(i4t bid){
      int iplane=bid/NCellInPlane;
      int iblock=bid%NCellInPlane;
      head.seek(iplane*(i8t)PlaneSize+sizeof(i4t)*iblock);
      head.write(BID_NOTUSED,FMT_LE);

      // plane 空きブロック数を dec
      byte c;
      head.seek(iplane*(i8t)PlaneSize);
      head.read(c);
      c--;
      head.seek(-(i4t)sizeof c,SEEK_CUR);
      head.write(c);

      //this->full_truncate();
      if(head.size()>(bid+1)*(i8t)CellSize)return;
      this->full_truncate();

      // ■■ 空きブロック・プレーンの truncate
      // どんどん遡っていって消せる所まで消す
    }
  private:
    /// <summary>
    /// ストリームの長さを縮められる所まで縮めます。
    /// </summary>
    void full_truncate(){
      // 削除できる Plane を全て削除
      u8t const length=head.size();
      int nplane=GetRequiredPlaneCount(length);
      for(;nplane>0;){
        byte c=0;
        head.seek((nplane-1)*(i8t)PlaneSize);
        head.read(c);
        if(c!=0)break;
        nplane--;
      }

      if(nplane==0){
        head.trunc(0);
        return;
      }

      // 削除できる Block を全て削除
      int ncell=NCellInPlane;
      for(;ncell>1;){
        i4t bid=0;
        head.seek((nplane-1)*(i8t)PlaneSize+(ncell-1)*sizeof(i4t));
        head.read(bid,FMT_LE);
        if(bid!=0)break;
        ncell--;
      }

      u8t const minimum=(nplane-1)*PlaneSize+ncell*CellSize;
      if(length!=minimum){
        //std::printf("dbg:full_truncate: cellsize=%d planesize=%d\n",CellSize,PlaneSize);
        //std::printf("dbg:full_truncate: nplane=%d ncell=%d\n",nplane,ncell);
        //std::printf("dbg:full_truncate: %lld -> %lld\n",length,minimum);
        head.trunc(minimum);
      }
    }
  };
  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  //  ブロックベーステープ
  //---------------------------------------------------------------------------
  template<typename ITape,int CellSize>
  class mwtfile_celltape:public iptrtape{
    typedef mwtfile_cellbath<ITape,CellSize> Bath;
    tape_head<ITape>&   head;
    Bath&               bath;
    //i8t const           offsetPtr;
    iprop<DataPtr>*     ptrDesc;
    bool const          ptrDescToDelete;

    mutable DataPtr          ptr;
    mutable i4t              position;
    mutable std::vector<i4t> cells;
    mutable bool             fZERO;
  private:
    mwtfile_celltape(const mwtfile_celltape&) mwg_std_deleted;
    mwtfile_celltape& operator=(const mwtfile_celltape&) mwg_std_deleted;
  public:
    void get_dataptr(DataPtr& ptr) const{ptr=this->ptr;}
  public:
    mwtfile_celltape(tape_head<ITape>& head,Bath& bath,i8t offsetPtr)
      :bath(bath),head(head)
      ,ptrDesc(new PtrBPropertyDescriptor<ITape>(head,offsetPtr)),ptrDescToDelete(true)
      ,position(0),fZERO(true)
    {
      ptrDesc->get_value(ptr);
      this->init_cells();
    }
    mwtfile_celltape(tape_head<ITape>& head,Bath& bath,iprop<DataPtr>* ptrDesc,bool ptrDescToDelete=false)
      :bath(bath),head(head)
      ,ptrDesc(ptrDesc),ptrDescToDelete(ptrDescToDelete)
      ,position(0),fZERO(true)
    {
      ptrDesc->get_value(ptr);
      this->init_cells();
    }
    ~mwtfile_celltape(){
      if(ptrDesc!=nullptr){
        ptrDesc->set_value(ptr);
        if(ptrDescToDelete)
          delete ptrDesc;
        ptrDesc=nullptr;
      }
      head.flush();
    }
  private:
    void init_cells(){
      i4t bid=ptr.offset;
      while(!BID::isSpecial(bid)){
        // ERR: 循環検出 ■ 以下の物は O(N^2)
        for(std::vector<i4t>::iterator i=cells.begin(),iN=cells.end();i<iN;++i){
          if(*i==bid){
            bath.cell_setNext(cells.back(),BID_LAST);
            break;
          }
        }

        cells.push_back(bid);
        bid=bath.cell_getNext(bid);
      }

      // 不正修正: length と cell数の不一致
      u4t ncell=cells.size();
      u4t mcell=Bath::GetRequiredCellCount(ptr.length);
      if(ncell<mcell){
        ptr.length=cells.size()*CellSize;
      }else if(ncell>mcell){
        i4t length=ptr.length;
        ptr.length=cells.size()*CellSize;
        this->trunc(length);
      }
    }
  //---------------------------------------------------------------------------
  //  itape その他
  //---------------------------------------------------------------------------
  public:
    bool can_read() const{return true;}
    bool can_write() const{return true;}
    bool can_seek() const{return true;}
    bool can_trunc() const{return true;}
    i8t tell() const{return position;}
    u8t size() const{return this->ptr.length;}
    int flush() const{
      return head.flush();
    }
  //---------------------------------------------------------------------------
  //  u8t trunc(u8t) const;
  //---------------------------------------------------------------------------
  public:
    int trunc(u8t size) const{
      if(size>=0x80000000){
        errno=EINVAL;
        return -1;
      }
      int e=errno;

      u4t nsize=size;
      u4t osize=this->ptr.length;
      i4t ncell=Bath::GetRequiredCellCount(nsize);
      i4t ocell=this->cells.size();

      if(ocell>ncell){
        // [ブロック解放]
        std::vector<i4t>::iterator i0=this->cells.begin()+ncell,iN=this->cells.end();
        for(std::vector<i4t>::iterator i=i0;i<iN;i++)bath.cell_free(*i);
        this->cells.erase(i0,iN);

        if(this->cells.size()>0)
          bath.cell_setNext(this->cells.back(),BID_LAST);
        else
          ptr.offset=BID_LAST;
      }else if(ocell<ncell){
        // [ブロック追加]
        this->ensure_ncell(ncell);

        if(this->fZERO){
          // zero padding
          u4t offs=osize%CellSize;
          i4t iblk=osize/CellSize;
          if(offs){
            head.seek(cells[iblk]*CellSize+offs);
            head.fill_n<byte>(0,CellSize-offs);
            offs=0;
            iblk++;
          }
        }
      }else if(osize<nsize&&this->fZERO){
        // [ブロック内伸張]
        // zero padding
        u4t offs=osize%CellSize;
        i4t iblk=osize/CellSize;
        head.seek(cells[iblk]*CellSize+offs);
        head.fill_n<byte>(0,nsize-osize);
      }

      this->ptr.length=nsize;
      if(this->position>nsize)
        this->position=nsize;

      return !e&&errno?-1:0;
    }
  private:
    void ensure_ncell(i4t n) const{
      i4t i=this->cells.size();
      if(n<=i)return;

      i4t bidNew=0;
      for(;i<n;i++){
        bidNew=bath.cell_alloc(bidNew+1);
        this->cells.push_back(bidNew);

        // link
        if(i)
          bath.cell_setNext(cells[i-1],bidNew);
        else
          ptr.offset=bidNew;

        // padding
        if(this->fZERO){
          head.seek(bidNew*CellSize);
          head.fill_n<byte>(0,CellSize);
        }
      }

      bath.cell_setNext(cells[i-1],BID_LAST);
    }
  //---------------------------------------------------------------------------
  //  int seek(i8t,int) const;
  //---------------------------------------------------------------------------
  public:
    int seek(i8t offset,int whence=SEEK_SET) const{
      if(whence==SEEK_SET){
        int e=errno;
        this->position=(offset<0?0:offset>=0x80000000?0x7FFFFFFF:offset);
        if(this->position>ptr.length)
          this->trunc(this->position);

        return !e&&errno?-1:0;
      }else if(whence==SEEK_CUR){
        return this->seek(this->tell()+offset);
      }else if(whence==SEEK_END){ // SEEK_END
        return this->seek(ptr.length+offset);
      }else{
        throw mwg::bio::nosupport_error("an unknown seek origin.");
      }
    }
  private:
    void _seekpos() const{
      i4t p_iblk=position/CellSize;
      i4t p_offs=position%CellSize;
      i8t pos=cells[p_iblk]*(i8t)CellSize+p_offs;
      if(head.tell()!=pos)head.seek(pos);
    }
  //---------------------------------------------------------------------------
  //  int read(void*,int,int) const;
  //---------------------------------------------------------------------------
  public:
    int read(void* buff,int size,int n=1) const{
      byte* p=(byte*)buff;

      // 読取可能
      int nmax=(ptr.length-position)/size;
      if(n>nmax)n=nmax;
      if(n<=0)return 0;

      i8t posN=position+n*size;
      while(position<posN){
        i4t len=std::min<i4t>(posN-position,CellSize-position%CellSize);
        _seekpos();head.read_data(p,1,len);
        position+=len;
        p+=len;
      }

      return n;
    }
  //---------------------------------------------------------------------------
  //  int write(void*,int,int) const;
  //---------------------------------------------------------------------------
  public:
    int write(const void* buff,int size,int n=1) const{
      if(n<=0)return 0;
      byte* p=(byte*)buff;
      i8t posN=position+n*size;

      // 拡張
      if(posN>ptr.length){
        fZERO=false; // suppress zero padding
        trunc(posN);
        fZERO=true;
      }

      while(position<posN){
        i4t len=std::min<i4t>(posN-position,CellSize-position%CellSize);
        _seekpos();head.write_data(p,1,len);
        position+=len;
        p+=len;
      }

      return n;
    }
  };
  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  template<typename FTape,int CellSize>
  struct celltape_factory{
    typedef tape_head<FTape>        FHead;
    typedef mwtfile_cellbath<FTape> BBath;
    typedef mwtfile_celltape<FTape> BTape;
    BTape                            tape;
    tape_head<BTape>                 head;
    mwtfile_cellbath<BTape,CellSize> bath;

    typedef mwtfile_celltape<BTape,CellSize> celltape_type;
  public:
    celltape_factory(FHead& head,BBath& bath,i8t ptrOffset)
      :tape(head,bath,ptrOffset)
      ,head(this->tape)
      ,bath(this->head)
    {}
    celltape_factory(FHead& head,BBath& bath,iprop<DataPtr>* ptrDesc,bool descToDelete)
      :tape(head,bath,ptrDesc,descToDelete)
      ,head(this->tape)
      ,bath(this->head)
    {}
  //public:
  //  celltape_type* create_celltape(){}
  };
  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  template<typename FTape>
  class mwtfile_autotape:public itape{
    mwtfile_manager<FTape>& m;

    mutable iptrtape* tape;
    typedef mwtfile_celltape<FTape>     tapeB;
    typedef mwtfile_celltape<tapeB,256> tape8;
    typedef mwtfile_celltape<tapeB, 64> tape6;
    typedef mwtfile_celltape<tapeB, 16> tape4;

    iprop<DataPtr>* ptrDesc;
    bool const      ptrDescToDelete;

    class ValuePtrDesc:public iprop<DataPtr>{
      DataPtr value;
    public:
      ValuePtrDesc(const DataPtr& value):value(value){}
      virtual void get_value(DataPtr& value) const{
        value=this->value;
      }
      virtual void set_value(const DataPtr& value) const{}
    };
  private:
    mwtfile_autotape(const mwtfile_autotape&) mwg_std_deleted;
    mwtfile_autotape& operator=(const mwtfile_autotape&) mwg_std_deleted;
  public:
    void get_dataptr(DataPtr& ptr) const{
      if(tape==nullptr){
        ptr.offset=BID_LAST;
        ptr.length=0;
      }else{
        tape->get_dataptr(ptr);
      }
    }
  public:
    mwtfile_autotape(mwtfile_manager<FTape>& m,i8t offsetPtr)
      :m(m)
      ,ptrDesc(new PtrBPropertyDescriptor<FTape>(m.head,offsetPtr)),ptrDescToDelete(true)
    {
      DataPtr ptr;
      ptrDesc->get_value(ptr);
      this->init(ptr);
    }
    mwtfile_autotape(mwtfile_manager<FTape>& m,iprop<DataPtr>* ptrDesc,bool ptrDescToDelete=false)
      :m(m)
      ,ptrDesc(ptrDesc),ptrDescToDelete(ptrDescToDelete)
    {
      DataPtr ptr;
      ptrDesc->get_value(ptr);
      this->init(ptr);
    }
    ~mwtfile_autotape(){
      std::printf("dbg: ~mwtfile_autotape!\n");
      this->grain_minify();
      std::printf("dbg: ~mwtfile_autotape! 1\n");

      //- テープ
      DataPtr ptr;this->get_dataptr(ptr);
      if(tape!=nullptr){
        delete tape; // ptr が更新される
        tape=nullptr;
      }

      std::printf("dbg: ~mwtfile_autotape! 2\n");
      //- 書き戻し
      if(ptrDesc!=nullptr){
        ptrDesc->set_value(ptr);
        if(ptrDescToDelete)delete ptrDesc;
        ptrDesc=nullptr;
      }
      std::printf("dbg: ~mwtfile_autotape! 3\n");
    }
  private:
    void init(const DataPtr& ptr){
      if(ptr.length>LENGTH_THRESH_BLOCK){
        grain=GRAIN_BLOCK;
        tape=new tapeB(m.head,*m.bath,new ValuePtrDesc(ptr),true);
      }else if(ptr.length>LENGTH_THRESH_256){
        grain=GRAIN_256;
        tape=new tape8(m.fat256->head,m.fat256->bath,new ValuePtrDesc(ptr),true);
      }else if(ptr.length>LENGTH_THRESH_64){
        grain=GRAIN_64;
        tape=new tape6(m.fat64->head,m.fat64->bath,new ValuePtrDesc(ptr),true);
      }else if(ptr.length>0){
        grain=GRAIN_16;
        tape=new tape4(m.fat16->head,m.fat16->bath,new ValuePtrDesc(ptr),true);
      }else{
        grain=0;
        tape=nullptr;
      }
    }
  //---------------------------------------------------------------------------
  //  Grain control
  //---------------------------------------------------------------------------
  private:
    mutable int grain;
    static const int GRAIN_BLOCK  =4;
    static const int GRAIN_256    =3;
    static const int GRAIN_64     =2;
    static const int GRAIN_16     =1;
    static const int GRAIN_NONE   =0;
    static const int LENGTH_THRESH_BLOCK=0x1000; // 4kB
    static const int LENGTH_THRESH_256  =0x0400; // 1kB
    static const int LENGTH_THRESH_64   =0x0100; // 256B
    static int grain_required(const u8t& size){
      if(size>LENGTH_THRESH_BLOCK)
        return GRAIN_BLOCK;
      else if(size>LENGTH_THRESH_256)
        return GRAIN_256;
      else if(size>LENGTH_THRESH_64)
        return GRAIN_64;
      else if(size>0)
        return GRAIN_16;
      else
        return GRAIN_NONE;
    }
  private:
    void grain_switch(int ngrain) const{
      std::printf("dbg: now switching grain... %d -> %d\n",grain,ngrain);
      grain=ngrain;

      //- 新規テープ
      DataPtr ptr;
      ptr.offset=BID_LAST;
      ptr.length=0;
      iptrtape* ntape=nullptr;
      switch(ngrain){
      case GRAIN_BLOCK:
        ntape=new tapeB(m.head,*m.bath,new ValuePtrDesc(ptr),true);
        break;
      case GRAIN_256:
        ntape=new tape8(m.fat256->head,m.fat256->bath,new ValuePtrDesc(ptr),true);
        break;
      case GRAIN_64:
        ntape=new tape6(m.fat64->head,m.fat64->bath,new ValuePtrDesc(ptr),true);
        break;
      case GRAIN_16:
        ntape=new tape4(m.fat16->head,m.fat16->bath,new ValuePtrDesc(ptr),true);
        break;
      default:
        mwg_assert(false&&"invalid code path");
        return;
      }

      //- 内容転写
      if(this->tape!=nullptr){
        const int BUFF_SIZE=256;
        byte buff[BUFF_SIZE];
        int n;
        tape->seek(0);
        while(n=tape->read(buff,sizeof(byte),BUFF_SIZE))
          ntape->write(buff,sizeof(byte),n);
        this->tape->trunc(0);
      }

      //- 差し換え
      delete this->tape;
      this->tape=ntape;
    }
  private:
    void grain_update(u8t size) const{
      int ngrain=grain_required(size);
      if(ngrain<=grain)return;
      this->grain_switch(ngrain);
    }
    void grain_minify() const{
      //std::printf("dbg: grain_minify: 0\n");
      int ngrain=grain_required(tape==nullptr?0:tape->size());
      std::printf("dbg: grain_minify: grain=%d ngrain=%d\n",grain,ngrain);
      mwg_assert(ngrain<=grain);
      if(ngrain==grain)return;
      this->grain_switch(ngrain);
    }
  //---------------------------------------------------------------------------
  //  Tape Properties
  //---------------------------------------------------------------------------
  public:
    bool can_read() const{return true;}
    bool can_write() const{return true;}
    bool can_seek() const{return true;}
    bool can_trunc() const{return true;}
    i8t tell() const{return tape==nullptr?0:tape->tell();}
    u8t size() const{return tape==nullptr?0:tape->size();}
    int flush() const{return tape==nullptr?0:tape->flush();}
  //---------------------------------------------------------------------------
  //  Tape Function
  //---------------------------------------------------------------------------
  public:
    int trunc(u8t size) const{
      if(tape==nullptr){
        if(size<0){errno=EINVAL;return -1;}
        if(size==0)return 0;
        this->grain_update(size);
        return tape->trunc(size);
      }else{
        int ret=tape->trunc(size);
        this->grain_update(tape->size());
        return ret;
      }
    }
    int seek(i8t offset,int whence=SEEK_SET) const{
      if(tape==nullptr){
        if(offset<0){errno=EINVAL;return -1;}
        if(offset==0)return 0;
        this->grain_update(offset);
        return tape->seek(offset,whence);
      }else{
        int ret=tape->seek(offset,whence);
        this->grain_update(tape->size());
        return ret;
      }
    }
    int read(void* buff,int size,int n=1) const{
      if(tape==nullptr)return 0;
      return tape->read(buff,size,n);
    }
    int write(const void* buff,int size,int n=1) const{
      if(tape==nullptr){
        this->grain_update(size);
        return tape->write(buff,size,n);
      }else{
        n=tape->write(buff,size,n);
        this->grain_update(tape->size());
        return n;
      }
    }
  };

  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  //  Node 管理
  struct NodeIndex0{
    i4t magic;
    i4t count;
    i4t padding1_;
  };
  struct NodeIndex{
    byte state; // 0|'B'|'8'|'6'|'4'|'0'
    byte rlock; // integer
    byte wlock; // 0|'w'
    byte reserved_;

    DataPtr ptr;
  };
#if 0 /* ■設計検討中■ */
  template<typename FTape>
  class Node{
    // '8' '6' '4' '0'
    mwtfile_manager<FTape>& m;
    NID nid;
    NodeIndex index;
    bool dirty;
    //std::map<std::string,std::shared_ptr<std::font> > props;
    //std::vector<std::string,std::shared_ptr<std::string> >          attrs;
  public:
    Node(mwtfile_manager<FTape>& m,NID nid):m(m),nid(nid){
      if(!m.nodelist->nid_lock(nid))
        throw std::logic_error("node_is_locked"); // ■
      m.nodelist->nid_getNodeIndex(nid,index);

      this->read_content();
      this->dirty=false;
    }
    ~Node(){
      m.nodelist->nid_setNodeIndex(nid,index);
      m.nodelist->nid_unlock(nid);
    }
    /*
      現在の lock の実装について
        誰かがファイルを開いていると、
        他のプロセスからは例えファイル名を取得するだけでもアクセス不可能になってしまう。
    */
  private:
    void read_content(){
      // ■
    }
    itape* get_filestream(){
      // ■
    }
  };
#endif
  template<typename FTape>
  struct mwtfile_nodelist{
    mwtfile_celltape<FTape>             tape;
    tape_head<mwtfile_celltape<FTape> > head;
  public:
    mwtfile_nodelist(mwtfile_celltape<FTape>& head,mwtfile_cellbath<FTape>& bath)
      :tape(head,bath,BlockSize+offsetof(RootBlock,listNodeIndex))
      ,head(this->tape)
    {}
  public:
    static const int NNodeInPlane =1024;
    static const i8t PlaneSize    =NNodeInPlane*sizeof(NodeIndex);
    stdm::shared_ptr<NID> createNode(){
      NID nid=nid_alloc(1);
      return new NID(nid);
      // dbg:
    }
  private:
    NID nid_alloc(NID nidStart){
      i4t iPlane=nidStart/nidStart;
      i4t iNode =nidStart%nidStart;

      if(iNode<1)iNode=1;
      for(;;iPlane++,iNode=1){
        // [新規 plane]
        if(PlaneSize*iPlane>=head.size()){
          NodeIndex0 node0={('N'|'I'<<8|'D'<<16|'s'<<24),1};

          NodeIndex node1={0};
          node1.state='0';
          node1.ptr.offset=BID_LAST;
          node1.ptr.length=0;

          head.trunc(PlaneSize*(iPlane+1));
          head.seek(PlaneSize*iPlane);
          head.write(node0);
          head.write(node1);
          head.fill_n<byte>(0,sizeof(NodeIndex)*(NNodeInPlane-2));
          return iPlane*NNodeInPlane+1;
        }

        // [使用ノード数]
        i4t c;
        head.seek(PlaneSize*iPlane+offsetof(NodeIndex0,count));
        head.read(c);
        if(c==NNodeInPlane-1)continue;

        for(;iNode<NNodeInPlane;iNode++){
          byte state;
          head.seek(PlaneSize*iPlane+sizeof(NodeIndex)*iNode+offsetof(NodeIndex,state));
          head.read(state);
          if(state!=0)continue;

          // [発見, 使用ノード数更新]
          c++;
          head.seek(PlaneSize*iPlane+offsetof(NodeIndex0,count));
          head.write(c);
          return iPlane*NNodeInPlane+iNode;
        }
      }
    }
    void nid_getNodeIndex(NID nid,NodeIndex& index){
      head.seek(nid*sizeof(NodeIndex));
      head.read(index); // ■ FMT_LE
    }
    void nid_setNodeIndex(NID nid,const NodeIndex& index){
      head.seek(nid*sizeof(NodeIndex));
      head.write(index); // ■ FMT_LE
    }
  };

  //ttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt
  template<typename ITape>
  struct mwtfile_manager{
    typedef mwtfile_celltape<ITape> block_tape;

    tape_head<ITape>           head;
    mwtfile_cellbath<ITape>*   bath;

    celltape_factory<ITape,256>* fat256;
    celltape_factory<ITape,64>*  fat64;
    celltape_factory<ITape,16>*  fat16;

    block_tape* nodelist;

  private:
    mwtfile_manager(const mwtfile_manager&) mwg_std_deleted;
    mwtfile_manager& operator=(const mwtfile_manager&) mwg_std_deleted;
  public:
    mwtfile_manager(const ITape& tape):head(tape),bath(nullptr){
      if(!tape.can_read())
        throw std::invalid_argument("the specified tape cannot be read.");
      if(!tape.can_write())
        throw std::invalid_argument("the specified tape cannot be written.");
      if(!tape.can_seek())
        throw std::invalid_argument("the specified tape cannot be seeked.");

      if(tape.size()==0)
        this->create_mwtfile();
      else{
        head.seek(4);
        i4t bid1;
        head.read(bid1,FMT_LE);
        if(bid1!=BID_ROOT)
          throw mwg::bio::ill_format_error("the specified tape is not mwt-formatted.",mwg::ecode::EArgument);
      }

      this->bath=new mwtfile_cellbath<ITape>(head);

      this->fat256=new celltape_factory<ITape,256>(head,*bath,BlockSize+offsetof(RootBlock,Fat256));
      this->fat64 =new celltape_factory<ITape,64> (head,*bath,BlockSize+offsetof(RootBlock,Fat64));
      this->fat16 =new celltape_factory<ITape,16> (head,*bath,BlockSize+offsetof(RootBlock,Fat16));

      this->nodelist=new mwtfile_celltape<ITape>(head,*bath,BlockSize+offsetof(RootBlock,listNodeIndex));
    }
    ~mwtfile_manager(){
      if(this->bath!=nullptr){
        delete this->nodelist;this->nodelist=nullptr;

        delete this->fat16;this->fat16=nullptr;
        delete this->fat64;this->fat64=nullptr;
        delete this->fat256;this->fat256=nullptr;

        delete this->bath;this->bath=nullptr;
      }
    }
  private:
    void create_mwtfile(){
      head.template write<i4t>(1);
      head.write(BID_ROOT,FMT_LE);
      for(int i=2;i<BlockSize/4;i++)head.write(BID_NOTUSED,FMT_LE);

      // RootBlock
      RootBlock rootBlock={MAGICR};
      rootBlock.Fat256        .offset=BID_LAST;
      rootBlock.Fat64         .offset=BID_LAST;
      rootBlock.Fat16         .offset=BID_LAST;
      rootBlock.listNodeIndex .offset=BID_LAST;
      rootBlock.listTagIndex  .offset=BID_LAST;
      rootBlock.HeapList      .offset=BID_LAST;
      rootBlock.HeapData      .offset=BID_LAST;
      rootBlock.RootDir       .offset=BID_LAST;
      head.write(rootBlock);
    }
  };

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
# ifdef _MSC_VER
#   pragma warning(pop)
# endif
#endif
