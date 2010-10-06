// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// finite_diff_solver.h
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
# ifndef FINITE_DIFF_SOLVER_H
# define FINITE_DIFF_SOLVER_H
// _______________________________________________________________________________________________
//
// Improvements:
//   Add an early-exit mechanism to interrupt the solve.
//   This can probably be just a pointer to a bool, or maybe a global bool value? Or a function?
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "pair_iter.h"
# include "stride_iter.h"
# include "finite_diff.h"

// The following is a precaution. If we #include <WinDef.h> somewhere before here, then the macros
// min and max will also be defined (unless we #define NOMINMAX ahead of time). This breaks
// std::min and std::max defined in <algorithm>.
# ifdef min
#   undef min
#   undef max

    // Make sure min/max doesn't get #defined again.
#   define NOMINMAX 1

    // Windows also defines _cpp_min() and _cpp_max() as (non-portable) "replacements" for
    // std::min() and std::max().
# endif

# include <iterator>
# include <algorithm>
# include <QtCore/QtConcurrentMap>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Solving functors

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
solving_functor_typenames_type
{
    // -- Typedefs -----------------------------------------------------
    typedef RATE_TYPE                             rate_type            ;

    typedef SRC_ITER_TYPE                         src_iter_type        ;
    typedef TRG_ITER_TYPE                         trg_iter_type        ;

    typedef stride_range< src_iter_type, 1 >      src_range_1_type     ;
    typedef stride_range< src_iter_type, 0 >      src_range_0_type     ;
    typedef stride_range< trg_iter_type, 1 >      trg_range_1_type     ;
    typedef stride_range< trg_iter_type, 0 >      trg_range_0_type     ;

    typedef stride_iter< src_iter_type , 1 >      src_iter_1_type      ;
    typedef stride_iter< src_iter_type , 0 >      src_iter_0_type      ;
    typedef stride_iter< trg_iter_type , 1 >      trg_iter_1_type      ;
    typedef stride_iter< trg_iter_type , 0 >      trg_iter_0_type      ;

    typedef typename trg_iter_0_type::diff_type   diff_type            ;
    typedef typename trg_iter_0_type::size_type   size_type            ;
    typedef typename trg_iter_0_type::val_type    val_type             ;

    typedef pair_iter
             <  src_iter_1_type
              , trg_iter_1_type
             >                                    src_trg_iter_1_type  ;
    typedef typename
            src_trg_iter_1_type::counter_type     src_trg_count_1_type ;
};

# define INHERIT_FUNCTOR_TYPENAMES( S_TYPE)                                \
    typedef typename S_TYPE::rate_type             rate_type            ;  \
    typedef typename S_TYPE::diff_type             diff_type            ;  \
    typedef typename S_TYPE::size_type             size_type            ;  \
    typedef typename S_TYPE::val_type              val_type             ;  \
    typedef typename S_TYPE::src_iter_0_type       src_iter_0_type      ;  \
    typedef typename S_TYPE::src_iter_1_type       src_iter_1_type      ;  \
    typedef typename S_TYPE::trg_iter_0_type       trg_iter_0_type      ;  \
    typedef typename S_TYPE::trg_iter_1_type       trg_iter_1_type      ;  \
    typedef typename S_TYPE::src_range_0_type      src_range_0_type     ;  \
    typedef typename S_TYPE::src_range_1_type      src_range_1_type     ;  \
    typedef typename S_TYPE::trg_range_0_type      trg_range_0_type     ;  \
    typedef typename S_TYPE::trg_range_1_type      trg_range_1_type     ;  \
    typedef typename S_TYPE::src_iter_type         src_iter_type        ;  \
    typedef typename S_TYPE::trg_iter_type         trg_iter_type        ;  \
    typedef typename S_TYPE::src_trg_count_1_type  src_trg_count_1_type ;  \
    typedef typename S_TYPE::src_trg_iter_1_type   src_trg_iter_1_type     /* end macro */

// _______________________________________________________________________________________________

  template<  typename BUF_ITER_TYPE >
  struct
