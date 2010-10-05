// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// tri_diag.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef TRI_DIAG_H
# define TRI_DIAG_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "util.h"

namespace linear_algebra {

// _______________________________________________________________________________________________
// solve_tridiagonal_destructive( count, a, b, c, d, x)
//
//   Finds the vector X in the equation (given Array and D):
//     Array * D = X
//
//   The array must be tridiagonal.
//   Does not invert the array. Instead solves using the Thomas algorithm which is a form of
//   Gaussian elimination. Also called the TriDiagonal Matrix Algorithm or TDMA.
//
//   See:
//     http://en.wikipedia.org/wiki/Tridiagonal_matrix
//     http://www.cfd-online.com/Wiki/Tridiagonal_matrix_algorithm_-_TDMA_(Thomas_algorithm)
//     http://www.cplusplus.com/forum/beginner/6788/
//     http://en.wikipedia.org/wiki/Tridiagonal_matrix_algorithm
//
//   The last source (wikipedia.org) presents code that is not obviously the same as this
//   implementation (28 April 2009). The wikipedia page also does not mention that its
//   solution should be restricted to cases where |b(i)| > |a(i)| + |c(i)|, so maybe the
//   wikipedia solution is more generally applicable?
//
//   Is this a solution only for a positive definite tridiagonal matrix?
//
//   The parameters are:
//     count    - the size of all the arrays - must be at least 1
//     a        - the sub-diagonal of the tridiagonal array
//                  The first item is not used and should be set to zero.
//     b        - the main diagonal of the array
//                  All items in the main diagonal must be non-zero.
//     c        - the super-diagonal of the array
//                  The last item is not used and should be set to zero.
//     d        - the result vector (Array * X)
//     d_stride -
//     x        - the vector we need to solve
//                  There is no need to set initial values in X.
//     x_stride -
//
//   This function returns the solution in the x vector. But it also uses b and d
//   for temporary calculations and thus destroys them.
//
//   The names of the LAPACK functions to solve tridiagonal equations are:
//     general           - SGTSV   CGTSV   DGTSV   ZGTSV
//     general (expert)  - SGTSVX  CGTSVX  DGTSVX  ZGTSVX
//     positive definite - SPTSVX  CPTSVX  DPTSVX  ZPTSVX
//
//   Improvement. We could make this work where x was the same as either a, b, c, or d.
//   Improvement. Express this as iterators instead of pointers. Get rid of stride.

// Not used for now.
# if 0
  template< typename ITEM_TYPE >
  void
solve_tridiagonal_destructive(
    std::size_t    const   count     // number of items in each vector below
  , ITEM_TYPE      const * a         // sub-diagonal, first item not use (must be zero)
  , ITEM_TYPE            * b         // main diagonal, no zero entries
  , ITEM_TYPE      const * c         // super-diagonal, last item not used (must be zero)
  , ITEM_TYPE            * d         // vector on other side of equal sign
  , std::ptrdiff_t const   d_stride  // stride for vector d
  , ITEM_TYPE            * x         // vector to be solved
  , std::ptrdiff_t const   x_stride  // stride for vector x
  )
{
    // This works when count == 1.
    d_assert( 0 < count);
    std::size_t const count_minus = count - 1;

    // We need pointers to the first and last items to control the loops.
    ITEM_TYPE const * const b_first = b;
    ITEM_TYPE const * const b_last  = b + count_minus;
    d_assert( b_first <= b_last);

    // Debug-only code.
#   ifndef NDEBUG
    // We only use b_first and b_last to control the loops, but we check
    // these in asserts below.
    ITEM_TYPE const * const a_first = a;
    ITEM_TYPE const * const a_last  = a + count_minus;

    ITEM_TYPE const * const c_first = c;
    ITEM_TYPE const * const c_last  = c + count_minus;

    ITEM_TYPE const * const d_first = d;
    ITEM_TYPE const * const d_last  = d + (count_minus * d_stride);

    ITEM_TYPE const * const x_first = x;

    // This algorithm only works when (|b| > |a| + |c|) (for every item in
    // the arrays), so check that here. This assumes the two unused items
    // (the first a, the last c) are both set to zero. This also assures us
    // that no diagonal values (in b) are zero.
    {   ITEM_TYPE const * a_debug = a;
        ITEM_TYPE const * b_debug = b;
        ITEM_TYPE const * c_debug = c;
        do {
            d_assert(
              util::absolute_value( *b_debug) >
              (util::absolute_value( *a_debug) + util::absolute_value( *c_debug)));
            a_debug += 1;
            b_debug += 1;
            c_debug += 1;
        }
        while ( b_debug <= b_last );
    }
#   endif

    // The first (unused) element in a should be set to zero.
    d_assert( 0 == *a);
    while ( b != b_last ) {
        // No values in b should be zero.
        d_assert( 0 != *b);

        // We don't use the first value in a.
        a += 1;
        ITEM_TYPE const scale = (*a) / (*b);

        // Use b for temporary storage.
        b += 1;
        *b -= scale * (*c);

        // Use d for temporary storage.
        ITEM_TYPE const delta = scale * (*d);
        d += d_stride;
        *d -= delta;

        // We don't use the last value in c.
        c += 1;
    }
    // No values in b should be zero.
    d_assert( 0 != *b);

    // The pointers (except x) should now be pointing at the last items.
    d_assert( a_last == a);
    d_assert( b_last == b);
    d_assert( c_last == c);
    d_assert( d_last == d);

    // The last (unused) value in c should be zero.
    d_assert( 0 == *c);

    // Sweep backwards over all the arrays assigning items in x.
    // First dvance x to the last element and assign it.
    x += count_minus * x_stride;
    *x = (*d) / (*b);
    while ( b != b_first ) {
        // Move the array pointers back.
        // Don't bother with a because we are done with it.
        b -= 1;
        c -= 1;
        d -= d_stride;

        // Calculate the previous value of x.
        ITEM_TYPE const prev_x = ((*d) - ((*c) * (*x))) / (*b);
        // Assign the previous value of x.
        x -= x_stride;
        *x = prev_x;
    }

    // All the pointers (except a) should now be pointing at the first items.
    d_assert( b_first == b);
    d_assert( c_first == c);
    d_assert( d_first == d);
    d_assert( x_first == x);
}
# endif

// _______________________________________________________________________________________________
// solve_tridiagonal_destructive( count, a_const, b, c_const, d, x)
//
//   This overloaded version of solve_tridiagonal_destructive(..) takes care of the case where
//   the sub- and super-diagonals are constant values.
//   We could also provide overloads where only one of the sub- or super-diagonals are const.
//
//   Another case to consider is where the values on the main diagonal (b) are all the same.
//   But this algorithm uses the main-diagonal vector as scratch space so it still needs the
//   array.
//
//   The parameters are:
//     count     - the size of all the arrays - must be at least 1
//     a_const   - the value filling the sub-diagonal of the tridiagonal array
//     b         - the main diagonal of the array
//                   All items in the main diagonal must be non-zero.
//     c_const   - the value filling the super-diagonal of the array
//     d         - the result vector (Array * X)
//     d_stride  -
//     x         - the vector we need to solve
//                   There is no need to set initial values in X.
//     x_stride  -
//
//   This function returns the solution in the x vector. But it also uses b and d
//   for temporary calculations and thus destroys them.

// Not used for now.
# if 0
  template< typename ITEM_TYPE >
  void
solve_tridiagonal_destructive(
    std::size_t    const   count      // number of items in each vector below
  , ITEM_TYPE      const   a_const    // value filling the sub-diagonal
  , ITEM_TYPE            * b          // main diagonal, no zero entries
  , ITEM_TYPE      const   c_const    // value filling the super-diagonal
  , ITEM_TYPE            * d          // vector on other side of equal sign
  , std::ptrdiff_t const   d_stride   // stride for vector d
  , ITEM_TYPE            * x          // vector to be solved
  , std::ptrdiff_t const   x_stride   // stride for vector x
  )
{
    // This works when count == 1.
    d_assert( 0 < count);
    std::size_t const count_minus = count - 1;

    // We need pointers to the first and last items to control the loops.
    ITEM_TYPE const * const b_first = b;
    ITEM_TYPE const * const b_last  = b + count_minus;
    d_assert( b_first <= b_last);

    // Debug-only code.
#   ifndef NDEBUG
    // We only use b_first and b_last to control the loops, but we check
    // these in asserts below.
    ITEM_TYPE const * const d_first = d;
    ITEM_TYPE const * const d_last  = d + (count_minus * d_stride);

    ITEM_TYPE const * const x_first = x;

    // This algorithm only works when (|b| > |a| + |c|) (for every item in
    // the arrays), so check that here. This assumes the two unused items
    // (the first a, the last c) are both set to zero. This also assures us
    // that no diagonal values (in b) are zero.
    {   ITEM_TYPE const   a_plus_c = util::absolute_value( a_const) +
                                     util::absolute_value( c_const);
        ITEM_TYPE const * b_debug  = b;
        do {
            d_assert( util::absolute_value( *b_debug) > a_plus_c);
            b_debug += 1;
        }
        while ( b_debug <= b_last );
    }
#   endif

    // Forward iteration
    while ( b != b_last ) {
        // No values in b should be zero.
        d_assert( 0 != *b);
        ITEM_TYPE const scale = a_const / (*b);

        // Use b for temporary storage.
        b += 1;
        *b -= scale * c_const;

        // Use d for temporary storage.
        ITEM_TYPE const delta = scale * (*d);
        d += d_stride;
        *d -= delta;
    }
    // No values in b should be zero.
    d_assert( 0 != *b);

    // The pointers should now be pointing at the last items.
    d_assert( b_last == b);
    d_assert( d_last == d);

    // Sweep backwards over all the arrays assigning items in x.
    // First dvance x to the last element and assign it.
    x += count_minus * x_stride;
    *x = (*d) / (*b);
    while ( b != b_first ) {
        // Move the array pointers back.
        b -= 1;
        d -= d_stride;

        // Calculate the previous value of x.
        ITEM_TYPE const prev_x = ((*d) - (c_const * (*x))) / (*b);
        // Assign the previous value of x.
        x -= x_stride;
        *x = prev_x;
    }

    // The pointers should now be pointing at the first items.
    d_assert( b_first == b);
    d_assert( d_first == d);
    d_assert( x_first == x);
}
# endif

// _______________________________________________________________________________________________
// solve_tridiagonal_destructive
//  (  assign_functor_type (2-param)
//   , count
//   , sub_diag_value, diag_values, super_diag_value
//   , in_vect
//   , out_vect
//  )

  template
   <  typename ASSIGN_FUNCTOR_TYPE
    , typename ITEM_TYPE
    , typename DIAG_ITER_TYPE       // readable and writable, used as input and temp storage
    , typename IN_VECT_ITER_TYPE    // readable and writable, used as input and temp storage
    , typename OUT_VECT_ITER_TYPE   // writable, output and intermediate calculation
   >
  void
solve_tridiagonal_destructive
 (  ASSIGN_FUNCTOR_TYPE    assign_functor    // used to assign final value to out_vect
  , std::size_t     const  count             // number of items in each of the vectors below
  , ITEM_TYPE       const  sub_diag_value    // value filling the sub-diagonal
  , DIAG_ITER_TYPE         diag_vect         // main diagonal, no zero entries
  , ITEM_TYPE       const  super_diag_value  // value filling the super-diagonal
  , IN_VECT_ITER_TYPE      in_vect           // vector on other side of equal sign
  , OUT_VECT_ITER_TYPE     out_vect          // vector to be solved
 )
  // assign_functor is probably either util::assign_set_type( ) or util::assign_sum_type( ).
  //
  // The three vectors must all be writable and all be the same size:
  //   diag_vect  - input and calc space
  //   in_vect    - input and calc space
  //   out_vect   - output only, each value written only once
  //
  // out_vect can be the same as diag_vect or in_vect:
  //   diag_vect != in_vect
  //   OK if diag_vect == out_vect
  //   OK if in_vect   == out_vect
{
    // This works when count == 1.
    d_assert( 0 < count);
    std::size_t const count_minus = count - 1;

    // We need pointers to the first and last items to control the loops.
    DIAG_ITER_TYPE const diag_vect_first = diag_vect;
    DIAG_ITER_TYPE const diag_vect_last  = diag_vect + count_minus;
    d_assert( diag_vect_first <= diag_vect_last);

    // Debug-only code.
#   ifndef NDEBUG
    // We only use diag_vect_first and diag_vect_last to control the loops, but we check
    // these in asserts below.
    IN_VECT_ITER_TYPE  const in_vect_first  = in_vect;
    IN_VECT_ITER_TYPE  const in_vect_last   = in_vect + count_minus;
    OUT_VECT_ITER_TYPE const out_vect_first = out_vect;

    // This algorithm only works when (|diag_vect| > |sub| + |super|) (for every item in
    // the arrays), so check that here. This assumes the two unused items
    // (the first a, the last c) are both set to zero. This also assures us
    // that no diagonal values (in diag_vect) are zero.
    d_assert( util::absolute_value( *diag_vect_first) > util::absolute_value( super_diag_value));
    d_assert( util::absolute_value( *diag_vect_last ) > util::absolute_value( sub_diag_value  ));
    {   ITEM_TYPE const a_plus_c = util::absolute_value( sub_diag_value  ) +
                                   util::absolute_value( super_diag_value);
        for ( DIAG_ITER_TYPE
                diag_vect_debug = diag_vect + 1
            ;   diag_vect_debug < diag_vect_last
            ;   ++ diag_vect_debug )
        {
            d_assert( util::absolute_value( *diag_vect_debug) > a_plus_c);
        }
    }
#   endif

    // Forward iteration
    while ( diag_vect != diag_vect_last ) {
        // No values in diag_vect should be zero.
        d_assert( 0 != (*diag_vect));
        ITEM_TYPE const scale = sub_diag_value / (*diag_vect);

        // Use diag_vect for temporary storage.
        ++ diag_vect;
        (*diag_vect) -= scale * super_diag_value;

        // Use in_vect for temporary storage.
        ITEM_TYPE const delta = scale * (*in_vect);
        ++ in_vect;
        (*in_vect) -= delta;
    }
    // No values in diag_vect should be zero.
    d_assert( 0 != (*diag_vect));

    // The pointers should now be pointing at the last items.
    d_assert( diag_vect_last == diag_vect);
    d_assert( in_vect_last == in_vect);

    // Sweep backwards over all the arrays assigning items in out_vect.
    // First advance out_vect to the last element and assign it.
    out_vect += count_minus;
    ITEM_TYPE out_vect_value = (*in_vect) / (*diag_vect);
    while ( diag_vect != diag_vect_first ) {
        assign_functor( *out_vect, out_vect_value);
        -- out_vect;
        -- diag_vect;
        -- in_vect;
        out_vect_value = ((*in_vect) - (super_diag_value * out_vect_value)) / (*diag_vect);
    }
    assign_functor( *out_vect, out_vect_value);

    // The pointers should now be pointing at the first items.
    d_assert( diag_vect_first == diag_vect);
    d_assert( in_vect_first == in_vect);
    d_assert( out_vect_first == out_vect);
}

// _______________________________________________________________________________________________
// solve_tridiagonal_destructive_extra_careful
//  (  assign2_functor_type (2-param)
//   , count
//   , sub_diag_value, diag_values, super_diag_value
//   , in_vect
//   , out_vect
//  )
//
//   This function is the same as the one above except it doesn't check that the matrix is
//   solvable (by this method), and it is careful to not divide by zero (whenever it sees a
//   zero in a denominator it substitutes a small positive non-zero value).
//   So this is enormous kludge, and only here so I can experiment.
//   I use this when solving the heat/wave equations will illegal rates (that cause the solution
//   to be unstable and blow up).
//
//   This function is here so you can solve tri-diags that probably shouldn't be solvable.
//   At least not solvable using this algorithm.
//   You shouldn't use this in non-experimental code though, because it's inaccurate and
//   it means you're trying to solve with illegal values.

