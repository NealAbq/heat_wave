// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_properties_style.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef BRISTLE_PROPERTIES_STYLE_H
# define BRISTLE_PROPERTIES_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "bristle_style.h"
# include "bristle_position.h"
# include "int_holder.h"
# include "color_holder.h"
# include "antialias_style.h"

// _______________________________________________________________________________________________

  class
bristle_properties_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
    /* ctor */  bristle_properties_style_type( QObject * p_parent)
                                                    ;

  public:
    bristle_style_type     *  get_style( )            { return p_style_           ; }
    bristle_position_type  *  get_position( )         { return p_position_        ; }
    int_range_steps_holder *  get_top_length( )       { return p_top_length_      ; }
    int_range_steps_holder *  get_under_length( )     { return p_under_length_    ; }
    int_range_steps_holder *  get_pixel_width( )      { return p_pixel_width_     ; }
    antialias_style_type   *  get_antialias_style( )  { return p_antialias_style_ ; }
    color_holder           *  get_top_color( )        { return p_top_color_       ; }
    color_holder           *  get_under_color( )      { return p_under_color_     ; }

  private:
    bristle_style_type     *  p_style_            ;
    bristle_position_type  *  p_position_         ;
    int_range_steps_holder *  p_top_length_       ;
    int_range_steps_holder *  p_under_length_     ;
    int_range_steps_holder *  p_pixel_width_      ;
    antialias_style_type   *  p_antialias_style_  ;
    color_holder           *  p_top_color_        ;
    color_holder           *  p_under_color_      ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef BRISTLE_PROPERTIES_STYLE_H
//
// bristle_properties_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
