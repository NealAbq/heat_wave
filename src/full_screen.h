// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// full_screen.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef FULL_SCREEN_H
# define FULL_SCREEN_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include <QtCore/QRect>
# include <QtGui/QWidget>
# include <QtGui/QLayout>

// _______________________________________________________________________________________________

  class
QeFullScreenChild
  : public QObject
{
  Q_OBJECT
  public:
    /* ctor */      QeFullScreenChild( QWidget * pChild)        ;

  public:
    bool            isFullScreen( )                       const ;

  public slots:
    bool            doFullScreen( )                             ;
    bool            undoFullScreen( )                           ;
    bool            toggleFullScreen( )                         ;

  signals:
    void            fullScreenDone( )                           ;
    void            fullScreenUndone( )                         ;

  private:
    // This class is widget oriented.
    QWidget    * const  pChild             ;

    // These are only set while we are in full-screen mode.
    QWidget    *        pParent            ;
    QLayout    *        pLayout            ;

    // Flags are set while in full-screen mode.
    Qt::WindowFlags     childRestoreFlags  ;
    QRect               childRestoreRect   ;

    enum {
        e_hidden
      , e_normal
      , e_minimized
      , e_maximized
      , e_full_screen
    }                   parentRestoreState ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef FULL_SCREEN_H
//
// full_screen.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
