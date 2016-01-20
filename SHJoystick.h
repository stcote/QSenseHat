//******************************************************************************
//******************************************************************************
//
// SHJoystick
//
// Implements an interface to the Sense Hat Joystick
//
//  Steve Cote 2016
//
//******************************************************************************
//******************************************************************************

#ifndef SHJOYSTICK_H
#define SHJOYSTICK_H

#include <QObject>
#include <QThread>

enum Joystick_Event { JS_ENTER, JS_LEFT, JS_RIGHT, JS_UP, JS_DOWN };


//******************************************************************************
//******************************************************************************
/**
 * @brief The JsThread class
 */
//******************************************************************************
class JsThread : public QThread
{
    Q_OBJECT

public:

    JsThread( int jsFd, QObject *parent );

signals:

    void joystickEvent( Joystick_Event jEv );

private:

    //*** override this for the actual thread code ***
    void run();

    //*** get and process events ***
    void handleEvents();

    //*** joystick device file descriptor ***
    int jsFd_;

};



//******************************************************************************
//******************************************************************************
/**
 * @brief The SHJoystick class
 */
//******************************************************************************
class SHJoystick : public QObject
{
    Q_OBJECT

public:

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief SHJoystick - Constructor
     * @param parent
     */
    //******************************************************************************
    explicit SHJoystick(QObject *parent = 0);


    //******************************************************************************
    //******************************************************************************
    /**
     * @brief ~SHJoystick - destructor
     */
    //******************************************************************************
    virtual ~SHJoystick();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief ready - indicates device is ready
     * @return - true if device is ready for use
     */
    //******************************************************************************
    bool ready() { return ready_; }


signals:

    //*** error signal ***
    void error( QString errStr );

    //*** joystick events ***
    void joystickEvent( Joystick_Event jEv );


protected:

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief findJsDevice - finds the correct input device
     * @return - the file descriptor for the device, or INVALID_DEV on error
     */
    //******************************************************************************
    int findJsDevice();


    //*** file descriptor for the joystick input device ***
    int jsFd_;

    //*** joystick thread ***
    JsThread *jsThread_;

    //*** indicates that the device is ready for use ***
    bool ready_;

};

#endif // SHJOYSTICK_H
