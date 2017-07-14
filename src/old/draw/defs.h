// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_DRAW_DEFS_H
#define MWG_DRAW_DEFS_H
#include <vector>
#include <cmath>
#include <mwg/defs.h>
#include <mwg/except.h>
#include <mwg/exp/attr.h>
namespace mwg{
namespace draw1{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  // mwg_define_class_error_ex(draw_error      ,mwg::except,mwg::ecode::draw);
  // mwg_define_class_error_ex(nosupport_error ,io_error   ,mwg::ecode::ESupport);
  // mwg_define_class_error_ex(ill_format_error,io_error   ,0);

  template<typename T>
  struct rectangle{
    T x; //!< x value of top-left corner
    T y; //!< y value of top-left corner
    T w; //!< width
    T h; //!< height
  public:
    rectangle(const T& x,const T& y,const T& w,const T& h)
      :x(x),y(y),w(w),h(h){}
    rectangle(const mwg::range<T>& xrange,const mwg::range<T>& yrange)
      :x(xrange.begin()),y(yrange.begin())
      ,w(xrange.length()),h(yrange.length()){}
    T left()   const{return this->x;}
    T right()  const{return this->x+this->w;}
    T width()  const{return this->w;}
    T top()    const{return this->y;}
    T bottom() const{return this->y+this->h;}
    T height() const{return this->h;}
    T center() const{return this->x+0.5*this->w;}
    T middle() const{return this->y+0.5*this->h;}
    mwg::range<T> xrange() const{return mwg::range<T>(this->x,this->x+this->w);}
    mwg::range<T> yrange() const{return mwg::range<T>(this->y,this->y+this->h);}
  };

  template<typename T>
  class matrix1{
    T x10;
    T x11;
  public:
    matrix1(const T& x10,const T& x11):x10(x10),x11(x11){}
    matrix1(const T& dstX,const T& dstW,const T& srcX,const T& srcW){
      this->x11=dstW/srcW;
      this->x10=dstX-x11*srcX;
    }
    T transform(const T& value) const{
      return x10+x11*value;
    }
    T dtransform(const T& value) const{
      return x11*value;
    }
    T itransform(const T& value) const{
      return (value-x10)/x11;
    }
    T idtransform(const T& value) const{
      return value/x11;
    }

    void transformD(T& value) const{
      value=x10+x11*value;
    }
    void transformD(T& value,T& delta) const{
      value=x10+x11*value;
      delta=    x11*delta;
    }
  };

  template<typename T>
  struct vector2{
    T x;
    T y;
  public:
    vector2(T x,T y):x(x),y(y){}
  };

//-----------------------------------------------------------------------------

  class ifont{
  public:
    virtual ~ifont(){}
  };

  struct FontNameType{
    enum enum_t{
      FilePath,
      FontName,
    };
  };
  struct FontStyle{
    enum enum_t{
      Normal =0,
      Slanted=0x01,
      Italic =0x02,
      Bold   =0x10,
    };
  };

  struct FontInfo:public ifont{
    FontNameType::enum_t name_type;
    std::string name;

    FontStyle::enum_t style;
    double size;
  };

  mwg_attr_define_flags(TextAlignment,dword,enum{
    Default=0,

    Mask_HorizontalAlign=0x0F,
    Left                =0x01,
    Center              =0x02,
    Right               =0x03,

    Mask_VerticalAlign  =0xF0,
    Top                 =0x10,
    Middle              =0x20,
    Bottom              =0x30,
  });

//-----------------------------------------------------------------------------
  class icanvas;
  class scoped_canvas_gsave;

  class scoped_canvas_gsave{
    icanvas& c;
  public:
    scoped_canvas_gsave(icanvas* c);
    scoped_canvas_gsave(icanvas& c);
    ~scoped_canvas_gsave();
  };

  class icanvas{
  public:
    virtual ~icanvas(){};
  public:
    virtual void gsave()=0;
    virtual void grestore()=0;

