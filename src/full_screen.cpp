// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// full_screen.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// _______________________________________________________________________________________________

# include "all.h"
# include "full_screen.h"
# include <QtGui/QMessageBox>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Static helper functions
// _______________________________________________________________________________________________

  static
  QLayout *
findLayoutHoldingChild( QLayout * const pLayout, QWidget * const pChild)
  //
  // Assumes a child window cannot be held by more than one layout-item.
{
    d_assert( pLayout);
    d_assert( pChild);

    // If pLayout is not enabled maybe we need to do more when we restore?
    d_assert( pLayout->isEnabled( ));

    // Loop thru all the child layout-items in pLayout.
    int const indexLimit = pLayout->count( );
    for ( int
            index = 0
        ;   index < indexLimit
        ;   index += 1 )
    {
        QLayoutItem * const pLayoutItem = pLayout->itemAt( index);
        d_assert( pLayoutItem);

        QWidget * const pLayoutWidget = pLayoutItem->widget( );
        if ( pLayoutWidget == pChild ) {
            // pLayout is the layout holding pChild.
            return pLayout;
        }
        else {
            QLayout * const pChildLayout = pLayoutItem->layout( );
            if ( pChildLayout ) {
                // Recurse.
                QLayout * const pFoundLayout = findLayoutHoldingChild( pChildLayout, pChild);
                // If we find the holder layout stop searching and return immediately.
                if ( pFoundLayout ) return pFoundLayout;
            }
        }
    }
    // Return 0 to indicate pChild is not held by any layout items under pLayout.
    return 0;
}

// _______________________________________________________________________________________________

  static
  QLayout *
