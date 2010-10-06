// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// full_screen.h
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
