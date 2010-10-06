// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_properties_style.h
//
//   Copyright (c) Neal Binnendyk 2009, 2010. <nealabq@gmail.com> nealabq.com
//
//   |=== GPL License Notice ====================================================================|
//   | This code is free software: you can redistribute it and/or modify it under the terms      |
//   | of the GNU General Public License as published by the Free Software Foundation, either    |
//   | version 3 of the License, or (at your option) any later version.                          |
//   |                                                                                           |
//   | This code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;    |
//   | without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. |
//   | See the GNU General Public License for more details: <http://www.gnu.org/licenses/>       |
//   |=== END License Notice ====================================================================|
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
