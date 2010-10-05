// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_holder.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef COLOR_HOLDER_H
# define COLOR_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"
# include "holder.h"
# include "int_holder.h"
# include "gl_env_type_colors.h"

# include <QtCore/QString>
# include <QtGui/QColor>
class QAbstractButton;

// _______________________________________________________________________________________________

  class
color_holder
  : public holder_base_type
{
  Q_OBJECT

  // ---------------------------------------------------------
  public:
    typedef gl_env::rgba_type< gl_env::float_type >  color_type      ;
    typedef QList< QWidget * >                       list_type       ;
    typedef int_range_steps_holder                   int_holder_type ;

  // ---------------------------------------------------------
  public:
    /* ctor */          color_holder
                         (  QObject          *  p_parent
                          , color_type const &  init_color
                          , QString    const &  dlg_title   = tr( "Choose color")
                         )                                    ;
    /* ctor */          color_holder
                         (  QObject          *  p_parent
                          , QColor     const &  init_qcolor
                          , QString    const &  dlg_title   = tr( "Choose color")
                         )                                    ;

  private: /* disable copy */
    /* copy */          color_holder( color_holder const *)   ; // no implementaton

  // ---------------------------------------------------------
  public:
    color_type const &  get_value( )                    const { return get_color( ); }
    void                set_value( color_type const & c)      { set_color( c); }

    color_type const &  get_color( )                    const { d_assert( is_color_valid( ));
                                                                return color_;
                                                              }
    QColor     const &  get_qcolor( )                   const { d_assert( is_qcolor_valid( ));
                                                                return qcolor_;
                                                              }

    void                set_color( color_type)                ;
    void                set_qcolor( QColor)                   ;

  // ---------------------------------------------------------
  public slots:
    void                choose_color_dlg( )                   ;

  // ---------------------------------------------------------
  public:
    void                attach( QAbstractButton *)            ;
    void                detach( QAbstractButton *)            ;
  protected slots:
    void                set_button_color( )                   ;
    void                before_dtor( QObject *)               ;

  // ---------------------------------------------------------
  public:
    int                 get_r_0_255( )                  const ;
    int                 get_g_0_255( )                  const ;
    int                 get_b_0_255( )                  const ;

    int                 get_h_0_359( )                  const ;
    int                 get_s_0_255( )                  const ;
    int                 get_v_0_255( )                  const ;

  public slots:
    void                set_r_0_255( int)                     ;
    void                set_g_0_255( int)                     ;
    void                set_b_0_255( int)                     ;

    void                set_h_0_359( int)                     ;
    void                set_s_0_255( int)                     ;
    void                set_v_0_255( int)                     ;

  signals:
    void                has_changed__r_0_255( int)            ;
    void                has_changed__g_0_255( int)            ;
    void                has_changed__b_0_255( int)            ;

    void                has_changed__h_0_359( int)            ;
    void                has_changed__s_0_255( int)            ;
    void                has_changed__v_0_255( int)            ;

  public:
    int_holder_type *   get_holder__r( )                      ;
    int_holder_type *   get_holder__g( )                      ;
    int_holder_type *   get_holder__b( )                      ;

    int_holder_type *   get_holder__h( )                      ;
    int_holder_type *   get_holder__s( )                      ;
    int_holder_type *   get_holder__v( )                      ;

  protected:
    static bool         is_color_valid( color_type const &)   ;
    static void         ensure_color_is_valid( color_type &)  ;
    bool                is_color_valid( )               const ;
    void                ensure_color_is_valid( )              ;

    static bool         is_qcolor_valid( QColor const &)      ;
    static void         ensure_qcolor_is_valid( QColor &)     ;
    bool                is_qcolor_valid( )              const ;
    void                ensure_qcolor_is_valid( )             ;

    void                maybe_save_last_good_hue( int const)  ;
    void                move_qcolor_to_color( )               ;
    void                move_color_to_qcolor( )               ;

  // ---------------------------------------------------------
  // Member vars
  private:
    color_type         color_         ;  /* rgba floats */
    QColor             qcolor_        ;  /* spec == QColor::Hsv */
    int                last_good_hue_ ;  /* 0 - 359 */

    QString const      dlg_title_     ;
    list_type          buttons_       ;

    int_holder_type *  p_holder_r_    ;  /* 0-255 */
    int_holder_type *  p_holder_g_    ;  /* 0-255 */
    int_holder_type *  p_holder_b_    ;  /* 0-255 */

    int_holder_type *  p_holder_h_    ;  /* -1, 0-359, we translate -1 to zero */
    int_holder_type *  p_holder_s_    ;  /* 0-255 */
    int_holder_type *  p_holder_v_    ;  /* 0-255 */
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef COLOR_HOLDER_H */
//
// color_holder.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
