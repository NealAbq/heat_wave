// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// angle_holder.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef ANGLE_HOLDER_H
# define ANGLE_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "int_holder.h"
# include "animate.h"

// _______________________________________________________________________________________________

class angle_holder        ;
class angle_arc_type      ;
class angle_animator_type ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
angle_arc_type
{
  public:
    enum value_type /* : char  -- this is c++0x */
     {  e_360  // range is [-180, +180)
      , e_180  // range is [-90, +90)
     };

  public:
    /* ctor */     angle_arc_type( value_type init = e_360)
                                             : value_( init)
                                             { }

  public:
    value_type     get_value( )        const { return value_; }
    bool           is_360( )           const { return get_value( ) == e_360; }
    bool           is_180( )           const { return get_value( ) == e_180; }

    int            get_min_degrees( )  const { return is_360( ) ? -180 : -90; }
    int            get_max_degrees( )  const { return - get_min_degrees( ); }

  private:
    value_type  value_ ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
angle_holder
  //
  // The supertype should be double_slide_holder instead of int_range_steps_holder.
  //
  : public int_range_steps_holder
{
  Q_OBJECT

  public:
    typedef angle_animator_type  default_animator_type ;

  public:
    /* ctor */     angle_holder
                    (  QObject *       p_parent
                     , double          init_degrees  = 0
                     , angle_arc_type  arc           = angle_arc_type::e_360
                    )                                         ;
    /* ctor */     angle_holder
                    (  QObject *  p_parent
                     , double     init_degrees
                     , double     min_degrees
                     , double     max_degrees
                    )                                         ;

  public:
    bool           is_full_circle( )                    const ;

    int            get_arc_angle__arcseconds( )         const ;
    double         get_arc_angle__degrees( )            const { return arcseconds_to_degrees( get_arc_angle__arcseconds( )); }

    int            get_min_angle__arcseconds( )         const { return get_min_value( ); }
    int            get_max_angle__arcseconds( )         const { return get_max_value( ); }

    int            get_angle__arcseconds( )             const { return get_value( ); }
    double         get_angle__degrees( )                const { return arcseconds_to_degrees( get_angle__arcseconds( )); }
    double         get_angle__radians( )                const { return degrees_to_radians( get_angle__degrees( )); }

    void           set_angle__arcseconds( int angle_arcsecs)  { set_value( angle_arcsecs); }
    void           set_angle__degrees( double angle_degrees)  { set_angle__arcseconds( degrees_to_arcseconds( angle_degrees)); }
    void           set_angle__radians( double angle_radians)  { set_angle__degrees( radians_to_degrees( angle_radians)); }

  public:
    static double  degrees_to_radians( double angle_degrees)  ;
    static double  radians_to_degrees( double angle_radians)  ;

    static double  arcseconds_to_degrees( int i_arcseconds)   ;
    static int     degrees_to_arcseconds( int i_degrees)      ;
    static int     degrees_to_arcseconds( double d_degrees)   ;

  public:
    static double const  pi           ;
    static double const  pi_over_2    ;
    static double const  pi_over_180  ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
angle_animator_type
  //
  // The supertype should probably be double_slide_animator_type instead of delta_animator_type.
  // And in that case it does not need to be a supertype at all.
  //
  : public delta_animator_type
{
  Q_OBJECT

  public:
    typedef angle_holder  default_holder_type ;

  public:
    /* ctor */         angle_animator_type
                        (  animate_type *  p_parent
                         , angle_holder *  p_holder
                         , double          init_degrees_per_second  = 1
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
    angle_holder * const  p_holder_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef ANGLE_HOLDER_H */
//
// angle_holder.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
