// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_properties_style.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef FACE_PROPERTIES_STYLE_H
# define FACE_PROPERTIES_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "face_style.h"
# include "shading_style.h"
# include "antialias_style.h"
# include "bool_holder.h"
# include "int_holder.h"

// _______________________________________________________________________________________________

  class
face_properties_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
    /* ctor */  face_properties_style_type( QObject * p_parent)  ;

  public:
    face_style_type        *  get_top_face_style( )        { return p_top_face_style_   ; }
    face_style_type        *  get_under_face_style( )      { return p_under_face_style_ ; }

    int_range_steps_holder *  get_top_face_shininess( )    { return p_top_shininess_    ; }
    int_range_steps_holder *  get_under_face_shininess( )  { return p_under_shininess_  ; }

    shading_style_type     *  get_shading_style( )         { return p_shading_style_    ; }
    bool_holder            *  get_is_triangle_mesh( )      { return p_is_triangle_mesh_ ; }
    bool_holder            *  get_is_interpolate( )        { return p_is_interpolate_   ; }

    int_range_steps_holder *  get_pixel_width( )           { return p_pixel_width_      ; }
    antialias_style_type   *  get_antialias_style( )       { return p_antialias_style_  ; }

    bool                      is_smooth_shade( )     const ;

  private:
    face_style_type        *  p_top_face_style_   ;
    face_style_type        *  p_under_face_style_ ;

    int_range_steps_holder *  p_top_shininess_    ;
    int_range_steps_holder *  p_under_shininess_  ;

    shading_style_type     *  p_shading_style_    ;
    bool_holder            *  p_is_triangle_mesh_ ;
    bool_holder            *  p_is_interpolate_   ;

    int_range_steps_holder *  p_pixel_width_      ;
    antialias_style_type   *  p_antialias_style_  ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef FACE_PROPERTIES_STYLE_H
//
// face_properties_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