    virtual void internal_moveto(double x,double y)=0;
    virtual void internal_rmoveto(double dx,double dy)=0;
    virtual void internal_lineto(double x,double y)=0;
    virtual void internal_rlineto(double dx,double dy)=0;
    void moveto (double x,double y)       {this->internal_moveto(x,y);}
    void moveto (const vector2<double>& v){this->internal_moveto(v.x,v.y);}
    void rmoveto(double x,double y)       {this->internal_rmoveto(x,y);}
    void rmoveto(const vector2<double>& v){this->internal_rmoveto(v.x,v.y);}
    void lineto (double x,double y)       {this->internal_lineto(x,y);}
    void lineto (const vector2<double>& v){this->internal_lineto(v.x,v.y);}
    void rlineto(double x,double y)       {this->internal_rlineto(x,y);}
    void rlineto(const vector2<double>& v){this->internal_rlineto(v.x,v.y);}
    virtual void internal_arc(double x,double y,double r,double a1,double a2)=0;
    virtual void internal_newpath()=0;
    void arc(double x,double y,double r,double a1,double a2){
      this->internal_arc(x,y,r,a1,a2);
    }
    void newpath(){
      this->internal_newpath();
    }

    virtual void stroke()=0;
    virtual void fill()=0;
    virtual void closepath()=0;

    virtual void internal_setrgbcolor(double r,double g,double b,double a)=0;
    void setrgbcolor(double r,double g,double b,double a){
      this->internal_setrgbcolor(r,g,b,a);
    }
    void setrgbcolor(double r,double g,double b){
      this->internal_setrgbcolor(r,g,b,1);
    }
    // void setrgbcolor(mwg::u1t r,mwg::u1t g,mwg::u1t b,mwg::u1t a){
    //   this->internal_setrgbcolor(r/255.0,g/255.0,b/255.0,a/255.0);
    // }
    // void setrgbcolor(mwg::u1t r,mwg::u1t g,mwg::u1t b){
    //   this->internal_setrgbcolor(r/255.0,g/255.0,b/255.0,1);
    // }
    void setrgbcolor(mwg::u4t argb){
      byte a=argb>>24&0xFF;
      byte r=argb>>16&0xFF;
      byte g=argb>> 8&0xFF;
      byte b=argb    &0xFF;
      this->internal_setrgbcolor(r/255.0,g/255.0,b/255.0,(0xFF-a)/255.0);
    }

    virtual void setlinewidth(double w)=0;
    virtual void translate(double dx,double dy)=0;
    virtual void internal_rotate(double radian)=0;
    void rotate(double angle){
      this->internal_rotate(angle);
    }
    void rotate_rad(double radian){
      this->internal_rotate(radian*(180/M_PI));
    }

    virtual void internal_show(const char* text)=0;
    virtual void internal_stringwidth(const char* text,double& w,double& h)=0;
    void show(const char* text){
      this->internal_show(text);
    }
    void show(const char* text,TextAlignment align){
      if(align==TextAlignment::Default)
        return this->internal_show(text);

      double w=1;
      double h=1;
      this->internal_stringwidth(text,w,h);

      double x;
      switch(align&TextAlignment::Mask_HorizontalAlign){
      default:
      case TextAlignment::Left:
        x=0;
        break;
      case TextAlignment::Center:
        x=-w*0.5;
        break;
      case TextAlignment::Right:
        x=-w;
        break;
      }

      double y;
      switch(align&TextAlignment::Mask_VerticalAlign){
      default:
      case TextAlignment::Bottom:
        y=0;
        break;
      case TextAlignment::Middle:
        y=h*0.5;
        break;
      case TextAlignment::Top:
        y=h;
        break;
      }

      this->internal_rmoveto(x,y);
      this->internal_show(text);
    }

  public:
    virtual ifont* create_font(const FontInfo& info)=0;
    virtual bool set_font(ifont* font)=0;
  };

  scoped_canvas_gsave::scoped_canvas_gsave(icanvas* c):c(*c){c->gsave();}
  scoped_canvas_gsave::scoped_canvas_gsave(icanvas& c):c(c){c.gsave();}
  scoped_canvas_gsave::~scoped_canvas_gsave(){c.grestore();}

