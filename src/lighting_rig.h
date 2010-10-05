// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// lighting_rig.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef LIGHTING_RIG_H
# define LIGHTING_RIG_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"
# include "bool_holder.h"
# include "angle_holder.h"
# include "color_holder.h"
# include "animate.h"
# include "gl_draw_lights.h"

# include <QtCore/QObject>
class QAbstractButton;
class QAbstractSlider;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
light_type
  : public QObject
{
  Q_OBJECT

  // The type of the parent object.
  // The parent object creates and attaches this object.
  friend class lighting_rig_type;

  public:
    typedef gl_env::lights::light_type               gl_type    ;
    typedef gl_env::rgba_type< gl_env::float_type >  color_type ;

  protected:
    /* ctor */  light_type
                 (  QObject *           p_parent
                  , gl_type *           p_gl_light
                  , bool                init_is_on
                  , color_type const &  init_ambient_color
                  , color_type const &  init_diffuse_color
                  , color_type const &  init_specular_color
                  , int                 init_spin_angle
                  , int                 init_rise_angle
                 )                                              ;

  public:
    void        attach_on_off_button( QAbstractButton *)        ;
    void        attach_arrow_display_button( QAbstractButton *) ;
    void        attach_ambient_color_ctrls
                 (  QAbstractButton *  is_on_check_button
                  , QAbstractButton *  color_button
                 )                                              ;
    void        attach_diffuse_color_ctrls
                 (  QAbstractButton *  is_on_check_button
                  , QAbstractButton *  color_button
                 )                                              ;
    void        attach_specular_color_ctrls
                 (  QAbstractButton *  is_on_check_button
                  , QAbstractButton *  color_button
                 )                                              ;
    void        attach_spin_angle_ctrls
                 (  QAbstractButton *  is_eye_move_check_button
                  , QAbstractSlider *  slider
                 )                                              ;
    void        attach_rise_angle_ctrls
                 (  QAbstractButton *  is_eye_move_check_button
                  , QAbstractSlider *  slider
                 )                                              ;

  signals:
    void        has_changed( )                                  ;

  private slots:
    void        has_changed__on_off( )                          ;
    void        has_changed__arrow_display( )                   ;
    void        has_changed__ambient_color( )                   ;
    void        has_changed__diffuse_color( )                   ;
    void        has_changed__specular_color( )                  ;
    void        has_changed__spin_angle( )                      ;
    void        has_changed__rise_angle( )                      ;

  private:
    void        move_from_holder_to_gl__on_off( )               ;
    void        move_from_holder_to_gl__arrow_display( )        ;
    void        move_from_holder_to_gl__ambient_color( )        ;
    void        move_from_holder_to_gl__diffuse_color( )        ;
    void        move_from_holder_to_gl__specular_color( )       ;
    void        move_from_holder_to_gl__spin_angle( )           ;
    void        move_from_holder_to_gl__rise_angle( )           ;

  // Expose the holders so we can create animators for them.
  public:
    bool_holder  *  get_is_on( )                                { return d_verify_not_zero( p_is_on_); }
    bool_holder  *  get_is_displaying_arrow( )                  { return d_verify_not_zero( p_is_displaying_arrow_); }

    bool_holder  *  get_is_ambient_on( )                        { return d_verify_not_zero( p_is_ambient_on_); }
    bool_holder  *  get_is_diffuse_on( )                        { return d_verify_not_zero( p_is_diffuse_on_); }
    bool_holder  *  get_is_specular_on( )                       { return d_verify_not_zero( p_is_specular_on_); }

    color_holder *  get_ambient_color( )                        { return d_verify_not_zero( p_ambient_color_); }
    color_holder *  get_diffuse_color( )                        { return d_verify_not_zero( p_diffuse_color_); }
    color_holder *  get_specular_color( )                       { return d_verify_not_zero( p_specular_color_); }

    angle_holder *  get_spin_angle( )                           { return d_verify_not_zero( p_spin_angle_); }
    angle_holder *  get_rise_angle( )                           { return d_verify_not_zero( p_rise_angle_); }

    bool_holder  *  get_is_move_with_eye__spin( )               { return d_verify_not_zero( p_is_move_with_eye__spin_); }
    bool_holder  *  get_is_move_with_eye__rise( )               { return d_verify_not_zero( p_is_move_with_eye__rise_); }

  // Private member vars
  private:
    gl_type      * const  p_gl_light_               ;

    bool_holder  * const  p_is_on_                  ;
    bool_holder  * const  p_is_displaying_arrow_    ;

    bool_holder  * const  p_is_ambient_on_          ;
    bool_holder  * const  p_is_diffuse_on_          ;
    bool_holder  * const  p_is_specular_on_         ;

    color_holder * const  p_ambient_color_          ;
    color_holder * const  p_diffuse_color_          ;
    color_holder * const  p_specular_color_         ;

    angle_holder * const  p_spin_angle_             ;
    bool_holder  * const  p_is_move_with_eye__spin_ ;

    angle_holder * const  p_rise_angle_             ;
    bool_holder  * const  p_is_move_with_eye__rise_ ;
};

// _______________________________________________________________________________________________

  class
lighting_rig_type
  : public QObject
{
  Q_OBJECT

  public:
    typedef gl_env::lights::lighting_type            gl_type    ;
    typedef gl_env::rgba_type< gl_env::float_type >  color_type ;

  public:
    /* ctor */    lighting_rig_type( QObject * p_parent)    ;

  public:
    void          attach_ambient_color_ctrls__overall
                   (  QAbstractButton *  is_on_check_button
                    , QAbstractButton *  color_button
                   )                                        ;

    void          attach_to_gl( )                           { gl_lighting_.attach_to_gl( ); }
    void          detach_from_gl( )                         { gl_lighting_.detach_from_gl( ); }

    void          copy_to_current_gl_context( )       const { gl_lighting_.copy_to_current_gl_context( ); }

    void          set_on( )                                 { gl_lighting_.set_on( ); }
    void          set_off( )                                { gl_lighting_.set_off( ); }

    void          place_in_eye_space
                   (  double  model_rise     // degrees, -90.0 .. +90.0
                    , double  model_spin     // degrees, -180.0 .. +180.0
                    , float   model_distance // zero for ortho projection, otherwise positive
                   )                                        { gl_lighting_.place_in_eye_space(
                                                                model_rise, model_spin, model_distance);
                                                            }
  signals:
    void          has_changed( )                            ; /* funnel */
  private slots:
    void          has_changed__ambient_color( )             ;
  private:
    void          move_from_holder_to_gl__ambient_color( )  ;

  // Expose the holders (and the holders in the individual lights) so we can make animators for them.
  public:
    light_type   *  get_light_0( )                          { return & light_0_; }
    light_type   *  get_light_1( )                          { return & light_1_; }

    bool_holder  *  get_is_ambient_on( )                    { return d_verify_not_zero( p_is_ambient_on_); }
    color_holder *  get_ambient_color( )                    { return d_verify_not_zero( p_ambient_color_); }

  // Private member vars
  private:
    gl_type               gl_lighting_     ;
    light_type            light_0_         ;
    light_type            light_1_         ;
    bool_holder  * const  p_is_ambient_on_ ;
    color_holder * const  p_ambient_color_ ;
};

// _______________________________________________________________________________________________

  struct
with_shader_lighting
{
  with_shader_lighting( ) ;
  ~with_shader_lighting( ) ;

  private:
    bool const  was_disabled__overall_ ;
    bool const  was_disabled__0_       ;
    bool const  was_disabled__1_       ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef LIGHTING_RIG_H
//
// lighting_rig.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
