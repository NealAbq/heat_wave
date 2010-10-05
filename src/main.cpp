// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// main.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Notes:
//   Set QT_ASCII_CAST_WARNINGS.
//   Wrap all raw strings:
//     QLatin1String(..)
//     QString::fromAscii(..)
//     QString::fromLatin1(..)
//     QString::fromUtf8(..)
//     QString::fromLocal8Bit(..)
//     QObject::tr(..)
//     QObject::trUtf8(..)
//     QTextCodec::codecForLocale( )
// _______________________________________________________________________________________________

# include "all.h"
# include "heat_simd.h"

# include <QtGui/QApplication>
# include <QtGui/QMessageBox>
# include <QtOpenGL/QGLFormat>

// _______________________________________________________________________________________________

  int
main( int argc, char *argv[ ])
{
    // Prove that assert(..)s are compiled away if NDEBUG is on.
    # ifdef NDEBUG
    d_assert( false);
    # endif

    // QApplication::CustomColor may be a better choice here.
    // Should we even bother to test on 8-bit color (index) systems (with a palette)? Probably not.
    QApplication::setColorSpec( QApplication::ManyColor);
    QApplication app( argc, argv);

    // Make sure we can use OpenGL.
    if ( ! QGLFormat::hasOpenGL( ) ) {
        // Should we use one of these here, in addition to the message box?
        //   qWarning( "No OpenGL support. Exiting.");
        //   qCritical(..)
        //   qFatal(..)
        QMessageBox::information( 0,
          QObject::tr( "Missing OpenGL Support"),
          QObject::tr( "Cannot run because system does not support OpenGL. Press OK to close."));
        return -1;
    }

    // Create the main window.
    heat_wave_main_window_type main_window;

    // These are client sizes, not including the window frame.
    // If we don't set this the min height will be determined by the layouts in the side-tabs.
    main_window.setMinimumHeight( 100);

    // We could set the window size here.
    // main_window.resize( 1280, 720);

    // Show the main window.
    main_window.show( );

    // Run the main window.
    // This opens the window as a modal dialog. The default is application-modal, so this
    // would not work if this app had any other top-level windows.
    // Window modal is also possible. See setWindowModality( ).
    return app.exec( );

    // To make this a modeless dialog consider the following example (from the Qt doc ro QDialog):
    //     findDialog = new FindDialog( p_parent);
    //     connect(findDialog, SIGNAL( findNext( )), p_parent, SLOT( findNext( )));
    //     findDialog->show( );
    //     findDialog->raise( );
    //     findDialog->activateWindow( );
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// main.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