solving_functor_typenames_buf_type
{
    // -- Typedefs -------------------------------------------------
    typedef BUF_ITER_TYPE                     buf_iter_type        ;

    typedef stride_range< buf_iter_type, 0 >  buf_range_0_type     ;
    typedef stride_iter<  buf_iter_type, 0 >  buf_iter_0_type      ;

    typedef pair_iter
             <  buf_iter_0_type
              , buf_iter_0_type
             >                                buf_buf_iter_0_type  ;
};

# define INHERIT_BUFFER_TYPENAMES( S_TYPE)                                  \
    typedef typename S_TYPE::buf_iter_type          buf_iter_type        ;  \
    typedef typename S_TYPE::buf_iter_0_type        buf_iter_0_type      ;  \
    typedef typename S_TYPE::buf_range_0_type       buf_range_0_type     ;  \
    typedef typename S_TYPE::buf_buf_iter_0_type    buf_buf_iter_0_type     /* end macro */

// _______________________________________________________________________________________________

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
solving_functor_base_type
  : public solving_functor_typenames_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_typenames_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Constructor, members
  public:
    solving_functor_base_type
     (  bool      const &  is_early
      , rate_type const &  damping
      , rate_type const &  rate
     )
      : is_early_exit_( is_early), damping_( damping), rate_( rate) { }
      bool      const &  is_early_exit_  ; /* this is a REF to a bool somewhere else */
      rate_type const    damping_        ;
      rate_type const    rate_           ;

  // Getters
  public:
    bool                 is_early_exit( )         const { return is_early_exit_    ; }
    bool                 not_early_exit( )        const { return ! is_early_exit( ); }
    rate_type const   &  get_damping( )           const { return damping_          ; }
    rate_type const   &  get_rate( )              const { return rate_             ; }
};

// _______________________________________________________________________________________________

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
solving_functor_no_buffer_type
  : public solving_functor_base_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_base_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Functor typedef
  public:
    typedef void (*            solve_function_type
                 )              (  rate_type
                                 , src_iter_0_type const &
                                 , src_iter_0_type const &
                                 , trg_iter_0_type const &
                                );

  // Constructor
  public:
    solving_functor_no_buffer_type
     (  solve_function_type const    solve_function
      , bool                const &  is_early
      , rate_type           const &  rate
     )
      : super_type( is_early, 1, rate)
      , solve_function_( solve_function) { }
      solve_function_type const solve_function_;

  // Functor, single param (src/trg pair object), used in map(..) functions
  public:
      void
    operator ()( src_trg_count_1_type src_trg) const
      {
        // Unpack to a pair of src/trg iters, and change iters into ranges.
        operator ()( src_trg.get<0>( ).get_range( ), src_trg.get<1>( ).get_range( ));
      }

  // Functor, 2 range params
  public:
      void
    operator ()
     (  src_range_0_type const &  src_range
      , trg_range_0_type const &  trg_range
     ) const
      {
        d_assert( src_range.get_count( ) == trg_range.get_count( ));
        if ( super_type::not_early_exit( ) ) {
          solve_function_
           (  super_type::get_rate( )
            , src_range.get_iter_lo( ), src_range.get_iter_post( )
            , trg_range.get_iter_lo( )
           );
        }
      }
};

