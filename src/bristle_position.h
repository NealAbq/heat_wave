// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_position.h
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
# ifndef BRISTLE_POSITION_H
# define BRISTLE_POSITION_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
class QRadioButton;

// _______________________________________________________________________________________________

  class
bristle_position_type
  : public holder_base_type
{
  Q_OBJECT

  public:
      enum
    value_type
     {  e_triangle_center
      , e_quad_center
      , e_quad_quads
      , e_simple_corners
      , e_strip_corners
      , e_area_corners
      , e_area_penta
     };

  public:
    /* ctor */  bristle_position_type
                 (  QObject *   p_parent
                  , value_type  init_value  = e_quad_center
                 )                                          ;

  public:
    void        attach
                 (  QRadioButton *  p_radio_triangle_center
                  , QRadioButton *  p_radio_quad_center
                  , QRadioButton *  p_radio_quad_quads
                  , QRadioButton *  p_radio_simple_corners
                  , QRadioButton *  p_radio_strip_corners
                  , QRadioButton *  p_radio_area_corners
                  , QRadioButton *  p_radio_area_penta
                  , bool            init_value_from_holder  = true
                 )                                          ;

  public:
    value_type  get_value( )                          const { return value_; }

    bool        is_triangle_center( )                 const { return get_value( ) == e_triangle_center; }
    bool        is_quad_center(     )                 const { return get_value( ) == e_quad_center    ; }
    bool        is_quad_quads(      )                 const { return get_value( ) == e_quad_quads     ; }
    bool        is_simple_corners(  )                 const { return get_value( ) == e_simple_corners ; }
    bool        is_strip_corners(   )                 const { return get_value( ) == e_strip_corners  ; }
    bool        is_area_corners(    )                 const { return get_value( ) == e_area_corners   ; }
    bool        is_area_penta(      )                 const { return get_value( ) == e_area_penta     ; }

  public:
    void        set_value( value_type)                      ; /* emits has_changed( ) */

    void        set_triangle_center( )                      { set_value( e_triangle_center); }
    void        set_quad_center(     )                      { set_value( e_quad_center    ); }
    void        set_quad_quads(      )                      { set_value( e_quad_quads     ); }
    void        set_simple_corners(  )                      { set_value( e_simple_corners ); }
    void        set_strip_corners(   )                      { set_value( e_strip_corners  ); }
    void        set_area_corners(    )                      { set_value( e_area_corners   ); }
    void        set_area_penta(      )                      { set_value( e_area_penta     ); }

  // Attach the slots to radio buttons.
  public slots:
    void        set_triangle_center( bool y)                { if ( y ) { set_triangle_center( ); } }
    void        set_quad_center(     bool y)                { if ( y ) { set_quad_center(     ); } }
    void        set_quad_quads(      bool y)                { if ( y ) { set_quad_quads(      ); } }
    void        set_simple_corners(  bool y)                { if ( y ) { set_simple_corners(  ); } }
    void        set_strip_corners(   bool y)                { if ( y ) { set_strip_corners(   ); } }
    void        set_area_corners(    bool y)                { if ( y ) { set_area_corners(    ); } }
    void        set_area_penta(      bool y)                { if ( y ) { set_area_penta(      ); } }

  signals:
    // Signals to update radio buttons. Connected in attach(..).
    void        has_changed__triangle_center( bool)         ;
    void        has_changed__quad_center(     bool)         ;
    void        has_changed__quad_quads(      bool)         ;
    void        has_changed__simple_corners(  bool)         ;
    void        has_changed__strip_corners(   bool)         ;
    void        has_changed__area_corners(    bool)         ;
    void        has_changed__area_penta(      bool)         ;

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef BRISTLE_POSITION_H
//
// bristle_position.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
