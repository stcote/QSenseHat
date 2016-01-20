//******************************************************************************
//******************************************************************************
//
// Sense Hat Sensor class
//
// Initializes the sensors on the SenseHat
//
//  Steve Cote 2016
//
//******************************************************************************
//******************************************************************************
#include "SHSensors.h"
#include <QDebug>


//******************************************************************************
//******************************************************************************
/**
 * @brief SHSensors::SHSensors
 * @param parent
 */
//******************************************************************************
SHSensors::SHSensors(QObject *parent) :
    QObject(parent)
{
    //*** initialize vars ***
    enabled_ = 0;
    ready_ = false;
    validIMU_ = false;
    started_ = false;
    imu_ = 0;
    pressure_ = 0;
    humidity_ = 0;
    imuTimer_ = 0;
    updateIntervalMSec_ = 200;

    //*** get the settings ***
    settings_ = new RTIMUSettings();

    //*** create an IMU object ***
    imu_ = RTIMU::createIMU( settings_ );

    //*** check for a valid IMU ***
    if ( (imu_ == NULL) || (imu_->IMUType() == RTIMU_TYPE_NULL) )
    {
        qDebug() << "No valid IMU found";
        return;
    }

    qDebug() << "IMU Found";

    //*** set valid flag ***
    validIMU_ = true;

    //*** set up the IMU ***
    imu_->IMUInit();

    //*** not sure what this does yet ***
    imu_->setSlerpPower( 0.02 );

    //*** create timer ***
    imuTimer_ = new QTimer( this );
    connect( imuTimer_, SIGNAL(timeout()), SLOT(handleUpdate()) );
}


//******************************************************************************
//******************************************************************************
/**
 * @brief ~SHSensors - Destructor
 */
//******************************************************************************
SHSensors::~SHSensors()
{
    if ( imuTimer_ )
    {
        if ( imuTimer_->isActive() )
        {
            imuTimer_->stop();
        }

        delete imuTimer_;
    }

    delete settings_;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief enableSensors - enable the specified sensors.
 *              CALL THIS BEFORE START
 * @param sensorsEnabled - 'OR' of all sensors to be enabled (or IMU_ALL)
 */
bool SHSensors::enableSensors( quint8 sensorsEnabled )
{
    //*** must have valid IMU ***
    if ( !validIMU_ )
    {
        emit error( "Sensors: No valid IMU found!!!" );
        return false;
    }

    //*** save the enabled flags ***
    enabled_ = sensorsEnabled;

    //*** pressure ***
    if ( enabled_ & IMU_PRESSURE || enabled_ & IMU_TEMP )
    {
        pressure_ = RTPressure::createPressure( settings_ );
        if ( pressure_ != NULL )
        {
            pressure_->pressureInit();
        }
    }

    //*** humidity ***
    if ( enabled_ & IMU_HUMIDITY )
    {
        humidity_ = RTHumidity::createHumidity( settings_ );
        if ( humidity_ != NULL )
        {
            humidity_->humidityInit();
        }
    }

    //*** gyroscope ***
    if ( enabled_ & IMU_GYRO )
    {
        imu_->setGyroEnable( true );
    }

    //*** accelerometer ***
    if ( enabled_ & IMU_ACCEL )
    {
        imu_->setAccelEnable( true );
    }

    //*** compass ***
    if ( enabled_ & IMU_COMPASS )
    {
        imu_->setCompassEnable( true );
    }

    //*** set ready flag ***
    ready_ = true;

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief setUpdateRate - set sensor data update rate
 * @param updatesPerSec
 */
//******************************************************************************
void SHSensors::setUpdateRate( quint16 updatesPerSec )
{
    //*** must have valid IMU ***
    if ( !validIMU_ )
    {
        emit error( "Sensors: No valid IMU found!!!" );
        return;
    }

    //*** save rate ***
    updateIntervalMSec_ = 1000 / updatesPerSec;

    //*** change if currently started ***
    if ( imuTimer_->isActive() )
    {
        imuTimer_->start( updateIntervalMSec_ );
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief startPeriodicUpdates - start sensors
 * @return - TRUE if started OK, else FALSE
 */
//******************************************************************************
bool SHSensors::startPeriodicUpdates()
{
    if ( !ready_ || started_ ) return false;

    //*** set started flag ***
    started_ = true;

    //*** start the update timer ***
    imuTimer_->start( updateIntervalMSec_ );

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief SHSensors::handleUpdate - handle periodic update
 */
//******************************************************************************
void SHSensors::handleUpdate()
{
const float fToC = 9.0 / 5.0;
const float fOffset = 32;

    //*** read all data ***
    while ( imu_->IMURead() )
    {
        //*** get IMU data ***
        RTIMU_DATA imuData = imu_->getIMUData();

        //*** pressure / temperature ***
        if ( ((enabled_ & IMU_PRESSURE) || (enabled_ & IMU_TEMP)) && pressure_ )
        {
            pressure_->pressureRead( imuData );

            if ( enabled_ & IMU_PRESSURE )
            {
                //*** pressure in hPa ***
                emit pressure( imuData.pressure, RTMath::convertPressureToHeight(imuData.pressure) );
            }

            if ( enabled_ & IMU_TEMP )
            {
                //*** temp celsius, fahrenheit ***
                emit temperature( imuData.temperature,
                                  imuData.temperature * fToC + fOffset );
            }
        }

        //*** humidity ***
        if ( (enabled_ & IMU_HUMIDITY) && humidity_ )
        {
            humidity_->humidityRead( imuData );

            //*** relative humidity ***
            emit humidity( imuData.humidity );
        }

        //*** gyroscope ***
        if ( enabled_ & IMU_GYRO )
        {
            //*** gyroscope in degrees per second ***
            emit gyro( imuData.gyro.x() * RTMATH_RAD_TO_DEGREE,
                       imuData.gyro.y() * RTMATH_RAD_TO_DEGREE,
                       imuData.gyro.z() * RTMATH_RAD_TO_DEGREE );
        }

        //*** accelerometer ***
        if ( enabled_ & IMU_ACCEL )
        {
            //*** acceleration in g's ***'
            emit accel( imuData.accel.x(), imuData.accel.y(),
                        imuData.accel.z(), imuData.accel.length() );
        }

        //*** compass ***
        if ( enabled_ & IMU_COMPASS )
        {
            //*** compas (magnetometer) in uT ***
            emit compass( imuData.compass.x(), imuData.compass.y(),
                          imuData.compass.z(), imuData.compass.length() );
        }

        //*** always emit fusion data - degrees ***
        float fusionX = imuData.fusionPose.x() * RTMATH_RAD_TO_DEGREE;
        float fusionY = imuData.fusionPose.y() * RTMATH_RAD_TO_DEGREE;
        float fusionZ = imuData.fusionPose.z() * RTMATH_RAD_TO_DEGREE;
        emit fusionPose( fusionX, fusionY, fusionZ );
    }

}
