// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_surface.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef DRAW_SHEET_SURFACE_H
# define DRAW_SHEET_SURFACE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "draw_sheet_base.h"
# include "color_gradient_holder.h"
# include "shading_style.h"

// _______________________________________________________________________________________________

  class
draw_sheet_surface_type
  : public draw_sheet::base_type
{
  // ---------------------------------------------------------------------------------------
  public:
    typedef draw_sheet::window_iter_type  window_iter_type ;
    typedef draw_sheet::color_type        color_type       ;
    typedef draw_sheet::coord_type        coord_type       ;
    typedef draw_sheet::pt_type           pt_type          ;

  // ---------------------------------------------------------------------------------------
  // Ctor/dtor
  public:
    /* ctor */    draw_sheet_surface_type
                   (  color_gradient_holder const *   p_grad
                    , bool                            is_drawing_triangles
                    , shading_style_type::value_type  shading_style
                   )                                            ;

  // ---------------------------------------------------------------------------------------
  // Main drawing
  public:
    void          draw( sheet_type const &)                     ;

  // ---------------------------------------------------------------------------------------
  // Drawing helpers
  protected:
    virtual void  continue_strip( window_iter_type const &)     ; /* overridden pure virtual */

  // ---------------------------------------------------------------------------------------
  protected:
    void          continue_strip__strip_average(   )      const ;
    void          continue_strip__area_average(    )      const ;
    void          continue_strip__triangle_block(  )      const ;
    void          continue_strip__triangle_simple( )      const ;
    void          continue_strip__quad_block(      )      const ;
    void          continue_strip__quad_simple(     )      const ;

    void          drop_color( coord_type z_draw)          const ;
    static void   drop_vertex_no_color( pt_type const &)        ;
    void          drop_vertex(           pt_type const &) const ;
    static void   drop_normal_any_length( pt_type const &)      ;

    bool          is_drawing_triangles( )                 const { return is_drawing_triangles_; }

    bool          is_shading_block( )                     const { return shading_style_ == shading_style_type::e_block; }
    bool          is_shading_simple( )                    const { return shading_style_ == shading_style_type::e_simple; }
    bool          is_shading_strip_average( )             const { return shading_style_ == shading_style_type::e_strip; }
    bool          is_shading_area_average( )              const { return shading_style_ == shading_style_type::e_area; }

  // ---------------------------------------------------------------------------------------
  // Member vars
  private:
    window_iter_type               const *  p_window_iter_        ;
    color_gradient_holder          const *  p_color_gradient_     ;
    bool                           const    is_drawing_triangles_ ;  /* either triangle or quad strip */
    shading_style_type::value_type const    shading_style_        ;

} /* end class draw_sheet_surface_type */ ;

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef DRAW_SHEET_SURFACE_H
//
// draw_sheet_surface.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
