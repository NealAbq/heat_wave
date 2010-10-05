// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// moving_sum.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef MOVING_SUM_H
# define MOVING_SUM_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include <boost/circular_buffer.hpp>
# include "all.h"
# include "debug.h"
# include "date_time.h"

// _______________________________________________________________________________________________

  template< typename VAL_T >
  class
moving_sum
  //
  // Provide statistics? Std dev, etc?
  //
  // Improve:
  //   We should probably provide separate versions of this class for int and real (float, double) values.
  //   The sum tends to drift when the numbers are real (especially if they're float) so for reals we should
  //   re-calc the sum once in a while.
  //
  // Other template params:
  //   Accumulate functor. Default std::plus< VAL_T >. Except this also needs an inverse function,
  //   to subtract the old values from the sum.
  //
  //   Sum type. Default VAL_T.
{
  public:
    typedef VAL_T       value_type;
    typedef value_type  value_param_type;

    typedef size_t      size_type;
    typedef size_type   size_param_type;

    typedef value_type  sum_type;
    typedef sum_type    sum_param_type;

  public:
    /* ctor */          moving_sum( tag::do_not_init_type, size_param_type capacity = 10)
                                                            : sum_( ), values_( capacity)
                                                            { }
    /* ctor */          moving_sum( size_param_type capacity = 10)
                                                            : sum_( 0), values_( capacity)
                                                            { }
    /* dtor */          ~moving_sum( )                      { }

  public:
    // Record a single value.
    void                record_next( value_param_type new_value)
                                                            { if ( is_full( ) ) {
                                                                sum_ -= get_oldest_value( );
                                                              }
                                                              sum_ += new_value;
                                                              values_.push_front( new_value);
                                                            }

    // Record a repeated value.
    void                record_next_n( size_type count, value_param_type new_value)
                                                            { if ( count >= get_capacity( ) ) {
                                                                set_full( new_value);
                                                              } else
                                                              while ( count > 0 ) {
                                                                record_next( new_value);
                                                                -- count;
                                                            } }

    value_param_type    get_oldest_value( )           const { return values_.back( ); }
    value_param_type    get_newest_value( )           const { return values_.front( ); }

    size_param_type     get_count( )                  const { return values_.size( ); }
    sum_param_type      get_sum( )                    const { return sum_; }

    size_param_type     get_capacity( )               const { return values_.capacity( ); }
    void                set_capacity( size_param_type new_capacity)
                                                            { while ( new_capacity < get_count( ) ) {
                                                                sum_ -= get_oldest_value( );
                                                                values_.pop_back( );
                                                              }
                                                              d_assert( get_count( ) <= new_capacity);
                                                              values_.set_capacity( new_capacity);
                                                              d_assert( get_capacity( ) == new_capacity);
                                                            }

    bool                is_full( )                    const { return values_.full( ); }
    bool                is_empty( )                   const { return values_.empty( ); }

    template< typename AVE_T >
    AVE_T               get_average( )                const { return is_empty( ) ? 0 : (
                                                                static_cast< AVE_T >( get_sum(   )) /
                                                                static_cast< AVE_T >( get_count( )) );
                                                            }
    void                set_empty( )                        { sum_ = 0;
                                                            # ifndef NDEBUG
                                                              size_type const old_capacity = get_capacity( );
                                                            # endif
                                                              values_.resize( 0);
                                                              d_assert( get_capacity( ) == old_capacity);
                                                              d_assert( is_empty( ));
                                                            }

    void                set_full( value_param_type v)       { size_type count = get_capacity( );
                                                              sum_ = v * count;
                                                              while ( count > 0 ) {
                                                                values_.push_front( v);
                                                                -- count;
                                                              }
                                                              d_assert( is_full( ));
                                                            }

  private:
    value_type                           sum_    ;
    boost::circular_buffer< value_type > values_ ;
};

// _______________________________________________________________________________________________
// Specialization for date_time::tick_duration_type

  template< >
  class
moving_sum< date_time::tick_duration_type >
{
  public:
    typedef date_time::tick_duration_type  value_type;
    typedef value_type const &             value_param_type;

    typedef size_t                         size_type;
    typedef size_type                      size_param_type;

    typedef value_type                     sum_type;
    typedef sum_type const &               sum_param_type;

  public:
    /* ctor */          moving_sum( size_param_type capacity = 10)
                                                            : sum_( ), values_( capacity)
                                                            { }
    /* dtor */          ~moving_sum( )                      { }

  public:
    // Record a single value.
    void                record_next( value_param_type new_value)
                                                            { if ( is_full( ) ) {
                                                                sum_ -= get_oldest_value( );
                                                              }
                                                              sum_ += new_value;
                                                              values_.push_front( new_value);
                                                            }

    // Record a repeated value.
    void                record_next_n( size_type count, value_param_type new_value)
                                                            { if ( count >= get_capacity( ) ) {
                                                                set_full( new_value);
                                                              } else
                                                              while ( count > 0 ) {
                                                                record_next( new_value);
                                                                -- count;
                                                            } }

    value_param_type    get_oldest_value( )           const { return values_.back( ); }
    value_param_type    get_newest_value( )           const { return values_.front( ); }

    size_param_type     get_count( )                  const { return values_.size( ); }
    sum_param_type      get_sum( )                    const { return sum_; }

    size_param_type     get_capacity( )               const { return values_.capacity( ); }
    void                set_capacity( size_param_type new_capacity)
                                                            { while ( new_capacity < get_count( ) ) {
                                                                sum_ -= get_oldest_value( );
                                                                values_.pop_back( );
                                                              }
                                                              d_assert( get_count( ) <= new_capacity);
                                                              values_.set_capacity( new_capacity);
                                                              d_assert( get_capacity( ) == new_capacity);
                                                            }

    bool                is_full( )                    const { return values_.full( ); }
    bool                is_empty( )                   const { return values_.empty( ); }

    value_type          get_average__ticks( )         const { return is_empty( ) ?
                                                                value_type( 0, 0, 0) :
                                                                (get_sum( ) / get_count( ));
                                                            }
    date_time::second_type
                        get_average__seconds( )       const { return date_time::convert_ticks_to_seconds( get_average__ticks( )); }

    void                set_empty( )                        { sum_ = value_type( 0, 0, 0);
                                                            # ifndef NDEBUG
                                                              size_type const old_capacity = get_capacity( );
                                                            # endif
                                                              values_.resize( 0);
                                                              d_assert( get_capacity( ) == old_capacity);
                                                              d_assert( is_empty( ));
                                                            }

    void                set_full( value_param_type v)       { size_type count = get_capacity( );
                                                              sum_ = v * count;
                                                              while ( count > 0 ) {
                                                                values_.push_front( v);
                                                                -- count;
                                                              }
                                                              d_assert( is_full( ));
                                                            }

  private:
    value_type                           sum_    ;
    boost::circular_buffer< value_type > values_ ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef MOVING_SUM_H */
//
// moving_sum.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
