// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shading_style.h
//
//   Copyright (c) Neal Binnendyk 2009, 2010.
//     <nealabq@gmail.com>
//     <http://nealabq.com/>
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
# ifndef SHADING_STYLE_H
# define SHADING_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
class QRadioButton;

// _______________________________________________________________________________________________

  class
shading_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
      enum
    value_type
     {  e_block
      , e_simple
      , e_strip
      , e_area
     };

  public:
    /* ctor */  shading_style_type
                 (  QObject *   p_parent
                  , value_type  init_value  = e_area
                 )                                          ;

  public:
    void        attach
                 (  QRadioButton *  p_radio_block
                  , QRadioButton *  p_radio_simple
                  , QRadioButton *  p_radio_strip
                  , QRadioButton *  p_radio_area
                  , bool            init_value_from_holder  = true
                 )                                          ;

  public:
    value_type  get_value( )                          const { return value_; }

    bool        is_block( )                           const { return get_value( ) == e_block; }
    bool        is_simple( )                          const { return get_value( ) == e_simple; }
    bool        is_strip( )                           const { return get_value( ) == e_strip; }
    bool        is_area( )                            const { return get_value( ) == e_area; }

  public:
    void        set_value( value_type)                      ; /* emits has_changed( ) */

    void        set_block( )                                { set_value( e_block); }
    void        set_simple( )                               { set_value( e_simple); }
    void        set_strip( )                                { set_value( e_strip); }
    void        set_area( )                                 { set_value( e_area); }

  // Attach the slots to radio buttons.
  public slots:
    void        set_block(  bool y)                         { if ( y ) { set_block( ); } }
    void        set_simple( bool y)                         { if ( y ) { set_simple( ); } }
    void        set_strip(  bool y)                         { if ( y ) { set_strip( ); } }
    void        set_area(   bool y)                         { if ( y ) { set_area( ); } }

  signals:
    // Signals to update radio buttons. Connected in attach(..).
    void        has_changed__block(  bool)                  ;
    void        has_changed__simple( bool)                  ;
    void        has_changed__strip(  bool)                  ;
    void        has_changed__area(   bool)                  ;

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef SHADING_STYLE_H
//
// shading_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
