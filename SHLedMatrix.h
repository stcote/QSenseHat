//******************************************************************************
//******************************************************************************
//
// Sense Hat LED Matrix display class
//
// Initializes sense hat LED matrix by memory mapping the framebuffer device.
//      Implements methods to set individual pixels or lines
//
// upper left hand corner is { 0, 0 }
//
//  Steve Cote 2016
//
//******************************************************************************
//******************************************************************************

#ifndef SHLEDMATRIX_H
#define SHLEDMATRIX_H

#include <QObject>
#include <QtCore>
#include <QMutex>
#include <QMutexLocker>
#include <QImage>
#include <QFont>
#include <QPainter>
#include <QTimer>

//*** set up known values for display - 8x8 matrix ***

const int DisplayXSize = 8;
const int DisplayYSize = 8;

const int DisplayBitsPerPixel = 16;
const int DisplayBytesPerPixel = DisplayBitsPerPixel / 8;

const int DisplayLineLenBytes = DisplayXSize * DisplayBytesPerPixel;

const int DisplayMemSizeBytes = DisplayXSize * DisplayYSize * DisplayBytesPerPixel;

//*** Amount of rotation ***
enum BufRotate { ROT_90, ROT_180, ROT_270 };

//*** reverse row or column ***
enum RevDir { REV_ROW, REV_COL };

//*** 16 bit color info ***
const quint8 MaxRed = 31;
const quint16 RedMask = 0x001F;
#define RedShift (11)
const quint8 MaxGreen = 63;
const quint16 GreenMask = 0x003F;
#define GreenShift (5)
const quint8 MaxBlue = 31;
const quint16 BlueMask = 0x001F;
#define BlueShift (0)

//******************************************************************************
//******************************************************************************
/**
 * @brief The Color16b class - a class to represent 16 bit color values
 */
//******************************************************************************
class Color16b
{
public:

    //*** constructors ***
    Color16b()              { col_ = 0;   }
    Color16b( quint16 col ) { col_ = col; }
    Color16b( quint8 red, quint8 green, quint8 blue )
        { setColor( red, green, blue ); }

    void setColor( quint8 red, quint8 green, quint8 blue  )
        { col_ = getColorValue( red, green, blue ); }

    static quint16 getColorValue( quint8 red, quint8 green, quint8 blue )
        { return (quint16)(((red & RedMask) << RedShift ) |
                           ((green & GreenMask) << GreenShift ) |
                            (blue & BlueMask)); }

    quint16 colorVal() { return col_; }

    quint8 redVal()   { return (quint8)((col_ >> RedShift))   & RedMask; }
    quint8 greenVal() { return (quint8)((col_ >> GreenShift)) & GreenMask; }
    quint8 blueVal()  { return (quint8)((col_ >> RedShift))   & BlueMask; }

    static quint16 aqua()    { return getColorValue(  0, 63, 31 ); }
    static quint16 black()   { return getColorValue(  0,  0,  0 ); }
    static quint16 blue()    { return getColorValue(  0,  0, 31 ); }
    static quint16 fuschia() { return getColorValue( 31,  0, 31 ); }
    static quint16 gray()    { return getColorValue( 16, 32, 16 ); }
    static quint16 green()   { return getColorValue(  0, 32,  0 ); }
    static quint16 lime()    { return getColorValue(  0, 63,  0 ); }
    static quint16 maroon()  { return getColorValue( 16,  0,  0 ); }
    static quint16 navy()    { return getColorValue(  0,  0, 16 ); }
    static quint16 purple()  { return getColorValue( 16,  0, 16 ); }
    static quint16 red()     { return getColorValue( 31,  0,  0 ); }
    static quint16 silver()  { return getColorValue( 24, 48, 24 ); }
    static quint16 teal()    { return getColorValue(  0, 32, 16 ); }
    static quint16 olive()   { return getColorValue( 16, 32,  0 ); }
    static quint16 white()   { return getColorValue( 31, 63, 31 ); }
    static quint16 yellow()  { return getColorValue( 31, 63,  0 ); }

private:

