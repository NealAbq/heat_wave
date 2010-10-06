// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date_ui.h
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
# ifndef OUT_OF_DATE_UI_H
# define OUT_OF_DATE_UI_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "out_of_date.h"
# include <QtCore/QObject>

class QRadioButton;
class QSpinBox;
class QDoubleSpinBox;

// _______________________________________________________________________________________________

  class
out_of_date_ui_type
  : public QObject
  //
  // Could be templated on value type.
{
  Q_OBJECT

  // -------------------------------------------------------------------------------------------
  // Types and type names
  public:
    typedef out_of_date_type::tick_point_type     tick_point_type    ;
    typedef out_of_date_type::tick_duration_type  tick_duration_type ;
    typedef out_of_date_type::gen_type            gen_type           ;

    typedef out_of_date_type::e_option            e_option           ;

  // -------------------------------------------------------------------------------------------
  // Consructor
  public:
    /* ctor */                out_of_date_ui_type( )                          ;
  private:
    Q_DISABLE_COPY( out_of_date_ui_type);

  // -------------------------------------------------------------------------------------------
  // Delay object
  public:
    out_of_date_type       &  ref_delay( )                                    { return delay_; }
    out_of_date_type const &  get_delay( )                              const { return delay_; }

  // -------------------------------------------------------------------------------------------
  // UI functions
  public:
    void                      set_ui_from_delay( )                            ;

    void                      set_delay_from_ui( )                            ;
    void                      set_delay_option_from_ui( )                     ;
    void                      set_delay_tick_max_from_ui( )                   ;
    void                      set_delay_gen_max_from_ui( )                    ;

    e_option                  get_option_from_ui( )                     const ;

  // -------------------------------------------------------------------------------------------
  // Setters
  public:
    void                      set_button_always(             QRadioButton *)  ;
    void                      set_button_never(              QRadioButton *)  ;
    void                      set_button_watch_tick(         QRadioButton *)  ;
    void                      set_button_watch_gen(          QRadioButton *)  ;
    void                      set_button_watch_tick_and_gen( QRadioButton *)  ;
    void                      set_button_watch_tick_or_gen(  QRadioButton *)  ;

    void                      set_spinbox_seconds( QDoubleSpinBox *)          ;
    void                      set_spinbox_gen( QSpinBox *)                    ;

  public slots:
    void                      ui_changed_option( bool)                        ;
    void                      ui_changed_tick_max( double)                    ;
    void                      ui_changed_gen_max( int)                        ;

  // -------------------------------------------------------------------------------------------
  // Getters
  public:
    QRadioButton   *          get_button_always( )                      const { return p_button_always_; }
    QRadioButton   *          get_button_never( )                       const { return p_button_never_; }
    QRadioButton   *          get_button_watch_tick( )                  const { return p_button_watch_tick_; }
    QRadioButton   *          get_button_watch_gen( )                   const { return p_button_watch_gen_; }
    QRadioButton   *          get_button_watch_tick_and_gen( )          const { return p_button_watch_tick_and_gen_; }
    QRadioButton   *          get_button_watch_tick_or_gen( )           const { return p_button_watch_tick_or_gen_; }

    QDoubleSpinBox *          get_spinbox_seconds( )                    const { return p_spinbox_seconds_; }
    QSpinBox       *          get_spinbox_gen( )                        const { return p_spinbox_gen_; }

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    out_of_date_type  delay_                       ;

    QRadioButton   *  p_button_always_             ;
    QRadioButton   *  p_button_never_              ;
    QRadioButton   *  p_button_watch_tick_         ;
    QRadioButton   *  p_button_watch_gen_          ;
    QRadioButton   *  p_button_watch_tick_and_gen_ ;
    QRadioButton   *  p_button_watch_tick_or_gen_  ;

    // These could also be labels or text edits.
    QDoubleSpinBox *  p_spinbox_seconds_           ;
    QSpinBox       *  p_spinbox_gen_               ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef OUT_OF_DATE_UI_H */
//
// out_of_date_ui.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
