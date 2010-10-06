// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_style.h
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
# ifndef FACE_STYLE_H
# define FACE_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
class QRadioButton;

// _______________________________________________________________________________________________

  class
face_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
      enum
    value_type
     {  e_filled
      , e_lined
      , e_dotted
      , e_invisible
     };

  public:
    /* ctor */  face_style_type
                 (  QObject *   p_parent
                  , value_type  init_value  = e_filled
                 )                                          ;

  public:
    void        attach
                 (  QRadioButton *  p_radio_filled
                  , QRadioButton *  p_radio_lined
                  , QRadioButton *  p_radio_dotted
                  , QRadioButton *  p_radio_invisible
                  , bool            init_value_from_holder  = true
                 )                                          ;

  public:
    value_type  get_value( )                          const { return value_; }

    bool        is_filled( )                          const { return get_value( ) == e_filled; }
    bool        is_lined( )                           const { return get_value( ) == e_lined; }
    bool        is_dotted( )                          const { return get_value( ) == e_dotted; }
    bool        is_invisible( )                       const { return get_value( ) == e_invisible; }

  public:
    void        set_value( value_type)                      ; /* emits has_changed( ) */

    void        set_filled( )                               { set_value( e_filled); }
    void        set_lined( )                                { set_value( e_lined); }
    void        set_dotted( )                               { set_value( e_dotted); }
    void        set_invisible( )                            { set_value( e_invisible); }

  // Attach the slots to radio buttons.
  public slots:
    void        set_filled( bool y)                         { if ( y ) { set_filled( ); } }
    void        set_lined( bool y)                          { if ( y ) { set_lined( ); } }
    void        set_dotted( bool y)                         { if ( y ) { set_dotted( ); } }
    void        set_invisible( bool y)                      { if ( y ) { set_invisible( ); } }

  signals:
    // Signals to update radio buttons. Connected in attach(..).
    void        has_changed__filled( bool)                  ;
    void        has_changed__lined( bool)                   ;
    void        has_changed__dotted( bool)                  ;
    void        has_changed__invisible( bool)               ;

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef FACE_STYLE_H
//
// face_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