// _______________________________________________________________________________________________

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
solving_functor_two_buffer_type
  : public solving_functor_base_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
  , public solving_functor_typenames_buf_type
            <  BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_base_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
    typedef solving_functor_typenames_buf_type
             <  BUF_ITER_TYPE
             >                 super2_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES( super2_type);

  // Typedefs
  public:
    typedef pair_iter
             <  src_trg_iter_1_type
              , buf_buf_iter_0_type
             >              quad_iter_type       ;
    typedef typename quad_iter_type::counter_type
                            quad_count_type      ;

    typedef void (*         solve_function_type
                 )           (  rate_type
                              , rate_type
                              , src_iter_0_type const &
                              , src_iter_0_type const &
                              , trg_iter_0_type const &
                              , buf_iter_type const &
                              , buf_iter_type const &
                             )                   ;

  // Constructor
  public:
    solving_functor_two_buffer_type
     (  solve_function_type const    solve_function
      , bool                const &  is_early
      , rate_type           const &  damping
      , rate_type           const &  rate
     )
      : super_type( is_early, damping, rate)
      , solve_function_( solve_function) { }
      solve_function_type const solve_function_;

  // Functor, single quad param, used in parallel map(..) functions
  public:
      void
    operator ()( quad_count_type src_trg_bb) const
      {
        // Unpack the single quad param to two iter-pair (src_trg and buf_buf) params.
        operator ()( src_trg_bb.get<0>( ), src_trg_bb.get<1>( ));
      }

  // Functor, 2 iter params
  public:
      void
    operator ()
     (  src_trg_iter_1_type const &  src_trg  // src and trg iters
      , buf_buf_iter_0_type const &  buf_buf  // pair of buf iters
     ) const
      {
        src_range_0_type  src_range   = (*src_trg).get<0>( ).get_range( );
        trg_range_0_type  trg_range   = (*src_trg).get<1>( ).get_range( );

        buf_iter_0_type   buf_iter_a  = (*buf_buf).get<0>( );
        buf_iter_0_type   buf_iter_b  = (*buf_buf).get<1>( );

        d_assert( buf_iter_a.get_stride( ) >= static_cast< diff_type >( src_range.get_count( )));
        d_assert( buf_iter_b.get_stride( ) >= static_cast< diff_type >( src_range.get_count( )));

        // Unpack to 4 params.
        operator ()
         (  src_range
          , trg_range
          , buf_iter_a.get_leaf_iter( )
          , buf_iter_b.get_leaf_iter( )
         );
      }

  // Functor, 4 params
  public:
      void
    operator ()
     (  src_range_0_type const &  src_range
      , trg_range_0_type const &  trg_range
      , buf_iter_type    const &  buf_iter_a
      , buf_iter_type    const &  buf_iter_b
     ) const
      {
        d_assert( src_range.get_count( ) == trg_range.get_count( ));
        if ( super_type::not_early_exit( ) ) {
          solve_function_
           (  super_type::get_damping( )
            , super_type::get_rate( )
            , src_range.get_iter_lo( ), src_range.get_iter_post( )
            , trg_range.get_iter_lo( )
            , buf_iter_a, buf_iter_b
           );
        }
      }
};

// _______________________________________________________________________________________________

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
solving_functor_fixed_two_buffer_type
  : public solving_functor_two_buffer_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_two_buffer_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >                  super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES(  super_type);

    typedef typename super_type::quad_iter_type        quad_iter_type       ;
    typedef typename super_type::quad_count_type       quad_count_type      ;
    typedef typename super_type::solve_function_type   solve_function_type  ;

  // Constructor
  public:
    solving_functor_fixed_two_buffer_type
     (  solve_function_type const    solve_function
      , bool                const &  is_early
      , rate_type           const &  damping
      , rate_type           const &  rate
      , buf_iter_type       const &  buf_iter_a
      , buf_iter_type       const &  buf_iter_b
     )
      : super_type( solve_function, is_early, damping, rate)
      , buf_iter_a_( buf_iter_a)
      , buf_iter_b_( buf_iter_b)
      { }
      buf_iter_type const buf_iter_a_ ;
      buf_iter_type const buf_iter_b_ ;

  // Functor, a single pair param, used in serial map(..) functions
  public:
      void
    operator ()( src_trg_count_1_type src_trg) const
      {
        // Unpack the pair into two range params.
        operator ()( src_trg.get<0>( ).get_range( ), src_trg.get<1>( ).get_range( ));
      }

  // Functor, two range params
  public:
      void
    operator ()
     (  src_range_0_type const &  src_range
      , trg_range_0_type const &  trg_range
     ) const
      {
        // Invoke the 4-param functor. See supertype.
        super_type::operator ()( src_range, trg_range, buf_iter_a_, buf_iter_b_);
      }
};

