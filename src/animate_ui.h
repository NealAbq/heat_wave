// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate_ui.h
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
# ifndef ANIMATE_UI_H
# define ANIMATE_UI_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "ui_heat_simd.h"
# include "holder.h"
# include "animate.h"

class color_holder;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
animate_ui_type
  : public holder_base_type
{
  Q_OBJECT

  public:
    typedef delta_animator_type        animator_type  ;

    typedef QListWidget                list_type      ;
    typedef QListWidgetItem            item_base_type ;

    typedef QLabel                     label_type     ;
    typedef QAbstractButton            button_type    ;
    typedef QAbstractSlider            slider_type    ;

    typedef Ui::heat_wave_main_window  scafold_type   ;

  // Public interface
  public:
    /* ctor */  animate_ui_type
                 (  QObject          *  p_parent
                  , scafold_type     *  p_ui
                  , animate_type     *  p_animate  = 0
                  , heat_widget_type *  p_hw       = 0
                 )                                                      ;
    /* ctor */  animate_ui_type
                 (  QObject     *  p_parent
                  , list_type   *  p_list
                  , label_type  *  p_label
                  , button_type *  p_button__is_on
                  , button_type *  p_button__is_wrapping
                  , button_type *  p_button__is_reversed
                  , slider_type *  p_slider__rate
                  , slider_type *  p_slider__value
                  , slider_type *  p_slider__trigger
                 )                                                      ;

    void        build_list_of_animators
                 (  animate_type     *
                  , heat_widget_type *
                  , scafold_type     *
                 )                                                      ;

  // Everything after this is protected or private.
  protected slots:
    void        on_list__current_item_changed
                 (  QListWidgetItem *
                  , QListWidgetItem *
                 )                                                      ;
    void        on_item__activated( QListWidgetItem *)                  ;
    void        on_animator__on_off( bool)                              ;

  protected:
    void        attach_list( )                                          ;

    template< typename HOLDER_T >
    animator_type *
                create_animator
                 (  QString const &  name
                  , HOLDER_T      *  p_holder
                  , double           init_value = 0
                 )                                                      ;

    animator_type *
                create_click_animator( QString const &, button_type *)  ;

    void        create_holder_animators( heat_widget_type *)            ;
    void        create_click_animators( scafold_type *)                 ;

    void        add_list_item( QString const &, animator_type *)        ;
    void        create_color_animators
                 (  QString const &  prefix
                  , color_holder  *
                  , bool             include_hue
                  , bool             include_sv
                  , bool             include_rgb
                  , bool_holder   *  p_auto_on_0
                  , bool_holder   *  p_auto_on_1
                 )                                                      ;
    void        create_hue_animator
                 (  QString const &  prefix
                  , color_holder  *  p_holder
                  , bool_holder   *  p_auto_on_0  = 0
                  , bool_holder   *  p_auto_on_1  = 0
                 )                                                      ;
    void        create_hsv_animators
                 (  QString const &  prefix
                  , color_holder  *  p_holder
                  , bool_holder   *  p_auto_on_0  = 0
                  , bool_holder   *  p_auto_on_1  = 0
                 )                                                      ;
    void        create_rgb_animators
                 (  QString const &  prefix
                  , color_holder  *  p_holder
                  , bool_holder   *  p_auto_on_0  = 0
                  , bool_holder   *  p_auto_on_1  = 0
                 )                                                      ;

    void        attach_animator( animator_type *)                       ;
    void        detach_animator( animator_type *)                       ;

    void        set_background_colors__list( )                          ;
    void        set_background_color__list_item( item_base_type *)      ;
    void        set_text_and_color__label( )                            ;
    void        set_text__is_reversed_button__is_reversed( )            ;
    void        set_text__is_reversed_button__is_target_on( )           ;

  private:
    list_type     * const  p_list_                ;
    label_type    * const  p_label_               ;
    button_type   * const  p_button__is_on_       ;
    button_type   * const  p_button__is_wrapping_ ;
    button_type   * const  p_button__is_reversed_ ;
    slider_type   * const  p_slider__rate_        ;
    slider_type   * const  p_slider__value_       ;
    slider_type   * const  p_slider__trigger_     ;

    animate_type  *        p_animate_             ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef ANIMATE_UI_H */
//
// animate_ui.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
