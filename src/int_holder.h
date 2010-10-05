// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// int_holder.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef INT_HOLDER_H
# define INT_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "holder.h"
# include "animate.h"
class QSpinBox;
class QAbstractSlider;

// _______________________________________________________________________________________________

class int_range_steps_holder;
class int_animator_type;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
int_range_steps_holder
  : public holder_base_type
  //
  // This is an aggregate holder.
{
  Q_OBJECT
  friend class holder_base_type;

  public:
    typedef int                 value_type ;
    typedef QList< QWidget * >  list_type  ;

    typedef int_animator_type   default_animator_type ;

  public:
    /* ctor */  int_range_steps_holder
                 (  QObject *                 p_parent
                  , int_range_steps_holder *  p_copy
                 )                                        ;

    /* ctor */  int_range_steps_holder
                 (  QObject *     p_parent
                  , int           init_value  = 0
                 )                                        ;

    /* ctor */  int_range_steps_holder
                 (  QObject *     p_parent
                  , int           init_value
                  , int           min_value
                  , int           max_value
                  , int           single_step  = 0
                  , int           page_step    = 0
                 )                                        ;

  public:
    bool        is_valid( )                         const ;
    bool        is_attached( )                      const ;
    int         get_attachment_count( )             const ;

  public:
    void        attach( QSpinBox *)                       ;
    void        attach( QAbstractSlider *)                ;

    void        detach( QSpinBox *)                       ;
    void        detach( QAbstractSlider *)                ;

    void        move_values_to( QSpinBox *       )  const ;
    void        move_values_to( QAbstractSlider *)  const ;

    void        set_values_from( QSpinBox        *)       ;
    void        set_values_from( QAbstractSlider *)       ;

  public:
    value_type  get_value( )                        const { return value_      ; }
    value_type  get_min_value( )                    const { return min_value_  ; }
    value_type  get_max_value( )                    const { return max_value_  ; }
    value_type  get_range( )                        const { return get_max_value( ) - get_min_value( ); }
    value_type  get_single_step( )                  const { return single_step_; }
    value_type  get_page_step( )                    const { return page_step_  ; }

  // Setters - these could be slots
  public:
    void        set_values
                 (  int_range_steps_holder const *  p_copy
                 )                                        ;
    void        set_values
                 (  value_type  val
                  , value_type  min
                  , value_type  max
                  , value_type  ss   = -1 /* -1 means use existing value */
                  , value_type  ps   = -1 /* -1 means use existing value */
                 )                                        ;

  public slots:
    void        set_range( int min   , int max )          ;
    void        set_steps( int single, int page)          ;

    void        set_min_value(   int)                     ;
    void        set_max_value(   int)                     ;
    void        set_single_step( int)                     ;
    void        set_page_step(   int)                     ;
    void        set_value(       int)                     ;

  protected slots:
    void        before_dtor( QObject *)                   ;

  // This should have collective signaling, based on tuples or some kind of bag object:
  //   get_values( )
  //   set_values( bag const & values)  - slot
  //   has_changed( bag const &)        - signal

  signals:
    void        has_changed(        int /*value_type*/)   ;
    void        has_changed__range( int, int)             ;
    void        has_changed__steps( int, int)             ;

  // Animation helpers
  public:
    bool       is_bumping_min( )                          { return get_value( ) == get_min_value( ); }
    bool       is_bumping_max( )                          { return get_value( ) == get_max_value( ); }

    void       wrap_to_min( )                             { set_value( get_min_value( )); }
    void       wrap_to_max( )                             { set_value( get_max_value( )); }

  // Member vars
  protected:
    list_type   attached_widgets_ ;
    value_type  value_            ;
    value_type  min_value_        ; // lo or range_lo
    value_type  max_value_        ; // hi or range_hi
    value_type  single_step_      ; // step_lo or small step
    value_type  page_step_        ; // step_hi or big step

    // All these values could be type <int_holder *>.
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
int_animator_type
  : public delta_animator_type
{
  Q_OBJECT

  public:
    typedef int_range_steps_holder  default_holder_type ;

  public:
    /* ctor */         int_animator_type
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
# endif /* ifndef INT_HOLDER_H */
//
// int_holder.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