// _______________________________________________________________________________________________
// Special solving functor, like above except only usable for 2d forward diff
//
//   Unlike the above this must be constructed for each solve, or at least any time it is
//   used with a src sheet that has changed.
//
//   You can use this serial or parallel since forward-diff does not use any temp buffers.

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
solving_functor_forward_diff_2d_type
  : public solving_functor_base_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_base_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Constructor
  public:
    solving_functor_forward_diff_2d_type
     (  bool                const &  is_early
      , rate_type           const &  damping
      , rate_type           const &  rate
      , rate_type           const &  rate_side
      , src_iter_1_type     const &  src_iter_1_lo // needed so we know if we're at the lo edge
      , src_iter_1_type     const &  src_iter_1_hi // needed so we know if we're at the hi edge
     )
      : super_type( is_early, damping, rate)
      , rate_side_     ( rate_side     )
      , src_iter_1_lo_ ( src_iter_1_lo )
      , src_iter_1_hi_ ( src_iter_1_hi )
      { }
      rate_type       const  rate_side_     ;
      src_iter_1_type const  src_iter_1_lo_ ;
      src_iter_1_type const  src_iter_1_hi_ ;

  // Functor, pair param, pair of src/trg iters
  // This is the operator() called by the mapping functor, serial and parallel.
  public:
      void
    operator ()( src_trg_count_1_type src_trg) const
      {
        src_iter_1_type  const &  src_iter_1   = src_trg.get<0>( );
        src_range_0_type const &  src_range_0  = src_iter_1.get_range( );
        trg_iter_1_type  const &  trg_iter_1   = src_trg.get<1>( );
        trg_range_0_type const &  trg_range_0  = trg_iter_1.get_range( );
        bool             const    is_lo_edge   = (src_iter_1 == src_iter_1_lo_);
        bool             const    is_hi_edge   = (src_iter_1 == src_iter_1_hi_);

        d_assert( src_range_0.get_count( ) == trg_range_0.get_count( ));
        if ( super_type::is_early_exit( ) ) {
            /* do nothing */
        } else
        if ( (! is_lo_edge) && (! is_hi_edge) ) {
            finite_difference::
            calc_next_generation_forward_difference_2d_middle
             (  super_type::get_damping( )
              , super_type::get_rate( )
              , rate_side_
              , src_range_0.get_iter_lo( ), src_range_0.get_iter_post( )
              , (src_iter_1 - 1).get_range( ).get_iter_lo( )
              , (src_iter_1 + 1).get_range( ).get_iter_lo( )
              , trg_range_0.get_iter_lo( )
             );
        } else
        if ( ! is_lo_edge ) {
            finite_difference::
            calc_next_generation_forward_difference_2d_edge
             (  super_type::get_damping( )
              , super_type::get_rate( )
              , rate_side_
              , src_range_0.get_iter_lo( ), src_range_0.get_iter_post( )
              , (src_iter_1 - 1).get_range( ).get_iter_lo( )
              , trg_range_0.get_iter_lo( )
             );
        } else
        if ( ! is_hi_edge ) {
            finite_difference::
            calc_next_generation_forward_difference_2d_edge
             (  super_type::get_damping( )
              , super_type::get_rate( )
              , rate_side_
              , src_range_0.get_iter_lo( ), src_range_0.get_iter_post( )
              , (src_iter_1 + 1).get_range( ).get_iter_lo( )
              , trg_range_0.get_iter_lo( )
             );
        } else
        /* both lo and hi edge (range_1 must be only one wide) */ {
            finite_difference::
            calc_next_generation_forward_difference_2d_thin_strip
             (  super_type::get_damping( )
              , super_type::get_rate( )
              , src_range_0.get_iter_lo( ), src_range_0.get_iter_post( )
              , trg_range_0.get_iter_lo( )
             );
        }
      }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Map(..) functions, serial and parallel

  template< typename SOLVING_FUNCTOR_TYPE >
  void
map_serial
 (  SOLVING_FUNCTOR_TYPE                             solving_functor
  , typename SOLVING_FUNCTOR_TYPE::src_range_1_type  src_range
  , typename SOLVING_FUNCTOR_TYPE::trg_range_1_type  trg_range
 )
  // Use this with any solving functor that has an operator() that accepts a src/trg pair. Namely:
  //   solving_functor_no_buffer_type<..>
  //   solving_functor_fixed_two_buffer_type<..>
  //   solving_functor_forward_diff_2d_type<..>
{
    // The src and trg must be the same width.
    d_assert( src_range.get_count( ) == trg_range.get_count( ));

    // Src/trg pair type.
      typedef typename SOLVING_FUNCTOR_TYPE::src_trg_iter_1_type
    src_trg_pair_iter_type;

    // Create two src/trg pairs:
    //   One to start the iteration.
    //   A limit that is past the end of the iteration.
    src_trg_pair_iter_type        iter_lo( src_range.get_iter_lo( ), trg_range.get_iter_lo( ));
    src_trg_pair_iter_type const  iter_post( src_range.get_iter_post( ));

    // Iterate thru the src/trg pairs, solving for each.
    std::for_each( iter_lo, iter_post, solving_functor);
}

  template< typename SOLVING_FUNCTOR_TYPE >
  void
