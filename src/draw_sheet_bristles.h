// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_bristle.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef DRAW_SHEET_BRISTLE_H
# define DRAW_SHEET_BRISTLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "draw_sheet_base.h"
# include "bristle_properties_style.h"

// _______________________________________________________________________________________________

  class
draw_sheet_bristles_type
  : public draw_sheet::base_type
{
  // ---------------------------------------------------------------------------------------
  public:
    typedef draw_sheet::window_iter_type  window_iter_type ;
    typedef draw_sheet::color_type        color_type       ;
    typedef draw_sheet::coord_type        coord_type       ;
    typedef draw_sheet::coord_scale_type  coord_scale_type ;
    typedef draw_sheet::pt_type           pt_type          ;

  // ---------------------------------------------------------------------------------------
  // Ctor/dtor
  public:
    /* ctor */    draw_sheet_bristles_type
                   (  bristle_properties_style_type *  bristle_props
                    , coord_scale_type                 model_xy_scale
                    , coord_scale_type                 model_z_scale
                   )                                            ;

  // ---------------------------------------------------------------------------------------
  // Main drawing
  public:
    void          draw( sheet_type const &)                     ;
  protected:
    bool          is_drawable( )                                ;
    void          setup_gl( )                                   ;

    virtual void  continue_strip( window_iter_type const &)     ; /* overridden */

    void          draw_triangle_center_bristles( )              ;
    void          draw_quad_center_bristle( )                   ;
    void          draw_quad_quad_bristles( )                    ;
    void          draw_simple_corner_bristles( )                ;
    void          draw_strip_corner_bristles( )                 ;
    void          draw_area_corner_bristle( )                   ;
    void          draw_top_and_under_bristles
                   (  pt_type const &  normal_vector__any_len
                    , pt_type const &  middle_pt
                   )                                            ;
    pt_type       calc_vector__unit_length__scaled_coords
                   (  pt_type const &  normal__random_length__local_coords
                   )                                            ;

  // ---------------------------------------------------------------------------------------
  // Member var
  private:
    window_iter_type const *  p_window_iter_     ;

    bristle_position_type::value_type
                     const    position_          ;
    color_type       const    top_color_         ;
    color_type       const    under_color_       ;
    int              const    width_             ;
    bool             const    is_lines_          ;
    float            const    hair_top_length_   ;
    float            const    hair_under_length_ ;

    antialias_style_type &    antialias_style_   ;

    // The following should be a separate elliptical-transform class.
    coord_scale_type const    model_xy_scale_    ;
    coord_scale_type const    model_z_scale_     ;
    coord_scale_type const    one_over_mxy_sqrd_ ; /* 1 / (model_xy_scale * model_xy_scale) */
    coord_scale_type const    one_over_mz_sqrd_  ;

} /* end class draw_sheet_bristles_type */ ;

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef DRAW_SHEET_BRISTLE_H
//
// draw_sheet_bristle.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
