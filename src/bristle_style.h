// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bristle_style.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef BRISTLE_STYLE_H
# define BRISTLE_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
class QRadioButton;

// _______________________________________________________________________________________________

  class
bristle_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
      enum
    value_type
     {  e_lines
      , e_dots
      , e_none
     };

  public:
    /* ctor */  bristle_style_type
                 (  QObject *   p_parent
                  , value_type  init_value  = e_none
                 )                                      ;

  public:
    void        attach
                 (  QRadioButton *  p_radio_lines
                  , QRadioButton *  p_radio_dots
                  , QRadioButton *  p_radio_none
                  , bool            init_value_from_holder  = true
                 )                                      ;

  public:
    value_type  get_value( )                      const { return value_; }

    bool        is_lines( )                       const { return get_value( ) == e_lines; }
    bool        is_dots( )                        const { return get_value( ) == e_dots; }
    bool        is_none( )                        const { return get_value( ) == e_none; }

  public:
    void        set_value( value_type)                  ; /* emits has_changed( ) */

    void        set_lines( )                            { set_value( e_lines); }
    void        set_dots( )                             { set_value( e_dots); }
    void        set_none( )                             { set_value( e_none); }

  // Attach the slots to radio buttons.
  public slots:
    void        set_lines( bool y)                      { if ( y ) { set_lines( ); } }
    void        set_dots( bool y)                       { if ( y ) { set_dots( ); } }
    void        set_none( bool y)                       { if ( y ) { set_none( ); } }

  signals:
    // Signals to update radio buttons. Connected in attach(..).
    void        has_changed__lines( bool)               ;
    void        has_changed__dots( bool)                ;
    void        has_changed__none( bool)                ;

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef BRISTLE_STYLE_H
//
// bristle_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
