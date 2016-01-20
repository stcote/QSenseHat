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

#ifndef QSENSEHAT_H
#define QSENSEHAT_H

//*** includes ***
#include <QObject>
#include "qsensehat_global.h"

#include "SHLedMatrix.h"
#include "SHJoystick.h"
#include "SHSensors.h"


//******************************************************************************
//******************************************************************************
/**
 * @brief The QSenseHat class
 */
//******************************************************************************
class QSENSEHATSHARED_EXPORT QSenseHat : public QObject
{
    Q_OBJECT

public:


    //******************************************************************************
    //******************************************************************************
    /**
     * @brief getInstance - gets or creates a singleton instance of the
     *              QSenseHat object.
     * @return - pointer to the singlton QSenseHat instance
     */
    //******************************************************************************
    static QSenseHat *getInstance();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief display - reference to display object
     * @return reference to display object
     */
    //******************************************************************************
    SHLedMatrix &display() { return ledMatrix_; }
    const SHLedMatrix *displayP() const { return &ledMatrix_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief joystick - reference to the joystick object
     * @return reference to the joystick object
     */
    //******************************************************************************
    SHJoystick &joystick() { return joystick_; }
    const SHJoystick *joystickP() const { return &joystick_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief sensors - reference to the sensors object
     * @return reference to the sensors object
     */
    //******************************************************************************
    SHSensors &sensors() { return sensors_; }
    const SHSensors *sensorsP() const { return &sensors_; }


signals:

    void error( QString errStr );


protected:

    //*** LED matrix display ***
    SHLedMatrix ledMatrix_;

    //*** joystick ***
    SHJoystick joystick_;

    //*** sensors ***
    SHSensors sensors_;

private:

    //*** singleton ***
    //******************************************************************************
    //******************************************************************************
    /**
     * @brief QSenseHat - constructor
     * @param parent - object parent
     */
    //******************************************************************************
    explicit QSenseHat( QObject *parent = 0 );

    //*** singleton instance ***
    static QSenseHat *senseHatInstance_;

    //*** disable copy constructors ***
    Q_DISABLE_COPY( QSenseHat )

};

#endif // QSENSEHAT_H