map_parallel
 (  SOLVING_FUNCTOR_TYPE                             solving_functor
  , typename SOLVING_FUNCTOR_TYPE::src_range_1_type  src_range
  , typename SOLVING_FUNCTOR_TYPE::trg_range_1_type  trg_range
 )
  // Use this with any solving functor that has an operator() that accepts a src/trg pair. Namely:
  //   solving_functor_no_buffer_type<..>
  //   solving_functor_fixed_two_buffer_type<..>
  //   solving_functor_forward_diff_2d_type<..>
{
    // The src and trg must be the same width.
    d_assert( src_range.get_count( ) == trg_range.get_count( ));

    // Src/trg pair type.
      typedef typename SOLVING_FUNCTOR_TYPE::src_trg_iter_1_type
    src_trg_pair_iter_type;

    // Create two src/trg pairs:
    //   One to start the iteration.
    //   A limit that is past the end of the iteration.
    src_trg_pair_iter_type        iter_lo( src_range.get_iter_lo( ), trg_range.get_iter_lo( ));
    src_trg_pair_iter_type const  iter_post( src_range.get_iter_post( ));

    // Iterate thru the src/trg pairs, solving for each.
    QtConcurrent::blockingMap( iter_lo, iter_post, solving_functor);
}

  template< typename SOLVING_FUNCTOR_TYPE >
  void
map_parallel
 (  SOLVING_FUNCTOR_TYPE                             solving_functor
  , typename SOLVING_FUNCTOR_TYPE::src_range_1_type  src_range
  , typename SOLVING_FUNCTOR_TYPE::trg_range_1_type  trg_range
  , typename SOLVING_FUNCTOR_TYPE::buf_range_0_type  buf_range_a
  , typename SOLVING_FUNCTOR_TYPE::buf_range_0_type  buf_range_b
 )
  // Use this with any solving functor that has an operator() that accepts a src/trg + buf/buf
  // quad (or pair-of-pairs).
  //
  // Only one of the solving-functor templates accepts quads:
  //   solving_functor_two_buffer_type<..>
  // So instead of SOLVING_FUNCTOR_TYPE we could have these template params:
  //   RATE_TYPE
  //   SRC_ITER_TYPE
  //   TRG_ITER_TYPE
  //   BUF_ITER_TYPE
{
    // The src, trg, and both buffers must all be the same width.
    d_assert( src_range.get_count( ) == trg_range.get_count( ));
    d_assert( src_range.get_count( ) <= buf_range_a.get_count( ));
    d_assert( src_range.get_count( ) <= buf_range_b.get_count( ));

    // Types we need to construct the quads.
    typedef typename SOLVING_FUNCTOR_TYPE::src_trg_iter_1_type  src_trg_iter_1_type;
    typedef typename SOLVING_FUNCTOR_TYPE::buf_buf_iter_0_type  buf_buf_iter_0_type;
    typedef typename SOLVING_FUNCTOR_TYPE::quad_iter_type       quad_iter_type     ;

    // We need 2 quad objects:
    //   One that starts the iteration, and
    //   One that is past the end of the iteration (this limit or post).
    quad_iter_type
        iter_lo
         (  src_trg_iter_1_type( src_range.get_iter_lo( ), trg_range.get_iter_lo( ))
          , buf_buf_iter_0_type( buf_range_a.get_iter_lo( ), buf_range_b.get_iter_lo( ))
         );
    quad_iter_type
        iter_post
         ( src_trg_iter_1_type( src_range.get_iter_post( ))
         );

    // Iterate thru the quads, solving for each.
    QtConcurrent::blockingMap( iter_lo, iter_post, solving_functor);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Solving functions for specific solve techniques

// _______________________________________________________________________________________________
// Forward diff 1d, serial and parallel

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
calc_next_1d_forward_diff_serial
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
  )
{
    // Solve (1d) and put the results in trg.
    // Forward-diff does not use buffers.
    map_serial
     (  solving_functor_no_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_forward_difference_1d
          , is_early_exit
          , rate
         )
      , src_range
      , trg_range
     );
}

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
calc_next_1d_forward_diff_parallel
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
  )
{
    // Solve (1d) and put the results in trg.
    // Forward-diff does not use buffers.
    map_parallel
     (  solving_functor_no_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_forward_difference_1d
          , is_early_exit
          , rate
         )
      , src_range
      , trg_range
     );
}