    quint16 col_;
};



//******************************************************************************
//******************************************************************************
/**
 * @brief The SHLedMatrix class
 */
//******************************************************************************
class SHLedMatrix : public QObject
{
    Q_OBJECT
public:

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief SHLedMatrix - constructor
     * @param parent
     */
    //******************************************************************************
    explicit SHLedMatrix( QObject *parent = 0 );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief ~SHLedMatrix - destructor
     */
    //******************************************************************************
    virtual ~SHLedMatrix();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief ready - indicates matrix is ready for output
     */
    //******************************************************************************
    bool ready() { return ready_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief xResolution - returns the X resolution
     * @return - x resolution or 0 if not ready
     */
    //******************************************************************************
    int xResolution() { return DisplayXSize; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief yResolution - returns the Y resolution
     * @return - y resolution or 0 if not ready
     */
    //******************************************************************************
    int yResolution() { return DisplayYSize; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief lastError - returns the last error
     * @return - the last error encountered
     */
    //******************************************************************************
    QString lastError() { return lastError_; }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief clear - clears the display
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool clear();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setPixel - sets the pixel value for the {x,y} location on the LED matrix
     * @param x     - x position (0 based)
     * @param y     - y position (0 based)
     * @param color - 16 bit color value
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool setPixel( int x, int y, quint16 color );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief drawHLine - draws a horizontal line
     * @param x1    - starting x coordinate (inclusive)
     * @param x2    - ending x coordinate (inclusive)
     * @param y     - y coordinate of line
     * @param color - 16 bit color value
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool drawHLine( int x1, int x2, int y, quint16 color );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief drawVLine - draws a vertical line
     * @param x     - x coordinate of line
     * @param y1    - starting y coordinate (inclusive)
     * @param y2    - ending y coordinate (inclusive)
     * @param color - 16 bit color value
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool drawVLine( int x, int y1, int y2, quint16 color );

   //******************************************************************************
    //******************************************************************************
    /**
     * @brief fill - fills the display with the given color
     * @param fillColor - color to fill with
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool fill( quint16 fillColor );
    bool fill( Color16b fillColor ) { return fill( fillColor.colorVal() ); }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setMatrix - sets the complete memory mapped buffer
     * @param buffer - pointer to an 8x8 quint16 array
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool setMatrix( quint16 *buffer );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief kaleidoscope - given a 4x4 array of pixel value which is copied to the
     *              upper left corner, the array is rotated and copied to the other
     *              3 quadrants
     * @param buf4x4 - pointer to a 4x4 array of pixel values
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool kaleidoscope( quint16 *buf4x4 );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setBlock - copy a block of pixel data to a section of the display
     * @param srcBuf - pointer to an array of pixel data
     * @param xSize  - x size of pixel array
     * @param ySize  - y size of pixel array
     * @param destX  - x coordinate of upper left corner of destination
     * @param destY  - y coordinate of upper left corner of destination
     * @return - true if everything OK, else false
     */
    //******************************************************************************
    bool setBlock( quint16 *srcBuf, int xSize, int ySize, int destX, int destY );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief rotateBuffer - utility to rotate a rectangular buffer - this version
     *              puts the rotated matrix into a separate buffer
     * @param src   - source buffer
     * @param dest  - destination (rotated) buffer
     * @param blockSize - x and y size of source buffer
     * @param rot   - amount to rotate
     */
    //******************************************************************************
    void rotateBuffer( quint16 *src, quint16 *dest, int blockSize, BufRotate rot );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief rotateBuffer - utility to rotate a rectangular buffer - this version
     *              rotates the matrix in place
     * @param src   - buffer to rotate
     * @param blockSize - x and y size of source buffer
     * @param rot   - amount to rotate
     */
    //******************************************************************************
    void rotateBuffer( quint16 *src, int blockSize, BufRotate rot );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief getImage - returns a QImage of the right size and format
     * @return pointer to new QImage. Must be deleted after use
     */
    //******************************************************************************
    QImage *getImage() { return new QImage( DisplayXSize, DisplayYSize, QImage::Format_RGB16 ); }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setImage - sets the display to the QImage pixel data. The QImage should
     *              have been attained through a call to getImage()
     * @param image - image to display
     * @param xOffset - x offset into the image
     * @param yOffset - y offset into the image
     * @return - TRUE if success else FALSE
     */
    //******************************************************************************
    bool setImage( QImage* image, quint16 xOffset = 0, quint16 yOffset=0 );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief scrollText - scrools the given text across the display
     * @param txt          - the text to scroll
     * @param pixelsPerSec - the speed of the scroll in pixels per second
     * @return - TRUE if succesful, else FALSE
     */
    //******************************************************************************
    bool scrollText( QString txt, quint8 pixelsPerSec=15 );
    bool scrollText( const char *txt, quint8 pixelsPerSec=15 )
        { return scrollText( QString(txt), pixelsPerSec ); }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setTextFamily - sets the family for the scrolling text font
     * @param family - font family name ( "Times", "Courier", etc )
     */
    //******************************************************************************
    void setTextFamily( QString family ) { txtFont_.setFamily( family ); }

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setTextPointSize - sets the point size for the scrolling text
     * @param pSize - font point size
     */
    //******************************************************************************
    void setTextPointSize( int pSize ) { txtFont_.setPointSize( pSize ); }

signals:

    //*** error signal ***
    void error( QString errStr );


public slots:

protected slots:

    void handleScrollText();


protected:

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief findFbDevice - finds the correct framebuffer device
     * @return - the fd to the device, or -1 on error
     */
    //******************************************************************************
    int findFbDevice();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief setupFbDevice - initializes the device by setting up a memory map
     */
    //******************************************************************************
    void setupFbDevice();

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief locationFromCoordinates - determines the memory location of the
     *              given {x,y} coordinate
     * @param x - x coordinate
     * @param y - y coordinate
     * @return location in framebuffer
     */
    //******************************************************************************
    long locationFromCoordinates( int x, int y );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief transpose - transpose the square matrix in place
     * @param matrix - pointer to the matrix
     * @param mSize  - x and y dimensions of the matrix
     */
    //******************************************************************************
    void transpose( quint16 *matrix, int mSize );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief reverse - reverse the rows/columns of a matrix
     * @param mat    - pointer to matrix
     * @param mSize  - x and y dimensions of matrix
     * @param revDir - reverse rows or columns
     */
    //******************************************************************************
    void reverse( quint16* mat, int mSize, RevDir revDir );

    //******************************************************************************
    //******************************************************************************
    /**
     * @brief dumpImage - dumps image values to debug
     * @param image - image to dump
     */
    //******************************************************************************
    void dumpImage( QImage *image );


    //*** file descriptor of frame buffer device ***
    int fbFd_;

    //*** indicates device is fully set up and ready for output ***
    bool ready_;

    //*** holds the last encountered error ***
    QString lastError_;

    //*** thread safety ***
    QMutex accessMutex_;

    //*** text font ***
    QFont txtFont_;

    //*** text info ***
    QImage *txtImg_;            // the image of the text
    int txtLen_;                // the width of the text image
    int curTxtOffset_;          // current offset into image
    QPainter painter_;          // painter to draw into text image
    bool isScrollingText_;      // indicates currently scrolling text
    QTimer *txtTimer_;          // timer for scrolling

    //*** framebuffer info ***
    quint16 *fbPtr_;            // pointer to memory mapped framebuffer
    bool validFbPtr_;           // indicates memory map pointer is valid

};

#endif // SHLEDMATRIX_H
