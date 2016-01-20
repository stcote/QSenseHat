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

#include "SHJoystick.h"
#include <QtCore>

#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/fb.h>
#include <poll.h>

//*** constants ***
const int INVALID_DEV = -1;

const QString DEV_INPUT_EVENT_DIR = "/dev/input";
const QString EVENT_DEV_PRENAME = "event";
const QString JOYSTICK_NAME = "Raspberry Pi Sense HAT Joystick";


//******************************************************************************
//******************************************************************************
/**
 * @brief SHJoystick::SHJoystick
 * @param parent
 */
//******************************************************************************
SHJoystick::SHJoystick( QObject *parent ) :
    QObject(parent)
{
    //*** initialize vars ***
    ready_ = false;
    jsThread_ = 0;

    //*** register the signal parameter metatype ***
    qRegisterMetaType<Joystick_Event>("Joystick_Event");

    //*** set up joystick access ***
    jsFd_ = findJsDevice();

    //*** continue if found ***
    if ( jsFd_ != INVALID_DEV )
    {
        qDebug() << "Joystick Device found";

        //*** create the joystick thread ***
        jsThread_ = new JsThread( jsFd_, this );

        //*** connect thread signal to outside ***
        connect( jsThread_, SIGNAL(joystickEvent(Joystick_Event)), SIGNAL(joystickEvent(Joystick_Event)) );

        //*** start joystick thread ***
        jsThread_->start();

        //*** we are ready ***
        ready_ = true;
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief SHJoystick::~SHJoystick - destructor
 */
//******************************************************************************
SHJoystick::~SHJoystick()
{
    //*** close input device ***
    if ( jsFd_ != INVALID_DEV )
    {
        ::close( jsFd_ );
    }

    //*** is the thread object valid? ***
    if ( jsThread_ )
    {
        //*** is it running? ***
        if ( jsThread_->isRunning() )
        {
            //*** request it to terminate ***
            jsThread_->requestInterruption();

            //*** wait for it to terminate ***
            jsThread_->wait();
        }

        //*** delete the thread ***
        delete jsThread_;
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief findJsDevice - finds the correct input device
 * @return - the file descriptor for the device, or INVALID_DEV on error
 */
//******************************************************************************
int SHJoystick::findJsDevice()
{
QDir eDir(DEV_INPUT_EVENT_DIR);
QStringList files;
QStringList filters;
int fd = -1;
char name[256];

    //*** event file filter ***
    filters << EVENT_DEV_PRENAME + "*";

    //*** get list of all event device files ***
    files = eDir.entryList( filters, QDir::System );

    //*** check each one till we find the right one ***
    foreach( QString s, files )
    {
        //*** create full path name ***
        QString fullPath = DEV_INPUT_EVENT_DIR + QDir::separator() + s;

        //*** open the device ***
        if ( (fd = open( qPrintable(fullPath), O_RDONLY )) < 0 )
        {
            continue;
        }

        //***  get the input event name ***
        ioctl( fd, EVIOCGNAME(sizeof(name)), name );

        //*** copy to QString for easier comparison ***
        QString eName = name;

        //*** check if we found it ***
        if ( eName == JOYSTICK_NAME )
        {
            //*** return file descriptor for input device ***
            return fd;
        }

        //*** not it, close file ***
        ::close(fd);
    }

    emit error( QString("Joystick: Could not find joystick device!!!") );

    //*** not found ***
    return INVALID_DEV;
}


//******************************************************************************
//******************************************************************************
//
// Joystick thread
//
//******************************************************************************
//******************************************************************************

//******************************************************************************
//******************************************************************************
/**
 * @brief JsThread::JsThread
 * @param jsFd
 * @param parent
 */
//******************************************************************************
JsThread::JsThread( int jsFd, QObject *parent )
    : QThread( parent )
{
    jsFd_ = jsFd;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief JsThread::run
 */
//******************************************************************************
void JsThread::run()
{
struct pollfd evPoll;           // polling structure
int pollRes = 0;                // poll result
const int NumFDs = 1;           // Num file descriptors in poll
const int PollTimeoutMs = 1000; // poll timeout in msecs

    //*** set file descriptor in poll struct ***
    evPoll.fd = jsFd_;

    //*** set poll type ***
    evPoll.events = POLLIN;

    //*** wait for an event to happen (while not terminated) ***
    while( !isInterruptionRequested() )
    {
        //*** wait up to a second for an event ***
        pollRes = poll( &evPoll, NumFDs, PollTimeoutMs );

        //*** was there an event??? ***
        if ( pollRes > 0 )
        {
            //*** handle the event(s) ***
            handleEvents();
        }
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief JsThread::handleEvents
 */
//******************************************************************************
void JsThread::handleEvents()
{
const int MaxEvents = 64;                           // Max # events to read
struct input_event ev[MaxEvents];                   // array of potential events
int i=0;                                            // loop index
int bytesRead = 0;                                  // bytes read from device
const int EventSize = sizeof(struct input_event);   // size of a single event
int numEvents = 0;                                  // # events read

    //*** read the input device ***
    bytesRead = read( jsFd_, ev, EventSize * MaxEvents );

    //*** make sure we have at least one event ***
    if ( bytesRead < EventSize )
    {
        return;
    }

    //*** process all events received ***
    numEvents = bytesRead / EventSize;
    for ( i=0; i<numEvents; i++ )
    {
        //*** only handle key events ***
        if ( ev[i].type != EV_KEY ) continue;

        //*** not sure what this is for ***
        if ( ev[i].value != 1 ) continue;

        switch( ev[i].code )
        {
            case KEY_ENTER:  emit joystickEvent( JS_ENTER );   break;
            case KEY_UP:     emit joystickEvent( JS_UP );      break;
            case KEY_DOWN:   emit joystickEvent( JS_DOWN );    break;
            case KEY_LEFT:   emit joystickEvent( JS_LEFT );    break;
            case KEY_RIGHT:  emit joystickEvent( JS_RIGHT );   break;
            default: break;
        }
    }
}