// _______________________________________________________________________________________________
// Backward diff, serial and parallel

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  void
calc_next_1d_backward_diff_serial
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
   , BUF_ITER_TYPE                    const &  buf_iter_a
   , BUF_ITER_TYPE                    const &  buf_iter_b
  )
{
    // Solve (1d) and put the results in trg.
    map_serial
     (  solving_functor_fixed_two_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
          , BUF_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_backward_difference_1d
          , is_early_exit
          , damping
          , rate
          , buf_iter_a
          , buf_iter_b
         )
      , src_range
      , trg_range
     );
}

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  void
calc_next_1d_backward_diff_parallel
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
   , stride_range< BUF_ITER_TYPE, 0 > const &  buf_range_a
   , stride_range< BUF_ITER_TYPE, 0 > const &  buf_range_b
  )
{
    // Solve (1d) and put the results in trg.
    map_parallel
     (  solving_functor_two_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
          , BUF_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_backward_difference_1d
          , is_early_exit
          , damping
          , rate
         )
      , src_range
      , trg_range
      , buf_range_a
      , buf_range_b
     );
}

// _______________________________________________________________________________________________
// Central diff, serial and parallel

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  void
calc_next_1d_central_diff_serial
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
   , BUF_ITER_TYPE                    const &  buf_iter_a
   , BUF_ITER_TYPE                    const &  buf_iter_b
  )
{
    // Solve (1d) and put the results in trg.
    map_serial
     (  solving_functor_fixed_two_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
          , BUF_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_central_difference_1d
          , is_early_exit
          , damping
          , rate
          , buf_iter_a
          , buf_iter_b
         )
      , src_range
      , trg_range
     );
}

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  void
calc_next_1d_central_diff_parallel
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
   , stride_range< BUF_ITER_TYPE, 0 > const &  buf_range_a
   , stride_range< BUF_ITER_TYPE, 0 > const &  buf_range_b
  )
{
    // Solve (1d) and put the results in trg.
    map_parallel
     (  solving_functor_two_buffer_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
          , BUF_ITER_TYPE
         >
         (  finite_difference::calc_next_generation_central_difference_1d
          , is_early_exit
          , damping
          , rate
         )
      , src_range
      , trg_range
      , buf_range_a
      , buf_range_b
     );
}

