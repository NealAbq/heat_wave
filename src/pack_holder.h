// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// pack_holder.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef PACK_HOLDER_H
# define PACK_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "int_holder.h"
# include "animate.h"

// _______________________________________________________________________________________________

class pack_range_steps_holder ;

class double_slide_holder ;
class double_slide_animator_type ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
pack_range_steps_holder
  : public int_range_steps_holder
{
  Q_OBJECT

  // Inherit default_animator_type from supertype.

  public:
    /* ctor */      pack_range_steps_holder
                     (  QObject *  p_parent
                      , double     init_pack    = 0.0
                      , double     min_pack     = -1.0
                      , double     max_pack     = +1.0
                      , bool       is_log_pack  = false
                     )                                      ;

  // Try to hide the integer interface. This is not perfect because it's still easy to get to
  // this interface (the supertype is public), but it'll help.
  protected:
    int_range_steps_holder::  get_value       ;
    int_range_steps_holder::  get_min_value   ;
    int_range_steps_holder::  get_max_value   ;
    int_range_steps_holder::  get_single_step ;
    int_range_steps_holder::  get_page_step   ;
    int_range_steps_holder::  set_values      ;

    // Slots from supertype:
    int_range_steps_holder::  set_range       ;
    int_range_steps_holder::  set_steps       ;
    int_range_steps_holder::  set_min_value   ;
    int_range_steps_holder::  set_max_value   ;
    int_range_steps_holder::  set_single_step ;
    int_range_steps_holder::  set_page_step   ;
    int_range_steps_holder::  set_value       ;

  public:
    bool            is_log_pack( )                    const { return is_log_pack_; }
    bool            is_linear_pack( )                 const { return ! is_log_pack_; }

  public:
    double          get_pack( )                       const { return convert_ui_to_pack( get_value( )); }
    double          get_min_pack( )                   const { return convert_ui_to_pack( get_min_value( )); }
    double          get_max_pack( )                   const { return convert_ui_to_pack( get_max_value( )); }

    // Do not define the following as they are UI-oriented distances:
    //   get_range_pack( )
    //   get_single_step_pack( )
    //   get_page_step_pack( )
    // Distances like that are tricky when the mapping between the outer and inner units is not linear.
    // The steps are part of the UI ctrl anyway and don't belong at this level. This holder should not have
    // to deal with them.

  public:
    void            set_pack(             double d_pack)    { set_value(       convert_pack_to_ui( d_pack)); }
    void            set_min_pack(         double d_pack)    { set_min_value(   convert_pack_to_ui( d_pack)); }
    void            set_max_pack(         double d_pack)    { set_max_value(   convert_pack_to_ui( d_pack)); }

    void            set_range_pack
                     (  double min_pack
                      , double max_pack
                     )                                      { set_range(
                                                                convert_pack_to_ui( min_pack),
                                                                convert_pack_to_ui( max_pack));
                                                            }

  public:
    // We don't use this class in linear mode any more, so the linear functions are not used.
    static int      convert_linear_pack_to_ui( double pack) ;
    static int      convert_linear_pack_to_ui( int pack)    ;
    static double   convert_ui_to_linear_pack( int ui)      ;

    static int      convert_log_pack_to_ui( double pack)    ;
    static double   convert_ui_to_log_pack( int ui)         ;

    static int      convert_pack_to_ui( bool is_log_pack, double pack)
                                                            { return is_log_pack ?
                                                                convert_log_pack_to_ui( pack) :
                                                                convert_linear_pack_to_ui( pack) ;
                                                            }
    static double   convert_ui_to_pack( bool is_log_pack, int ui)
                                                            { return is_log_pack ?
                                                                convert_ui_to_log_pack( ui) :
                                                                convert_ui_to_linear_pack( ui) ;
                                                            }

    int             convert_pack_to_ui( double pack)  const { return convert_pack_to_ui( is_log_pack_, pack); }
    double          convert_ui_to_pack( int ui)       const { return convert_ui_to_pack( is_log_pack_, ui  ); }

  private:
    bool const  is_log_pack_ ; /* otherwise it is linear pack */
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
double_slide_holder
  //
  // This is an improvement on pack_range_steps_holder in linear mode.
  // This was written later.
  // We should get rid of pack_range_steps_holder when we have a more general holder type to
  // replace the exp/log part of that class.
  //
  // Compare this to angle_holder. Angle_holder should become a subtype of this class.
  //
  // This does not inhert from int_range_steps_holder, but instead creates an inner private copy of
  // that object.
  //
  : public holder_base_type
{
  Q_OBJECT
  friend class holder_base_type;

  public:
    typedef double                         value_type            ;
    typedef int_range_steps_holder         inner_holder_type     ;
    typedef inner_holder_type::value_type  inner_value_type      ; /* int */

    typedef double_slide_animator_type     default_animator_type ;

  public:
    /* ctor */  double_slide_holder
                 (  QObject *   p_parent
                  , value_type  init
                  , value_type  min   = -1.0
                  , value_type  max   = +1.0
                 )                                         ;

  public:
    bool        is_valid( )                          const ;
    bool        is_attached( )                       const { return p_inner_holder_->is_attached( ); }
    int         get_attachment_count( )              const { return p_inner_holder_->get_attachment_count( ); }

  public:
    void        attach( QAbstractSlider * p_slider)        { p_inner_holder_->attach( p_slider); }
    void        detach( QAbstractSlider * p_slider)        { p_inner_holder_->detach( p_slider); }

    void        move_values_to( QAbstractSlider * p) const { p_inner_holder_->move_values_to( p); }
    void        set_values_from( QAbstractSlider * p)      { p_inner_holder_->set_values_from( p); }

  public:
    value_type  get_value( )                         const { return convert_inner_to_outer( p_inner_holder_->get_value( )); }
    value_type  get_min_value( )                     const { return convert_inner_to_outer( p_inner_holder_->get_min_value( )); }
    value_type  get_max_value( )                     const { return convert_inner_to_outer( p_inner_holder_->get_max_value( )); }
    value_type  get_range( )                         const { return convert_inner_to_outer_distance( p_inner_holder_->get_range( )); }
    value_type  get_single_step( )                   const { return convert_inner_to_outer_distance( p_inner_holder_->get_single_step( )); }
    value_type  get_page_step( )                     const { return convert_inner_to_outer_distance( p_inner_holder_->get_page_step( )); }

  // Setters - these could be slots
  public:
    void        set_values
                 (  value_type  val
                  , value_type  min
                  , value_type  max
                  , value_type  ss   = -1 /* -1 means use existing value */
                  , value_type  ps   = -1 /* -1 means use existing value */
                 )                                         ;

  public slots:
    void        set_range( double min   , double max )     ;
    void        set_steps( double single, double page)     ;

    void        set_min_value(   double /*value_type*/)    ;
    void        set_max_value(   double)                   ;
    void        set_single_step( double)                   ;
    void        set_page_step(   double)                   ;
    void        set_value(       double)                   ;

  signals:
    void        has_changed(        double /*value_type*/) ;
    void        has_changed__range( double, double)        ;
    void        has_changed__steps( double, double)        ;

  public:
    bool        is_bumping_min( )                          { return p_inner_holder_->is_bumping_min( ); }
    bool        is_bumping_max( )                          { return p_inner_holder_->is_bumping_max( ); }
    void        wrap_to_min( )                             { p_inner_holder_->wrap_to_min( ); }
    void        wrap_to_max( )                             { p_inner_holder_->wrap_to_max( ); }

  protected slots:
    void        intercept__has_changed( )                  ;
    void        intercept__has_changed( int)               ;
    void        intercept__has_changed__range( int, int)   ;
    void        intercept__has_changed__steps( int, int)   ;

  // Conversion
  public:
    inner_value_type  convert_outer_to_inner(          value_type      )  const ;
    inner_value_type  convert_outer_to_inner_distance( value_type      )  const ;
    value_type        convert_inner_to_outer(          inner_value_type)  const ;
    value_type        convert_inner_to_outer_distance( inner_value_type)  const ;
  protected:
    void              setup_conversions( value_type, value_type)                ;

  // Member vars
  private:
    inner_holder_type *  p_inner_holder_ ;

    // Conversion params:
    value_type           outer_lo_       ;  // offset, same as min_value
    value_type           inner_to_outer_ ;
    value_type           outer_to_inner_ ;  // inverse of inner_to_outer_
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
double_slide_animator_type
  : public delta_animator_type
{
  Q_OBJECT

  public:
    typedef double_slide_holder  default_holder_type ;

  public:
    /* ctor */         double_slide_animator_type
                        (  animate_type        *  p_parent
                         , default_holder_type *  p_holder
                         , double                 init_delta_per_sec  = 0.0
                         , double                 min_delta_per_sec   = 0.0
                         , double                 max_delta_per_sec   = 0.0
                        )                                       ;

  // Overridden virtuals, used by supertype
  protected:
    virtual double     get_animating_value( )             const ;
    virtual void       set_animating_value( double)             ;

    virtual bool       is_bumping_min( )                        ;
    virtual bool       is_bumping_max( )                        ;

    virtual void       wrap_to_min( )                           ;
    virtual void       wrap_to_max( )                           ;

    virtual holder_base_type *
                       get_animated( )                    const ;

  // Member var
  private:
    default_holder_type * const  p_holder_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef PACK_HOLDER_H */
//
// pack_holder.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
