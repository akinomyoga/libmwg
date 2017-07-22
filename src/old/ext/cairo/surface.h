// -*- mode: c++; coding: utf-8 -*-
#ifndef MWG_EXT_CAIRO_SURFACE_H
#define MWG_EXT_CAIRO_SURFACE_H
#include <cstdio>
#include <cairo/cairo.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-ft.h>

#include <mwg/defs.h>
#include <mwg/draw/defs.h>
#include <mwg/std/memory>
#include <ft2build.h>

#include FT_FREETYPE_H
namespace mwg{
namespace ext{
namespace cairo{
//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN

  class Font:public mwg::draw::ifont{
    mwg::stdm::shared_ptr<cairo_font_face_t> pface;
    cairo_user_data_key_t key;
  public:
    mwg::draw::FontInfo info;
    Font(){}
    bool load_from_file(const char* ttfname){
      FT_Face ft_face;
      FT_Library library;
      if(FT_Init_FreeType(&library)){
        std::fprintf(stderr,"mwg::ext::cairo: failed to initialize freetype.\n");
        return false;
      }

      if(FT_New_Face(library,ttfname,0,&ft_face)){
        std::fprintf(stderr,"mwg::ext::cairo: failed to load ttf file.\n");
        return false;
      }

      if(FT_Set_Pixel_Sizes(ft_face,20,20)){
        std::fprintf(stderr,"mwg::ext::cairo: failed to set pixel sizes.\n");
        return false;
      }

      cairo_font_face_t* font_face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
      int status = cairo_font_face_set_user_data(font_face, &key, ft_face, (cairo_destroy_func_t) FT_Done_Face);
      if(status){
        cairo_font_face_destroy (font_face);
        FT_Done_Face (ft_face);
        std::fprintf(stderr,"mwg::ext::cairo: failed to bind FT_Face as an user data.\n");
        return false;
      }

      typedef mwg::stdm::shared_ptr<cairo_font_face_t> return_type;
      this->pface.reset(font_face,&cairo_font_face_destroy);
      return true;
    }
    cairo_font_face_t* get_fontface() const{
      return this->pface.get();
    }
  };

//-----------------------------------------------------------------------------