findLayoutHoldingChild( QWidget * const pParent, QWidget * const pChild)
{
    d_assert( pParent);
    d_assert( pChild);

    // Get the layout object that positions the children of pParent.
    QLayout * const pTopLayout = pParent->layout( );
    if ( pTopLayout ) {
        d_assert( pTopLayout->parentWidget( ) == pParent);

        // Recursively search the tree of layout items.
        return findLayoutHoldingChild( pTopLayout, pChild);
    }
    // Return 0 to indicate the parent has no layout objects.
    return 0;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Class QeFullScreenChild
// _______________________________________________________________________________________________

  /* constructor */
  QeFullScreenChild::
QeFullScreenChild( QWidget * pCh)
  : QObject            ( )
  , pChild             ( pCh)
  , pParent            ( 0)
  , pLayout            ( 0)
  , childRestoreFlags  ( 0)
  , parentRestoreState ( e_normal)
{
    d_assert( 0 != pChild);
}

// _______________________________________________________________________________________________

  bool
  QeFullScreenChild::
isFullScreen( ) const
{
    // A full-screen window has no parent.
    // Assume this is the only way to make the child full-screen.
    if ( pChild->isFullScreen( ) ) {
        d_assert( pChild->parent( ) == 0);
        d_assert( pParent != 0);
        return true;
    }

    // Should we confirm child is a child window?
    d_assert( pParent == 0);
    return false;
}

// _______________________________________________________________________________________________

  /* slot */
  bool
  QeFullScreenChild::
doFullScreen( )
{
    // Should this test Q_WS_WIN or Q_OS_WIN32? Or just WIN32, which implies both a (family of) WS and OS.
  # ifndef Q_WS_WIN
    QMessageBox::information( 0,
          QObject::tr( "Full screen not supported"),
          QObject::tr( "Full screen mode is only supported on Microsoft Windows. Press OK to close."));
    return false;
  # endif

    // Make sure we're not already full-screen.
    if ( isFullScreen( ) ) return false;

    // We need to save the parent while we are full screen.
    // Remember pParent may be a child window itself.
    pParent = pChild->parentWidget( );
    if ( ! pParent ) return false;

    // Find out if we have a layout object. This might return zero.
    pLayout = findLayoutHoldingChild( pParent, pChild);

    // This doesn't work if we have a layout that contains more than one layout item.
    // In that case when we remove pChild from pLayout we have no reliable way to put it back
    // in the same place when we're done with full-screen.
    if ( pLayout && (pLayout->count( ) != 1) ) {
        d_assert( pLayout->count( ) > 1);

        // Something we could try: Disable the layout object but do not remove pChild?
        d_assert( false);
        pParent = 0;
        return false;
    }

    // Remember the window style flags so we can restore them when we leave full-screen.
    childRestoreFlags = pChild->windowFlags( );
    // Remember the rectangle. This isn't necessary if the child has a layout.
    childRestoreRect  = pChild->geometry( );

    // Remember what the parent window looked like before full-screen.
    // Only a top-level pParent can be minimized or maximized.
    parentRestoreState =
        (pParent->isHidden(     ) ? e_hidden      :
        (pParent->isMinimized(  ) ? e_minimized   :
        (pParent->isMaximized(  ) ? e_maximized   :
        (pParent->isFullScreen( ) ? e_full_screen : e_normal))));

    // Hide the parent window and the widget inside.
    // Don't bother if this window is already hidden or minimized.
    if ( (e_hidden != parentRestoreState) && (e_minimized != parentRestoreState) ) {

        // If pWindow->isMaximized( ) it true and you pWindow->hide( ) you lose the "normal"
        // geometry stored in the window. So if you later show the window the normal geometry will
        // have become the same as the maximized geometry.
        // I think this is only true for top-level windows.
        // This is not true if the window is showing "normal" instead of maximized when ->hide( )
        // is called. I don't know what happens if the window isMinimized( ) or isFullScreen( ).
        // This is a Windows (Vista) problem.
        // One solution is to set the "normal" geometry after you hide. normalGeometry( ) is only
        // defined for top-level windows.
        //   QRect const parent_restore_normal_geometry = normalGeometry( );
        //   hide( );
        //   setGeometry( parent_restore_normal_geometry);

        if ( pParent->parent( ) ) {
            // pParent is itself a child window so hide it instead of minimizing.
            d_assert( e_normal == parentRestoreState);
            pParent->hide( );
        }
        else {
            // pParent is a top-level window. Minimize it instead of hiding it.
            pParent->showMinimized( );
        }
    }

    // Hide pChild. This is probably not necessary, but we don't want it flashing alive
    // when we remove it from the parent and before we full-screen.
    pChild->hide( );

    // Take pChild out of the layout for this window.
    if ( pLayout ) {
        pLayout->removeWidget( pChild);
    }

    // Change the widget from a child window to a top-level window.
    // Should we also set the Qt::Popup style bit?
    pChild->setParent( 0, Qt::Window);

    // Show pChild (which is now a top-level window) full-screen.
    // This makes the (full-screen) window visible so we don't have to also pChild->show( ).
    //
    // This does not create a new widget, but it does replace the GL context with a new one.
    // The new context inherits (shares) from the old one, which appears to mean that it can
    // use the compiled-programs from the old context even after the old context is deleted (which
    // it is soon after). It looks like other stuff is not inherited, like textures, lights,
    // settings, matrices, etc. The replacement happens in <qgl.cpp> in QGLWidget::event(QEvent *e)
    // in code that looks like this (Qt 4.4):
    //
    //    if (e->type() == QEvent::ParentChange) {
    //        QGLContext *newContext = new QGLContext(d->glcx->requestedFormat(), this);
    //        qgl_share_reg()->replaceShare(d->glcx, newContext);
    //        setContext(newContext);
    //        // the overlay needs to be recreated as well
    //        delete d->olcx;
    //
    pChild->showFullScreen( );
    d_assert( pChild->isVisible( ));

    d_assert( pChild->parent( ) == 0);
    d_assert( pChild->isFullScreen( ));
    return true;
}

// _______________________________________________________________________________________________

  /* slot */
  bool
  QeFullScreenChild::
undoFullScreen( )
{
    // Do nothing if we're not fullscreen.
    if ( ! isFullScreen( ) ) return false;

    // pParent must be set.
    if ( ! pParent ) return false;

    // Hide pChild (which is now full-screen) to avoid flashes. We still get some flashing though...
    pChild->hide( );

    // Hide pParent if necessary. Although it should already be hidden or minimized.
    if ( (! pParent->isHidden( )) && (! pParent->isMinimized( )) ) {
        if ( pParent->parent( ) ) {
            pParent->hide( );
        }
        else {
            pParent->showMinimized( );
        }
    }

    // Attach pChild and pParent like before.
    pChild->setParent( pParent, childRestoreFlags | Qt::Widget);
    pChild->setGeometry( childRestoreRect);

    // Put pChild back in the layout. This will size it correctly.
    if ( pLayout ) {
        pLayout->addWidget( pChild);
    }

    // Make the (now child) widget visible inside its parent. But the parent is still invisible or minimized.
    pChild->show( );

    // Restore this parent window to how it was before.
    switch ( parentRestoreState ) {
      case e_hidden      : /* do nothing */            break;
      case e_minimized   : /* ->showMinimized( ) */    break;
      case e_maximized   : pParent->showMaximized(  ); break;
      case e_full_screen : pParent->showFullScreen( ); break;
      default:
        d_assert( parentRestoreState == e_normal);
        pParent->showNormal( );
        break;
    }

    // I have seen this fail, although it's rare.
    // Maybe the flags seem to take some time to get themselves caught up?
    //d_assert( ! pChild->isFullScreen( ));

    // Success -- return true
    pParent = 0;
    return true;
}

// _______________________________________________________________________________________________

  /* slot */
  bool
  QeFullScreenChild::
toggleFullScreen( )
{
    return isFullScreen( ) ? undoFullScreen( ) : doFullScreen( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// full_screen.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