// _______________________________________________________________________________________________
// Forward diff 2d, serial and parallel

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
calc_next_2d_forward_diff_serial
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , RATE_TYPE                        const &  rate_side
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
  )
{
    // Solve and put the results in trg.
    map_serial
     (  solving_functor_forward_diff_2d_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
         >
         (  is_early_exit
          , damping
          , rate
          , rate_side
          , src_range.get_iter_lo( )
          , src_range.get_iter_hi( )
         )
      , src_range
      , trg_range
     );
}

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
calc_next_2d_forward_diff_parallel
  (  bool                             const &  is_early_exit
   , RATE_TYPE                        const &  damping
   , RATE_TYPE                        const &  rate
   , RATE_TYPE                        const &  rate_side
   , stride_range< SRC_ITER_TYPE, 1 > const &  src_range
   , stride_range< TRG_ITER_TYPE, 1 > const &  trg_range
  )
{
    // Solve and put the results in trg.
    map_parallel
     (  solving_functor_forward_diff_2d_type
         <  RATE_TYPE
          , SRC_ITER_TYPE
          , TRG_ITER_TYPE
         >
         (  is_early_exit
          , damping
          , rate
          , rate_side
          , src_range.get_iter_lo( )
          , src_range.get_iter_hi( )
         )
      , src_range
      , trg_range
     );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Unified functor

// _______________________________________________________________________________________________
// Supertype, base with virtual, no buffers

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
calc_next_1d_functor_super_type
  : public solving_functor_typenames_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef solving_functor_typenames_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Constructor, member, getters
  public:
    calc_next_1d_functor_super_type( bool const & is_early)
      : is_early_exit_( is_early) { }
      bool const & is_early_exit_;
    bool  is_early_exit(  )  const { return is_early_exit_; }
    bool  not_early_exit( )  const { return ! is_early_exit( ); }

  // Make the dtor virtual
  public:
      virtual
    ~calc_next_1d_functor_super_type( )
      { }

  // Buffer size methods
  public:
      virtual /* base virtual */
      size_type
    get_min_buf_count( size_type, size_type) const
      { return 0; }

      /* not virtual, calls virtual */
      size_type
    get_min_buf_count( src_range_1_type const & src_range) const
      { return get_min_buf_count( src_range.get_count( ), src_range.get_next_range( ).get_count( )); }

  // Functor operator, pure base virtual
  public:
      virtual
      void
    operator ()
     (  rate_type        const &  damping
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      = 0; /* pure virual */
};

// _______________________________________________________________________________________________
// Supertype, serial, two buffers

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_serial_functor_super_type
  : public calc_next_1d_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
  , public solving_functor_typenames_buf_type
            <  BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
    typedef solving_functor_typenames_buf_type
             <  BUF_ITER_TYPE
             >                 super2_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES( super2_type);

  // Constructor, members
  public:
    calc_next_1d_serial_functor_super_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_functor_super_type< rate_type, src_iter_type, trg_iter_type >( is_early)
      , buf_iter_a_( buf_iter_a)
      , buf_iter_b_( buf_iter_b)
      { }
      buf_iter_type const & buf_iter_a_ ;
      buf_iter_type const & buf_iter_b_ ;

  // Buffer size method
  public:
      virtual /* overridden virtual */
      size_type
    get_min_buf_count( size_type x_size, size_type y_size) const
      { return std::max( x_size, y_size); }
};

// _______________________________________________________________________________________________
// Supertype, parallel, two buffers

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_parallel_functor_super_type
  : public calc_next_1d_serial_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_serial_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
              , BUF_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES( super_type);

  // Constructor
  public:
    calc_next_1d_parallel_functor_super_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_serial_functor_super_type< rate_type, src_iter_type, trg_iter_type, buf_iter_type >
         ( is_early, buf_iter_a, buf_iter_b)
      { }

  // Buffer size method
  public:
      virtual /* overridden virtual */
      size_type
    get_min_buf_count( size_type x_size, size_type y_size) const
      { return x_size * y_size; }

  // Buffer setup
  public:
      static
      buf_range_0_type
    get_parallel_buf_range
     (  src_range_1_type const &  src_range
      , buf_iter_type    const &  buf
     )
      { size_type const count  = src_range.get_count( );
        diff_type const stride = src_range.get_next_range( ).get_count( );
        return buf_range_0_type( count, stride, buf);
      }
};

// _______________________________________________________________________________________________
// Forward-diff subtypes

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
calc_next_1d_forward_diff_serial_functor_type
  : public calc_next_1d_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Constructor
  public:
    calc_next_1d_forward_diff_serial_functor_type( bool const & is_early)
      : calc_next_1d_functor_super_type< rate_type, src_iter_type, trg_iter_type >( is_early)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  /* damping - not used */
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_forward_diff_serial( super_type::is_early_exit_, rate, src_range, trg_range); }
};

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  struct
calc_next_1d_forward_diff_parallel_functor_type
  : public calc_next_1d_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);

  // Constructor
  public:
    calc_next_1d_forward_diff_parallel_functor_type( bool const & is_early)
      : calc_next_1d_functor_super_type< rate_type, src_iter_type, trg_iter_type >( is_early)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  /* damping - not used */
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_forward_diff_parallel( super_type::is_early_exit_, rate, src_range, trg_range); }
};

