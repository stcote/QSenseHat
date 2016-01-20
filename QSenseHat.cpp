//******************************************************************************
//******************************************************************************
//
// Qt Sense Hat library
//
// Implements interfaces to all of the functionality on the Raspberry Pi
//      Sense Hat. This class aggregates the component classes
//
//  Steve Cote 2016
//
//******************************************************************************
//******************************************************************************

#include "QSenseHat.h"


//*** initialize the singleton instance ***
QSenseHat* QSenseHat::senseHatInstance_ = NULL;


//******************************************************************************
//******************************************************************************
/**
 * @brief QSenseHat::QSenseHat - constructor
 * @param parent - parent object
 */
//******************************************************************************
QSenseHat::QSenseHat( QObject *parent ) :
    QObject(parent)
{
    //*** connect error signals to main signal ***
    connect( &ledMatrix_, SIGNAL(error(QString)), SIGNAL(error(QString)) );
    connect( &joystick_,  SIGNAL(error(QString)), SIGNAL(error(QString)) );
    connect( &sensors_,   SIGNAL(error(QString)), SIGNAL(error(QString)) );
}


//******************************************************************************
//******************************************************************************
/**
 * @brief getInstance - gets or creates a singleton instance of the
 *              QSenseHat object.
 * @return - pointer to the singlton QSenseHat instance
 */
//******************************************************************************
QSenseHat *QSenseHat::getInstance()
{
    //*** create instance if it doesn't exist ***
    if ( senseHatInstance_ == NULL )
    {
        senseHatInstance_ = new QSenseHat();
    }

    //*** return the singleton instance ***
    return senseHatInstance_;
}