  class multicast_canvas:mwg::attr::noncopyable,public icanvas{
    std::vector<icanvas*> resource;
    std::vector<icanvas*> children;
    typedef std::vector<icanvas*>::iterator iterator;
  public:
    multicast_canvas(){}
    ~multicast_canvas(){
      for(iterator i=resource.begin();i!=resource.end();i++)
        delete *i;
      resource.clear();
    }

    void add(icanvas* child,bool autoDelete=false){
      children.push_back(child);
      if(autoDelete)
        resource.push_back(child);
    }

  public:
    virtual void gsave(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->gsave();
    }
    virtual void grestore(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->grestore();
    }
    virtual void internal_moveto(double x,double y){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_moveto(x,y);
    }
    virtual void internal_rmoveto(double dx,double dy){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_rmoveto(dx,dy);
    }
    virtual void internal_lineto(double x,double y){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_lineto(x,y);
    }
    virtual void internal_rlineto(double dx,double dy){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_rlineto(dx,dy);
    }
    virtual void internal_arc(double x,double y,double r,double a1,double a2){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_arc(x,y,r,a1,a2);
    }
    virtual void internal_newpath(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_newpath();
    }
    virtual void stroke(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->stroke();
    }
    virtual void fill(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->fill();
    }
    virtual void closepath(){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->closepath();
    }
    virtual void internal_setrgbcolor(double r,double g,double b,double a){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_setrgbcolor(r,g,b,a);
    }
    virtual void setlinewidth(double w){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->setlinewidth(w);
    }
    virtual void translate(double dx,double dy){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->translate(dx,dy);
    }
    virtual void internal_rotate(double radian){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_rotate(radian);
    }
    virtual void internal_show(const char* text){
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        (*i)->internal_show(text);
    }
    virtual void internal_stringwidth(const char* text,double& w,double& h){
      iterator i=children.begin();
      if(i!=children.end())
        (*i)->internal_stringwidth(text,w,h);
    }
  private:
    // template<typename T>
    // struct auto_vector{
    //   typedef std::vector<T*>::iterator iterator;
    //   typedef std::vector<T*>::const_iterator const_iterator;
    //   std::vector<T*> list;
    //   ~auto_vector(){
    // 	for(iterator i=list.begin();i!=list.end();i++)
    // 	  delete *i;
    // 	list.clear();
    //   }
    //   iterator begin()            {return this->list.begin();}
    //   iterator end()              {return this->list.end();}
    //   const_iterator begin() const{return this->list.begin();}
    //   const_iterator end() const  {return this->list.end();}
    //   void push_back(T* p)        {this->list.push_back(p);}
    // };
    // struct multicast_font:public ifont{
    //   mwg::stdm::shared_ptr<auto_vector<ifont> > fonts;
    // };
    struct multicast_font:public ifont{
      std::vector<mwg::stdm::shared_ptr<ifont> > fonts;
    };
  public:
    virtual ifont* create_font(const FontInfo& info){
      multicast_font* ret=new multicast_font;
      for(iterator i=children.begin(),iN=children.end();i!=iN;i++)
        ret->fonts.push_back(mwg::stdm::shared_ptr<ifont>((*i)->create_font(info)));
      return ret;
    }
    virtual bool set_font(ifont* font){
      multicast_font* f=dynamic_cast<multicast_font*>(font);
      if(f==nullptr){
        return false;
      	// if(font==nullptr)
      	//   throw std::invalid_argument("mwg::draw::multicast_canvas::set_font: the specified font is null.");
      	// else
      	//   throw std::invalid_argument("mwg::draw::multicast_canvas::set_font: the specified font may be initialized with other canvas.");
      }else if(f->fonts.size()!=this->children.size()){
        return false;
      }

      for(std::size_t i=0,iN=f->fonts.size();i<iN;i++)
        this->children[i]->set_font(f->fonts[i].get());
      return true;
    }
  };
//-----------------------------------------------------------------------------

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}

  namespace draw=mwg::draw1;
}
#endif
