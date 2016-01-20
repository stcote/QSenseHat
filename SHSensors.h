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
#ifndef SHSENSORS_H
#define SHSENSORS_H

#include <QObject>
#include <QTimer>

#include "RTIMULib.h"

//*** used in enableSensors() call ***
typedef enum
{
    IMU_PRESSURE = 0x01,
    IMU_HUMIDITY = 0x02,
    IMU_GYRO     = 0x04,
    IMU_ACCEL    = 0x10,
    IMU_COMPASS  = 0x20,
    IMU_TEMP     = 0x40,
    IMU_ALL      = 0x7F
} ImuSensors;

//******************************************************************************
//******************************************************************************
/**
 * @brief The SHSensors class
 */
//******************************************************************************
class SHSensors : public QObject
{
    Q_OBJECT

public:

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief SHSensors - Constructor
     * @param parent
     */
    //******************************************************************************
    explicit SHSensors( QObject *parent = 0 );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief ~SHSensors - Destructor
     */
    //******************************************************************************
    virtual ~SHSensors();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief isReady - indicates the IMU is found and ready
     * @return - TRUE if ready else FALSE
     */
    //******************************************************************************
    bool isReady() { return ready_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief isStarted - indicates readings have started
     * @return - TRUE if started, else FALSE
     */
    //******************************************************************************
    bool isStarted() { return started_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief enableSensors - enable the specified sensors. Use the ImuSensors enums
     *              CALL THIS BEFORE START
     * @param sensorsEnabled - 'OR' of all sensors to be enabled (or IMU_ALL)
     * @return TRUE if sensors enabled. FALSE if error
     */
    //******************************************************************************
    bool enableSensors( quint8 sensorsEnabled );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setUpdateRate - set sensor data update rate
     * @param updatesPerSec
     */
    //******************************************************************************
    void setUpdateRate( quint16 updatesPerSec );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief startPeriodicUpdates - start periodic update of sensor data via signals
     * @return - TRUE if started OK, else FALSE
     */
    //******************************************************************************
    bool startPeriodicUpdates();


signals:

    //*** error signal ***
    void error( QString errStr );

    //*** IMU data ***
    void pressure( float pres_hPa, float altitudeM );
    void temperature( float tempC, float tempF );
    void humidity( float rh );
    void gyro( float x_degPerSec, float y_degPerSec, float z_degPerSec );
    void accel( float x_g, float y_g, float z_g, float mag_g );
    void compass( float x_uT, float y_uT, float z_uT, float mag );
    void fusionPose( float rollDeg, float pitchDeg, float yawDeg );


protected slots:

    void handleUpdate();


protected:

    //*** pointer to IMU object ***
    RTIMU *imu_;

    //*** settings object ***
    RTIMUSettings *settings_;

    //*** pressure and humidity objects ***
    RTPressure *pressure_;
    RTHumidity *humidity_;

    //*** enabled sensors ***
    quint8 enabled_;

    //*** indicates IMU is ready for readings ***
    bool ready_;

    //*** indicates taking readings ***
    bool started_;

    //*** indicates we have a valid IMU ***
    bool validIMU_;

    //*** update timer ***
    QTimer *imuTimer_;

    //*** update interval ***
    int updateIntervalMSec_;


};

#endif // SHSENSORS_H
