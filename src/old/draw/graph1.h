// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_DRAW_PLOT_GRAPH1_H
#define MWG_DRAW_PLOT_GRAPH1_H
#include <cmath>
#include <mwg/stat/errored_value.h>
#include <mwg/range.h>
#include <mwg/exp/utils.h>
#include <mwg/exp/attr.h>
namespace mwg{
namespace draw1{
namespace plot{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  class AxisRangeOption;
  class AxisRange;
  class AxisScaler;

  mwg_attr_define_flags(AxisRangeOption,mwg::dword,enum{
    Default          =0,
    Mask_Lower       =0x0F,
    LowerFixed       =0x01,
    LowerInclude     =0x02,
    LowerBound       =0x03,
    Mask_Upper       =0xF0,
    UpperFixed       =0x10,
    UpperInclude     =0x20,
    UpperBound       =0x30,
  });

  class AxisRange{
    double vmin;
    double vmax;
    double emin;
    double emax;
  public:
    AxisRange(){
      emin=vmin=+HUGE_VAL;
      emax=vmax=-HUGE_VAL;
    }
    void register_value(double value){
      if(value<vmin){
        vmin=value;
        if(value<emin)
          emin=value;
      }
      if(value>vmax){
        vmax=value;
        if(value>emax)
          emax=value;
      }
    }
    void register_value(const mwg::stat::errored<>& value){
      double v=value.value();
      double eu=value.upper1sigma();
      double el=value.lower1sigma();

      if(v<vmin)vmin=v;
      if(v>vmax)vmax=v;
      if(el<emin)emin=el;
      if(eu>emax)emax=eu;
    }
    mwg::range<double> get_range(double margin=0.1){
      double len=emax-emin;
      double l=emin-margin*len;
      double u=emax+margin*len;
      return mwg::range<double>(l,u);
    }

    mwg::range<double> get_range(AxisRangeOption option,double lower=0,double upper=0,double margin=0.1){
      double l=emin;
      bool marginL=true;
      switch(option&AxisRangeOption::Mask_Lower){
      case AxisRangeOption::LowerFixed:
        l=lower;
        marginL=false;
        break;
      case AxisRangeOption::LowerInclude:
        if(vmin>lower){
          l=lower;
          marginL=false;
        }
        break;
      case AxisRangeOption::LowerBound:
        if(l<lower){
          l=lower;
          marginL=false;
        }
        break;
      }

      double u=emax;
      bool marginU=true;
      switch(option&AxisRangeOption::Mask_Upper){
      case AxisRangeOption::UpperFixed:
        u=upper;
        marginU=false;
        break;
      case AxisRangeOption::UpperInclude:
        if(vmax<upper){
          u=upper;
          marginU=false;
        }
        break;
      case AxisRangeOption::UpperBound:
        if(u>upper){
          u=upper;
          marginU=false;
        }
        break;
      }

      margin*=u-l;
      if(marginL)l-=margin;
      if(marginU)u+=margin;
      return mwg::range<double>(l,u);
    }
  };

  class AxisScaler{
    mwg::range<double> r;
    mwg::range<double> dr;
    double dratio;

    double scale0;
    int scale1interval;
  public:
    AxisScaler(const mwg::range<double>& r,const mwg::range<double>& dr)
      :r(r),dr(dr),dratio(dr.length()/r.length()){}
  public:
    void set_scale(double scale0,int scale1interval){
      this->scale0=scale0;
      this->scale1interval=scale1interval;
    }
    void set_scale_auto(double displayScale0,double displayScale1){
      this->scale0=determine_scale_min(displayScale0/dratio);
      double scale1=determine_scale_min(displayScale1/dratio);
      if(int(scale1/scale0*2+1e-5)&1)scale0/=2;
      this->scale1interval=int(scale1/scale0+1e-5);
    }
    static double determine_scale_min(double minValue){
      double value=std::pow(10,std::ceil(std::log10(minValue)));
      if(value/minValue>5)
        value/=5; // 0.2 step
      else if(value/minValue>2)
        value/=2; // 0.5 step
      return value;
    }
  public:
    struct enumerator:mwg::exp::ienumerator<double,enumerator>{
      const AxisScaler& parent;
      int i;
      int iN;
      enumerator(const AxisScaler& parent)
        :parent(parent)
      {
        i=std::ceil(parent.r.begin()/parent.scale0);
        iN=std::floor(parent.r.end()/parent.scale0)+1;
      }
      operator bool() const{return i<iN;}
      void next(){i++;}
      double operator*() const{return parent.scale0*i;}
      bool is_scale1() const{return i%parent.scale1interval==0;}
      double display_position() const{
        double value=parent.scale0*i;
        return parent.dr.begin()+parent.dratio*(value-parent.r.begin());
      }
    };
    enumerator scale_begin() const{
      return enumerator(*this);
    }
  };
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif

