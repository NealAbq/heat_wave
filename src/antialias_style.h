// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// antialias_style.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef ANTIALIAS_STYLE_H
# define ANTIALIAS_STYLE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
class QRadioButton;

// _______________________________________________________________________________________________

  class
antialias_style_type
  : public holder_base_type
{
  Q_OBJECT

  public:
      enum
    value_type
     {  e_best
      , e_fastest
      , e_none
     };

  public:
    /* ctor */  antialias_style_type
                 (  QObject *   p_parent
                  , value_type  init_value  = e_none
                 )                                            ;

  public:
    void        attach
                 (  QRadioButton *  p_radio_best
                  , QRadioButton *  p_radio_fastest
                  , QRadioButton *  p_radio_none
                  , bool            init_value_from_holder  = true
                 )                                            ;

  public:
    void        set_gl
                 (  bool  is_points  = true
                  , bool  is_lines   = true
                 )                                      const ;

  public:
    value_type  get_value( )                            const { return value_; }

    bool        is_best( )                              const { return get_value( ) == e_best; }
    bool        is_fastest( )                           const { return get_value( ) == e_fastest; }
    bool        is_none( )                              const { return get_value( ) == e_none; }

  public:
    void        set_value( value_type)                        ; /* emits has_changed( ) */

    void        set_best( )                                   { set_value( e_best); }
    void        set_fastest( )                                { set_value( e_fastest); }
    void        set_none( )                                   { set_value( e_none); }

  // Attach the slots to radio buttons.
  public slots:
    void        set_best( bool y)                             { if ( y ) { set_best( ); } }
    void        set_fastest( bool y)                          { if ( y ) { set_fastest( ); } }
    void        set_none( bool y)                             { if ( y ) { set_none( ); } }

  signals:
    // Signals to update radio buttons. Connected in attach(..).
    void        has_changed__best(    bool)                   ;
    void        has_changed__fastest( bool)                   ;
    void        has_changed__none(    bool)                   ;

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef ANTIALIAS_STYLE_H
//
// antialias_style.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