// _______________________________________________________________________________________________
// Backward-diff subtypes

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_backward_diff_serial_functor_type
  : public calc_next_1d_serial_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_serial_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
              , BUF_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES(  super_type);

  // Constructor
  public:
    calc_next_1d_backward_diff_serial_functor_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_serial_functor_super_type< rate_type, src_iter_type, trg_iter_type, buf_iter_type >
         ( is_early, buf_iter_a, buf_iter_b)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  damping
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_backward_diff_serial
         (  super_type::is_early_exit_
          , damping
          , rate
          , src_range
          , trg_range
          , super_type::buf_iter_a_
          , super_type::buf_iter_b_
         );
      }
};

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_backward_diff_parallel_functor_type
  : public calc_next_1d_parallel_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_parallel_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
              , BUF_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES(  super_type);

  // Constructor
  public:
    calc_next_1d_backward_diff_parallel_functor_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_parallel_functor_super_type< rate_type, src_iter_type, trg_iter_type, buf_iter_type >
         ( is_early, buf_iter_a, buf_iter_b)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  damping
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_backward_diff_parallel
         (  super_type::is_early_exit_
          , damping
          , rate
          , src_range
          , trg_range
          , super_type::get_parallel_buf_range( src_range, super_type::buf_iter_a_)
          , super_type::get_parallel_buf_range( src_range, super_type::buf_iter_b_)
         );
      }
};

// _______________________________________________________________________________________________
// Central-diff subtypes

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_central_diff_serial_functor_type
  : public calc_next_1d_serial_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_serial_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
              , BUF_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES(  super_type);

  // Constructor
  public:
    calc_next_1d_central_diff_serial_functor_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_serial_functor_super_type< rate_type, src_iter_type, trg_iter_type, buf_iter_type >
         ( is_early, buf_iter_a, buf_iter_b)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  damping
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_central_diff_serial
         (  super_type::is_early_exit_
          , damping
          , rate
          , src_range
          , trg_range
          , super_type::buf_iter_a_
          , super_type::buf_iter_b_
         );
      }
};

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename BUF_ITER_TYPE
   >
  struct
calc_next_1d_central_diff_parallel_functor_type
  : public calc_next_1d_parallel_functor_super_type
            <  RATE_TYPE
             , SRC_ITER_TYPE
             , TRG_ITER_TYPE
             , BUF_ITER_TYPE
            >
{
  // Inherited typedefs
  private:
    typedef calc_next_1d_parallel_functor_super_type
             <  RATE_TYPE
              , SRC_ITER_TYPE
              , TRG_ITER_TYPE
              , BUF_ITER_TYPE
             >                 super_type;
  public:
    INHERIT_FUNCTOR_TYPENAMES( super_type);
    INHERIT_BUFFER_TYPENAMES(  super_type);

  // Constructor
  public:
    calc_next_1d_central_diff_parallel_functor_type
     (  bool          const & is_early
      , buf_iter_type const & buf_iter_a
      , buf_iter_type const & buf_iter_b
     )
      : calc_next_1d_parallel_functor_super_type< rate_type, src_iter_type, trg_iter_type, buf_iter_type >
         ( is_early, buf_iter_a, buf_iter_b)
      { }

  // Functor operator, overridden virtual
  public:
      virtual /* overridden pure virtual */
      void
    operator ()
     (  rate_type        const &  damping
      , rate_type        const &  rate
      , src_range_1_type const &  src_range
      , trg_range_1_type const &  trg_range
     ) const
      { calc_next_1d_central_diff_parallel
         (  super_type::is_early_exit_
          , damping
          , rate
          , src_range
          , trg_range
          , super_type::get_parallel_buf_range( src_range, super_type::buf_iter_a_)
          , super_type::get_parallel_buf_range( src_range, super_type::buf_iter_b_)
         );
      }
};

// _______________________________________________________________________________________________

# undef INHERIT_FUNCTOR_TYPENAMES
# undef INHERIT_BUFFER_TYPENAMES

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef FINITE_DIFF_SOLVER_H
//
// finite_diff_solver.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
