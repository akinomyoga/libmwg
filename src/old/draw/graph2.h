// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_DRAW_PLOT_GRAPH2_H
#define MWG_DRAW_PLOT_GRAPH2_H
// #include <cmath>
// #include <mwg/stat/errored_value.h>
// #include <mwg/range.h>
// #include <mwg/exp/utils.h>
// #include <mwg/exp/attr.h>
#include "defs.h"
namespace mwg{
namespace draw1{
namespace plot{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  class IGraphSeries{
  public:
    std::string series_name;
    mwg::u4t series_color;
    IGraphSeries():color(0xFFFFFFFFu){}
    virtual ~IGraphSeries(){}
  };

  template<typename T>
  class IGraphSeries1:public IGraphSeries{
  public:
    IGraphSeries1(){}
  public:
    virtual int length() const=0;
    virtual T get_value(int index) const=0;
  };

  template<typename Tx,typename Ty>
  class IGraphSeries2:public IGraphSeries{
  public:
    IGraphSeries(){}
  public:
    virtual int length() const=0;
    virtual void get_value(int index,Tx& x,Ty& y) const=0;
  };

//-----------------------------------------------------------------------------

  static mwg::u4t SeriesColors[8]={
    0xFF0000,
    0x008000,
    0x0000FF,
    0x808000,
    0x008080,
    0x800080,
    0x800000,
    0x000080,
  };


//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif
