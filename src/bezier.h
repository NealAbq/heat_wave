// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// bezier.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef BEZIER_H
# define BEZIER_H
// _______________________________________________________________________________________________
//
// Generic bezier/spline functions.
// Assumes interpolate( FACTOR_T factor, SCALE_T, SCALE_T) is defined.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  template< typename SCALE_T, typename FACTOR_T >
  typename boost::remove_reference<
    typename boost::remove_cv<
      SCALE_T
    >::type
  >::type
interpolate( FACTOR_T factor, SCALE_T v0, SCALE_T v1)
{
    return (v0 == v1) ? v0 : ((FACTOR_T( 1) - factor) * v0) + (factor * v1);
}

// _______________________________________________________________________________________________

  template< typename SCALE_T, typename FACTOR_T >
  typename boost::remove_reference<
    typename boost::remove_cv<
      SCALE_T
    >::type
  >::type
bezier( FACTOR_T factor, SCALE_T v0, SCALE_T v1, SCALE_T v2)
  //
  // Quadratic Bézier curve.
{
    return
        interpolate( factor,
            interpolate( factor, v0, v1),
            interpolate( factor, v1, v2));
}

// _______________________________________________________________________________________________

  template< typename SCALE_T, typename FACTOR_T >
  typename boost::remove_reference<
    typename boost::remove_cv<
      SCALE_T
    >::type
  >::type
bezier( FACTOR_T factor, SCALE_T v0, SCALE_T v1, SCALE_T v2, SCALE_T v3)
  //
  // Cubic Bézier curve.
{
    // The intuitive definition:
    //
    //    interpolate( factor,
    //        bezier( factor, v0, v1, v2),
    //        bezier( factor, v1, v2, v3));
    //
    // That expands to this:
    //
    //    interpolate( factor,
    //        interpolate( factor,
    //            interpolate( factor, v0, v1),
    //            interpolate( factor, v1, v2)),
    //        interpolate( factor,
    //            interpolate( factor, v1, v2),
    //            interpolate( factor, v2, v3));
    //
    // But that evaluates interpolate( factor, v1, v2) twice.
    // So we use the more efficient definition below.

      boost::add_const<
        boost::remove_reference<
          boost::remove_cv<
            SCALE_T
          >::type
        >::type
      >::type
    v1_v2 = interpolate( factor, v1, v2);
    return
        interpolate( factor,
            interpolate( factor,
                interpolate( factor, v0, v1),
                v1_v2),
            interpolate( factor,
                v1_v2,
                interpolate( factor, v2, v3)));
}

// _______________________________________________________________________________________________

  template< typename SCALE_T, typename FACTOR_T >
  typename boost::remove_reference<
    typename boost::remove_cv<
      SCALE_T
    >::type
  >::type
bezier_spline( FACTOR_T factor, SCALE_T v0, SCALE_T v1, SCALE_T v2, SCALE_T v3)
  //
  // Spline two quadratic Bézier curves together.
  // You can get a smooth curve from v1 thru v2 that will fit with the next curve in the sequence.
  // To get a curve through points v1, v2, v3, v4, and v5 do the following:
  //
  //   for ( factor 0 .. 1 ) bezier_spline( factor, v0, v1, v2, v3)
  //   for ( factor 0 .. 1 ) bezier_spline( factor, v1, v2, v3, v4)
  //   for ( factor 0 .. 1 ) bezier_spline( factor, v2, v3, v4, v5)
  //   for ( factor 0 .. 1 ) bezier_spline( factor, v3, v4, v5, v6)
  //
  // The end control points v0 and v6 control the slope at the ends.
  // If you don't care about that you could add these points before and after the above.
  //   for ( factor 0 .. 1/2 ) bezier( factor, v0, v1, v2)
  //   for ( factor 1/2 .. 1 ) bezier( factor, v4, v5, v6)
{
    return
        interpolate( factor,
            bezier( (factor + FACTOR_T( 1)) / 2, v0, v1, v2),
            bezier( (factor + FACTOR_T( 0)) / 2, v1, v2, v3));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef BEZIER_H
//
// bezier.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