  class CairoCanvasBase:public mwg::draw::icanvas{
    typedef mwg::draw::icanvas base;
  protected:
    cairo_surface_t* sf;
    cairo_t* cr;
    int width;
    int height;
    CairoCanvasBase(cairo_surface_t* sf)
      :sf(sf),cr(nullptr)
      ,width(-1),height(-1)
    {}
    virtual ~CairoCanvasBase(){
      if(this->cr!=nullptr){
        cairo_show_page(this->cr);
        cairo_destroy(this->cr);
        this->cr=nullptr;
      }
      cairo_surface_destroy(this->sf);
      this->sf=nullptr;
    }
  public:
    cairo_surface_t* get_surface() const{
      return this->sf;
    }
    cairo_t* get_cairo() const{
      return this->cr;
    }
    int get_width() const{
      return this->width;
    }
    int get_height() const{
      return this->height;
    }
  public:
    void gsave(){
      cairo_save(cr);
    }
    void grestore(){
      cairo_restore(cr);
    }
    void internal_moveto(double x,double y){ // override
      cairo_move_to(this->cr,x,y);
    }
    void internal_rmoveto(double dx,double dy){ // override
      cairo_rel_move_to(cr,dx,dy);
    }
    void internal_lineto(double x,double y){ // override
      cairo_line_to(this->cr,x,y);
    }
    void internal_rlineto(double dx,double dy){ // override
      cairo_rel_line_to(cr,dx,dy);
    }
    void stroke(){ // override
      cairo_stroke(cr);
    }
    void fill(){
      cairo_fill(cr);
    }
    void closepath(){
      cairo_close_path(cr);
    }
    void internal_setrgbcolor(double r,double g,double b,double a){
      cairo_set_source_rgba(cr,r,g,b,a);
    }
    void setlinewidth(double w){
      cairo_set_line_width(cr,w);
    }
    void translate(double dx,double dy){
      cairo_translate(cr,dx,dy);
    }
    void internal_rotate(double deg){
      cairo_rotate(cr,-deg*(M_PI/180));
    }
    void internal_arc(double x,double y,double r,double a1,double a2){
      cairo_arc_negative(cr,x,y,r,-a1*(M_PI/180),-a2*(M_PI/180));
    }
    void internal_newpath(){
      cairo_new_path(cr);
    }
    void internal_show(const char* text){
      cairo_show_text(this->cr,text);
    }
    void internal_stringwidth(const char* text,double& w,double& h){
      cairo_text_extents_t ret;
      cairo_text_extents(cr,text,&ret);
      w=ret.width;
      h=ret.height;
    }
  public:
    mwg::draw::ifont* create_font(const mwg::draw::FontInfo& info){
      if(info.name_type==mwg::draw::FontNameType::FilePath){
        mwg::ext::cairo::Font* ret=new mwg::ext::cairo::Font;
        ret->info=info;
        if(!ret->load_from_file(info.name.c_str())){
          delete ret;
          return nullptr;
        }
        return ret;
      }else{
        return new mwg::draw::FontInfo(info);
      }
    }
    bool set_font(mwg::draw::ifont* font){
      mwg::draw::FontInfo* f1=dynamic_cast<mwg::draw::FontInfo*>(font);
      if(f1!=nullptr){
        cairo_select_font_face(
          this->cr,
          f1->name.c_str(),
          (f1->style&mwg::draw::FontStyle::Italic)?CAIRO_FONT_SLANT_ITALIC:
          (f1->style&mwg::draw::FontStyle::Slanted)?CAIRO_FONT_SLANT_OBLIQUE:
          CAIRO_FONT_SLANT_NORMAL,
          (f1->style&mwg::draw::FontStyle::Bold)?CAIRO_FONT_WEIGHT_BOLD:
          CAIRO_FONT_WEIGHT_NORMAL
        );
        cairo_set_font_size(this->cr,f1->size);
        return true;
      }

      mwg::ext::cairo::Font* f2=dynamic_cast<mwg::ext::cairo::Font*>(font);
      if(f2!=nullptr){
        cairo_set_font_face(this->cr,f2->get_fontface());
        cairo_set_font_size(this->cr,f2->info.size);
        return true;
      }

      return false;
    }
  };

  class EpsCanvas:public CairoCanvasBase{
    std::string fpath;
  public:
    EpsCanvas(int width,int height,const char* fpath)
      :CairoCanvasBase(cairo_ps_surface_create(fpath,width,height))
    {
      this->fpath=fpath;
      this->width=width;
      this->height=height;
      cairo_ps_surface_set_eps(this->sf,true);
      this->cr=cairo_create(this->sf);
    }
  };

  class PngCanvas:public CairoCanvasBase{
    std::string fpath;
    typedef CairoCanvasBase base;
  public:
    PngCanvas(int width,int height,const char* fpath)
      :CairoCanvasBase(cairo_image_surface_create(CAIRO_FORMAT_ARGB32,width,height))
    {
      this->fpath=fpath;
      this->width=width;
      this->height=height;
      this->cr=cairo_create(this->sf);
      cairo_translate(cr,0.5,0.5);
      //cairo_set_antialias(this->cr,CAIRO_ANTIALIAS_NONE);
    }
    ~PngCanvas(){
      if(this->cr!=nullptr){
        cairo_show_page(this->cr);
        cairo_destroy(this->cr);
        this->cr=nullptr;
        cairo_surface_write_to_png(this->sf,fpath.c_str());
      }
    }
  public:
    void internal_moveto(double x,double y){ // override
      base::internal_moveto(int(x),int(y));
    }
    void internal_rmoveto(double dx,double dy){ // override
      base::internal_rmoveto(int(dx),int(dy));
    }
    void internal_lineto(double x,double y){ // override
      base::internal_lineto(int(x),int(y));
    }
    void internal_rlineto(double dx,double dy){ // override
      base::internal_rlineto(int(dx),int(dy));
    }
    void translate(double dx,double dy){
      base::translate(int(dx),int(dy));
    }
  };


//NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
}
}
}
#endif
