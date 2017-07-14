// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_DRAW_PLOT_H
#define MWG_DRAW_PLOT_H
#include <mwg/range.h>
#include <mwg/draw/defs.h>
#include <mwg/stat/errored_value.h>
#include <mwg/exp/attr.h>
namespace mwg{
namespace draw1{
namespace plot{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

//#define MWG_DRAW_PLOT_H__IMPL1
#define MWG_DRAW_PLOT_H__IMPL2

#ifdef MWG_DRAW_PLOT_H__IMPL1
  class plot_histogram_impl1{
    icanvas* c;
    rectangle<double> dstRect;
    matrix1<double> matX;
    matrix1<double> matY;
  public:
    plot_histogram_impl1(icanvas& c,const rectangle<double>& dstRect,const rectangle<double>& srcRect)
      :c(&c),dstRect(dstRect)
      ,matX(dstRect.x,dstRect.w,srcRect.x,srcRect.w)
      ,matY(dstRect.bottom(),-dstRect.h,srcRect.y,srcRect.h)
    {}
    void draw_frame() const{
      double const w=dstRect.w;
      double const h=dstRect.h;
      c->moveto (dstRect.x,dstRect.y);
      c->rlineto(w ,0 );
      c->rlineto(0 ,h );
      c->rlineto(-w,0 );
      c->rlineto(0 ,-h);
      c->stroke();
    }

    void draw_back_hist1(double x,double dx,int ir) const{
      if(ir&1)
        c->setrgbcolor(0.95,0.95,0.95);
      else
        c->setrgbcolor(1.0,1.0,1.0);

      matX.transformD(x,dx);
      c->moveto (x-dx,dstRect.y);
      c->rlineto(2*dx ,0        );
      c->rlineto(0    ,dstRect.h);
      c->rlineto(-2*dx,0        );
      c->closepath();
      c->fill();
    }
    void draw_mark_hist1(double x,double y,double dx,double dy) const{
      matX.transformD(x,dx);
      matY.transformD(y,dy);
      c->setlinewidth(1);
      c->moveto(x+dx,y);
      c->lineto(x-dx,y);
      c->moveto(x   ,y+dy);
      c->lineto(x   ,y-dy);
      c->moveto(x-3 ,y+dy);
      c->lineto(x+3 ,y+dy);
      c->moveto(x-3 ,y-dy);
      c->lineto(x+3 ,y-dy);
      c->stroke();
    }

    void draw_back_hist1(const mwg::range<double>& x,int ir) const{
      if(ir&1)
        c->setrgbcolor(0.95,0.95,0.95);
      else
        c->setrgbcolor(1.0,1.0,1.0);

      double const x1=matX.transform(x.begin());
      double const x2=matX.transform(x.end());
      c->moveto(x1,dstRect.top());
      c->lineto(x2,dstRect.top());
      c->lineto(x2,dstRect.bottom());
      c->lineto(x1,dstRect.bottom());
      c->closepath();
      c->fill();
    }
    void draw_mark_hist1(const mwg::range<double>& x,const mwg::stat::errored<double>& y) const{
      double const x1=matX.transform(x.begin());
      double const x2=matX.transform(x.end());
      double const x0=(x1+x2)*0.5;
      double const dx=std::max((x2-x1)*0.10,3.0);
      double const y0=matY.transform(y.value());
      double const dy=matY.dtransform(y.error());
      c->setlinewidth(1);
      c->moveto(x1   ,y0   );
      c->lineto(x2   ,y0   );
      c->moveto(x0   ,y0+dy);
      c->lineto(x0   ,y0-dy);
      c->moveto(x0-dx,y0+dy);
      c->lineto(x0+dx,y0+dy);
      c->moveto(x0-dx,y0-dy);
      c->lineto(x0+dx,y0-dy);
      c->stroke();
    }
    void draw_line_plot(double x1,double y1,double x2,double y2) const{
      c->setlinewidth(1);
      c->moveto(matX.transform(x1),matY.transform(y1));
      c->lineto(matX.transform(x2),matY.transform(y2));
      c->stroke();
    }

    vector2<double> transform(const vector2<double>& v) const{
      return vector2<double>(matX.transform(v.x),matY.transform(v.y));
    }
    vector2<double> itransform(const vector2<double>& v) const{
      return vector2<double>(matX.itransform(v.x),matY.itransform(v.y));
    }
  };
#endif

#ifdef MWG_DRAW_PLOT_H__IMPL2
  mwg_attr_define_flags(PlotMarkType,mwg::dword,enum{
    Default          =0,
    Point            ,
    Circle           ,
    Cross            ,
    Plus             ,
    TriangleU        ,
    TriangleD        ,
    TriangleR        ,
    TriangleL        ,
    Square           ,
    Diamond          ,
    Star5            ,
    Asterisk         ,
    Star6            ,

    /* reserved */
    SizeNormal       =0,
    SizeLarge        =0x100,
    SizeSmall        =0x200,
    SizeLarger       =0x300,
    SizeSmaller      =0x400,
  });

  class PlotHelperImpl2{
    icanvas* c;
    rectangle<double> dstRect;
    matrix1<double> matX;
    matrix1<double> matY;
  public:
    PlotHelperImpl2(icanvas& c,const rectangle<double>& dstRect,const rectangle<double>& srcRect)
      :c(&c),dstRect(dstRect)
      ,matX(dstRect.x,dstRect.w,srcRect.x,srcRect.w)
      ,matY(dstRect.bottom(),-dstRect.h,srcRect.y,srcRect.h)
    {}
    void draw_frame() const{
      double const w=dstRect.w;
      double const h=dstRect.h;
      c->moveto (dstRect.x,dstRect.y);
      c->rlineto(w ,0 );
      c->rlineto(0 ,h );
      c->rlineto(-w,0 );
      c->rlineto(0 ,-h);
      c->stroke();
    }

    icanvas& canvas() const{return *this->c;}

    // plot_ で始まる関数はグラフ座標系
    // draw_ で始まる関数は描画座標系
  public:
    void plot_mark(double x,double y,PlotMarkType type=(PlotMarkType)0) const{
      draw_mark(matX.transform(x),matY.transform(y),type);
    }
    void draw_mark(double x,double y,PlotMarkType type=(PlotMarkType)0) const{
      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);

      switch(type){
      default:
      case PlotMarkType::Cross:
        c->moveto(x+3.0,y+3.0);
        c->lineto(x-3.0,y-3.0);
        c->moveto(x-3.0,y+3.0);
        c->lineto(x+3.0,y-3.0);
        c->stroke();
        break;
      case PlotMarkType::Square:
        c->moveto(x+3.0,y+3.0);
        c->lineto(x-3.0,y+3.0);
        c->lineto(x-3.0,y-3.0);
        c->lineto(x+3.0,y-3.0);
        c->closepath();
        c->stroke();
        break;
      case PlotMarkType::Diamond:
        c->moveto(x,y+4.0);
        c->lineto(x+4.0,y);
        c->lineto(x,y-4.0);
        c->lineto(x-4.0,y);
        c->closepath();
        c->stroke();
        break;
      case PlotMarkType::TriangleU:
        c->moveto(x,y-4.0);
        c->lineto(x+3.46,y+2.0);
        c->lineto(x-3.46,y+2.0);
        c->closepath();
        c->stroke();
        break;
      case PlotMarkType::TriangleD:
        c->moveto(x,y+4.0);
        c->lineto(x+3.46,y-2.0);
        c->lineto(x-3.46,y-2.0);
        c->closepath();
        c->stroke();
        break;
      }
    }
  public:
    void plot_mark(const mwg::stat::errored<>& x,const mwg::stat::errored<>& y) const{
      internal_mark_xeye(
        matX.transform(x.value()),
        matX.dtransform(x.error()),
        matY.transform(y.value()),
        matY.dtransform(y.error())
      );
    }
    void plot_mark(const mwg::range<double>& x,const mwg::stat::errored<>& y) const{
      internal_mark_xrye(
        matX.transform(x.begin()),
        matX.transform(x.end()),
        matY.transform(y.value()),
        matY.dtransform(y.error())
      );
    }
    void plot_mark(const mwg::stat::errored<>& x,const mwg::range<double>& y) const{
      internal_mark_xeyr(
        matX.transform(x.value()),
        matX.dtransform(x.error()),
        matY.transform(y.begin()),
        matY.transform(y.end())
      );
    }
    void draw_mark(const mwg::stat::errored<>& x,const mwg::stat::errored<>& y) const{
      internal_mark_xeye(
        x.value(),
        x.error(),
        y.value(),
        y.error()
      );
    }
    void draw_mark(const mwg::range<double>& x,const mwg::stat::errored<>& y) const{
      internal_mark_xrye(
        x.begin(),
        x.end(),
        y.value(),
        y.error()
      );
    }
    void draw_mark(const mwg::stat::errored<>& x,const mwg::range<double>& y) const{
      internal_mark_xeyr(
        x.value(),
        x.error(),
        y.begin(),
        y.end()
      );
    }
    void plot_mark(double x,const mwg::stat::errored<>& y) const{
      internal_mark_ye(
        matX.transform(x),
        matY.transform(y.value()),
        matY.dtransform(y.error())
      );
    }
    void draw_mark(double x,const mwg::stat::errored<>& y) const{
      internal_mark_ye(x,y.value(),y.error());
    }
  private:
    void internal_mark_xeye(double x0,double dX,double y0,double dY) const{
      double const dx=std::max(dX*0.20,3.0);
      double const dy=std::max(dY*0.20,3.0);

      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);
      internal_path_mark_xerr(x0,y0,dX,dy);
      internal_path_mark_yerr(x0,y0,dY,dx);
      c->stroke();
    }
    void internal_mark_xrye(double x1,double x2,double y0,double dY) const{
      double const x0=(x1+x2)*0.5;
      double const dx=std::max((x2-x1)*0.10,3.0);

      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);
      c->moveto(x1,y0);
      c->lineto(x2,y0);
      internal_path_mark_yerr(x0,y0,dY,dx);
      c->stroke();
    }
    void internal_mark_xeyr(double x0,double dX,double y1,double y2) const{
      double const y0=(y1+y2)*0.5;
      double const dy=std::max((y2-y1)*0.10,3.0);

      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);
      c->moveto(x0,y1);
      c->lineto(x0,y2);
      internal_path_mark_xerr(x0,y0,dX,dy);
      c->stroke();
    }
    void internal_mark_ye(double x0,double y0,double dY) const{
      double const dx=3.0;

      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);
      internal_path_mark_yerr(x0,y0,dY,dx);
      c->stroke();
    }
    void internal_path_mark_xerr(double x0,double y0,double dX,double dy) const{
      c->moveto(x0-dX,y0   );
      c->lineto(x0+dX,y0   );
      c->moveto(x0-dX,y0-dy);
      c->lineto(x0-dX,y0+dy);
      c->moveto(x0+dX,y0-dy);
      c->lineto(x0+dX,y0+dy);
    }
    void internal_path_mark_yerr(double x0,double y0,double dY,double dx) const{
      c->moveto(x0   ,y0-dY);
      c->lineto(x0   ,y0+dY);
      c->moveto(x0-dx,y0-dY);
      c->lineto(x0+dx,y0-dY);
      c->moveto(x0-dx,y0+dY);
      c->lineto(x0+dx,y0+dY);
    }
  public:
    void plot_line(double x1,double y1,double x2,double y2) const{
      mwg::draw::scoped_canvas_gsave _(c);
      c->setlinewidth(1);
      c->moveto(matX.transform(x1),matY.transform(y1));
      c->lineto(matX.transform(x2),matY.transform(y2));
      c->stroke();
    }
  public:
    void plot_fill_xrange(const mwg::range<double>& xrange) const{
      this->internal_fill_xrange(
        matX.transform(xrange.begin()),
        matX.transform(xrange.end())
      );
    }
    void draw_fill_xrange(const mwg::range<double>& xrange) const{
      this->internal_fill_xrange(xrange.begin(),xrange.end());
    }
  public:
    void plot_fill_rect(const mwg::range<double>& xrange,const mwg::range<double>& yrange) const{
      internal_fill_rect(
        matX.transform(xrange.begin()),
        matX.transform(xrange.end()),
        matY.transform(yrange.begin()),
        matY.transform(yrange.end())
      );
    }
    void draw_fill_rect(const mwg::range<double>& xrange,const mwg::range<double>& yrange) const{
      internal_fill_rect(
        xrange.begin(),
        xrange.end(),
        yrange.begin(),
        yrange.end()
      );
    }
  private:
    void internal_fill_xrange(double x1,double x2) const{
      c->moveto(x1,dstRect.top());
      c->lineto(x2,dstRect.top());
      c->lineto(x2,dstRect.bottom());
      c->lineto(x1,dstRect.bottom());
      c->closepath();
      c->fill();
    }
    void internal_fill_rect(double x1,double x2,double y1,double y2) const{
      c->moveto(x1,y1);
      c->lineto(x2,y1);
      c->lineto(x2,y2);
      c->lineto(x1,y2);
      c->closepath();
      c->fill();
    }
  public:
    vector2<double> transform(const vector2<double>& v) const{
      return vector2<double>(matX.transform(v.x),matY.transform(v.y));
    }
    vector2<double> itransform(const vector2<double>& v) const{
      return vector2<double>(matX.itransform(v.x),matY.itransform(v.y));
    }
    const matrix1<double>& xmatrix() const{return this->matX;}
    const matrix1<double>& ymatrix() const{return this->matY;}
  };
#endif

//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif
