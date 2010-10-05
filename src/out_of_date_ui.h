// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// out_of_date_ui.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
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