  template
   <  typename ASSIGN_FUNCTOR_TYPE
    , typename ITEM_TYPE
    , typename DIAG_ITER_TYPE       // readable and writable, used as input and temp storage
    , typename IN_VECT_ITER_TYPE    // readable and writable, used as input and temp storage
    , typename OUT_VECT_ITER_TYPE   // writable, output and intermediate calculation
   >
  void
solve_tridiagonal_destructive_extra_careful
 (  ASSIGN_FUNCTOR_TYPE    assign_functor    // used to assign final value to out_vect
  , std::size_t     const  count             // number of items in each of the vectors below
  , ITEM_TYPE       const  sub_diag_value    // value filling the sub-diagonal
  , DIAG_ITER_TYPE         diag_vect         // main diagonal, no zero entries
  , ITEM_TYPE       const  super_diag_value  // value filling the super-diagonal
  , IN_VECT_ITER_TYPE      in_vect           // vector on other side of equal sign
  , OUT_VECT_ITER_TYPE     out_vect          // vector to be solved
  )
{
    // This works when count == 1.
    d_assert( 0 < count);
    std::size_t const count_minus = count - 1;

    // We need pointers to the first and last items to control the loops.
    DIAG_ITER_TYPE const diag_vect_first = diag_vect;
    DIAG_ITER_TYPE const diag_vect_last  = diag_vect + count_minus;
    d_assert( diag_vect_first <= diag_vect_last);

    // Debug-only code.
#   ifndef NDEBUG
    // We only use diag_vect_first and diag_vect_last to control the loops, but we check
    // these in asserts below.
    IN_VECT_ITER_TYPE  const in_vect_first  = in_vect;
    IN_VECT_ITER_TYPE  const in_vect_last   = in_vect + count_minus;
    OUT_VECT_ITER_TYPE const out_vect_first = out_vect;
#   endif

    ITEM_TYPE const small_d_value = 0.00001f;

    // Forward iteration
    while ( diag_vect != diag_vect_last ) {
        ITEM_TYPE const scale = sub_diag_value / ((*diag_vect) ? (*diag_vect) : small_d_value);

        // Use diag_vect for temporary storage.
        ++ diag_vect;
        (*diag_vect) -= scale * super_diag_value;

        // Use in_vect for temporary storage.
        ITEM_TYPE const delta = scale * (*in_vect);
        ++ in_vect;
        (*in_vect) -= delta;
    }

    // The pointers should now be pointing at the last items.
    d_assert( diag_vect_last == diag_vect);
    d_assert( in_vect_last == in_vect);

    // Sweep backwards over all the arrays assigning items in out_vect.
    // First advance out_vect to the last element and assign it.
    out_vect += count_minus;
    ITEM_TYPE out_vect_value = (*in_vect) / ((*diag_vect) ? (*diag_vect) : small_d_value);
    while ( diag_vect != diag_vect_first ) {
        assign_functor( *out_vect, out_vect_value);
        -- out_vect;
        -- diag_vect;
        -- in_vect;
        out_vect_value =
            ((*in_vect) - (super_diag_value * out_vect_value)) /
            ((*diag_vect) ? (*diag_vect) : small_d_value);
    }
    assign_functor( *out_vect, out_vect_value);

    // The pointers should now be pointing at the first items.
    d_assert( diag_vect_first == diag_vect);
    d_assert( in_vect_first == in_vect);
    d_assert( out_vect_first == out_vect);
}

} /* end namespace linear_algebra */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef TRI_DIAG_H
//
// tri_diag.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
