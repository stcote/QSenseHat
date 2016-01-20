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

//*** includes ***
#include "SHLedMatrix.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <QtGui>
#include <QDebug>
#include <QFontMetrics>

//*** constants ***
const int INVALID_FB = -1;

const QString FB_DEV_DIR = "/dev";
const QString FB_DEV_PRENAME = "fb";
const QString FB_NAME = "RPi-Sense FB";


//******************************************************************************
//******************************************************************************
/**
 * @brief SHLedMatrix::SHLedMatrix - constructor
 * @param parent
 */
//******************************************************************************
SHLedMatrix::SHLedMatrix(QObject *parent) :
    QObject(parent)
{
    //*** initialize vars ***
    ready_ = false;
    fbFd_ = INVALID_FB;
    lastError_ = "LED Matrix not initialized yet...";
    validFbPtr_ = false;
    fbPtr_ = 0;
    isScrollingText_ = false;
    curTxtOffset_ = 0;
    txtLen_ = 0;

    //*** set up text font ***
    txtFont_.setFamily( "Helvetica" );
    txtFont_.setPointSize( 8 );

    txtTimer_ = new QTimer( this );
    connect( txtTimer_, SIGNAL(timeout()), SLOT(handleScrollText()) );

    //*** set up framebuffer access ***
    fbFd_ = findFbDevice();

    //*** only continue if found ***
    if ( fbFd_ != INVALID_FB )
    {
        qDebug() << "Framebuffer Device found";

        //*** set up the device (memory map it) ***
        setupFbDevice();
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief ~SHLedMatrix - destructor
 */
//******************************************************************************
SHLedMatrix::~SHLedMatrix()
{
    //*** if framebuffer valid ***
    if ( validFbPtr_ )
    {
        //*** unmap the framebuffer ***
        munmap( fbPtr_, DisplayMemSizeBytes );
    }

    //*** close the device if open ***
    if ( fbFd_ != INVALID_FB )
    {
        //*** close the framebuffer file ***
        close( fbFd_ );
    }

}

//******************************************************************************
//******************************************************************************
/**
 * @brief SHLedMatrix::clear - clears the framebuffer display
 * @return - TRUE if successful, else FALSE
 */
//******************************************************************************
bool SHLedMatrix::clear()
{
QMutexLocker dLock( &accessMutex_ );

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not ready";
        return false;
    }

    //*** clear all framebuffer memory ***
    memset( fbPtr_, 0, DisplayMemSizeBytes );

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief setPixel - sets the pixel value for the {x,y} location on the LED matrix
 * @param x     - x position (0 based)
 * @param y     - y position (0 based)
 * @param color - 16 bit color value
 * @return - true if successful, else false
 */
//******************************************************************************
bool SHLedMatrix::setPixel( int x, int y, quint16 color )
{
QMutexLocker dLock( &accessMutex_ );
bool xValid = false;
bool yValid = false;
long location = 0;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** check parameters ***
    xValid = ( x >= 0 && x < DisplayXSize );
    yValid = ( y >= 0 && y < DisplayYSize );
    if ( !xValid || !yValid )
    {
        if ( !xValid && yValid )
            lastError_ = "Invalid X coordinate";
        else if ( xValid && !yValid )
            lastError_ = "Invalid Y coordinate";
        else
            lastError_ = "Invalid X and Y coordinates";

        return false;
    }

    //*** calculate the location within the framebuffer ***
    location = locationFromCoordinates( x, y );

    //*** set the pixel ***
    *(fbPtr_ + location) = color;

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief drawHLine - draws a horizontal line
 * @param x1    - starting x coordinate (inclusive)
 * @param x2    - ending x coordinate (inclusive)
 * @param y     - y coordinate of line
 * @param color - 16 bit color value
 * @return - true if successful, else false
 */
//******************************************************************************
bool SHLedMatrix::drawHLine( int x1, int x2, int y, quint16 color )
{
QMutexLocker dLock( &accessMutex_ );
bool xValid = false;
bool yValid = false;
long location = 0;
int lineLenPix = 0;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** check parameters ***
    xValid = ( x1 >= 0 && x1 < DisplayXSize && x2 >= 0 && x2 < DisplayXSize );
    yValid = ( y >= 0 && y < DisplayYSize );
    if ( !xValid || !yValid )
    {
        if ( !xValid && yValid )
            lastError_ = "Invalid X coordinate";
        else if ( xValid && !yValid )
            lastError_ = "Invalid Y coordinate";
        else
            lastError_ = "Invalid X and Y coordinates";

        return false;
    }

    //*** swap if ordered wrong ***
    if ( x2 < x1 ) qSwap( x1, x2 );

    //*** calculate the location within the framebuffer ***
    location = locationFromCoordinates( x1, y );

    //*** determine line length in pixels ***
    lineLenPix = x2 - x1 + 1;

    //*** set all the pixels ***
    for ( int i=0; i<lineLenPix; i++ )
    {
        //*** set the pixel ***
        *(fbPtr_ + location + i) = color;
    }

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief drawVLine - draws a vertical line
 * @param x     - x coordinate of line
 * @param y1    - starting y coordinate (inclusive)
 * @param y2    - ending y coordinate (inclusive)
 * @param color - 16 bit color value
 * @return - true if successful, else false
 */
//******************************************************************************
bool SHLedMatrix::drawVLine( int x, int y1, int y2, quint16 color )
{
QMutexLocker dLock( &accessMutex_ );
bool xValid = false;
bool yValid = false;
long location = 0;
int lineLenPix = 0;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** check parameters ***
    xValid = ( x >= 0 && x < DisplayXSize );
    yValid = ( y1 >= 0 && y1 < DisplayYSize && y2 >= 0 && y2 < DisplayYSize );
    if ( !xValid || !yValid )
    {
        if ( !xValid && yValid )
            lastError_ = "Invalid X coordinate";
        else if ( xValid && !yValid )
            lastError_ = "Invalid Y coordinate";
        else
            lastError_ = "Invalid X and Y coordinates";

        return false;
    }

    //*** swap if ordered wrong ***
    if ( y2 < y1 ) qSwap( y1, y2 );

    //*** calculate the location within the framebuffer ***
    location = locationFromCoordinates( x, y1 );

    //*** determine line length in pixels ***
    lineLenPix = y2 - y1 + 1;

    //*** set all the pixels ***
    for ( int i=0; i<lineLenPix; i++ )
    {
        //*** set the pixel ***
        *(fbPtr_ + location + (i * DisplayXSize)) = color;
    }

    return true;
}


//******************************************************************************
 //******************************************************************************
 /**
  * @brief fill - fills the display with the given color
  * @param fillColor - color to fill with
  * @return - true if successful, else false
  */
 //******************************************************************************
bool SHLedMatrix::fill( quint16 fillColor )
{
QMutexLocker dLock( &accessMutex_ );
const int NumPixels = DisplayXSize*DisplayYSize;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** copy color into each memory location ***
    for ( int i=0; i<NumPixels; i++ )
        fbPtr_[i] = fillColor;

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief setMatrix - sets the complete memory mapped buffer
 * @param buffer - pointer to an 8x8 quint16 array
 * @return - true if everything OK, else false
 */
//******************************************************************************
bool SHLedMatrix::setMatrix( quint16 *buffer )
{
QMutexLocker dLock( &accessMutex_ );

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** copy the data ***
    memcpy( fbPtr_, buffer, DisplayMemSizeBytes );

    return true;
}


#define BlockSize (4)
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
bool SHLedMatrix::kaleidoscope( quint16 *buf4x4 )
{
quint16 alt[BlockSize][BlockSize];
quint16 *altP = &(alt[0][0]);

    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** upper left quadrant ***
    setBlock( buf4x4, BlockSize, BlockSize, 0, 0 );

    //*** upper right quadrant ***
    rotateBuffer( buf4x4, altP, BlockSize, ROT_90 );
    setBlock( altP, BlockSize, BlockSize, BlockSize, 0 );

    //*** lower right quadrant ***
    rotateBuffer( buf4x4, altP, BlockSize, ROT_180 );
    setBlock( altP, BlockSize, BlockSize, BlockSize, BlockSize );

    //*** lower left quadrant ***
    rotateBuffer( buf4x4, altP, BlockSize, ROT_270 );
    setBlock( altP, BlockSize, BlockSize, 0, BlockSize );

    return true;
}


//******************************************************************************
//***************************=***************************************************
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
bool SHLedMatrix::setBlock( quint16 *srcBuf, int xSize, int ySize, int destX, int destY )
{
QMutexLocker dLock( &accessMutex_ );
int endX = destX + xSize - 1;
int endY = destY + ySize - 1;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** make sure parameters are valid ***
    if ( destX < 0 || endX < 0 ||
         destX >= DisplayXSize || endX >= DisplayXSize ||
         destY < 0 || endY < 0 ||
         destY >= DisplayYSize || endY >= DisplayYSize )
    {
        lastError_ = "Invalid parameter";
        return false;
    }

    //*** calculate pointer to start of destination ***
    quint16 *destBuf = fbPtr_ + locationFromCoordinates( destX, destY );

    //*** copy all lines ***
    for ( int i=0; i<ySize; i++ )
    {
        //*** copy data ***
        memcpy( destBuf, srcBuf, xSize * sizeof(quint16) );

        //*** adjust pointers ***
        srcBuf  += DisplayXSize;
        destBuf += DisplayYSize;
    }

    return true;
}


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
void SHLedMatrix::rotateBuffer( quint16 *src, quint16 *dest, int blockSize, BufRotate rot )
{
    //*** copy to output buffer ***
    memcpy( dest, src, blockSize * blockSize * sizeof(quint16) );

    //*** use rotate in place version ***
    rotateBuffer( dest, blockSize, rot );
}


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
void SHLedMatrix::rotateBuffer( quint16 *src, int blockSize, BufRotate rot )
{
    //*** process according to amount of rotation ***
    switch( rot )
    {
    case ROT_90:
        transpose( src, blockSize );
        reverse( src, blockSize, REV_COL );
        break;

    case ROT_180:
        reverse( src, blockSize, REV_ROW );
        reverse( src, blockSize, REV_COL );
        break;

    case ROT_270:
        transpose( src, blockSize );
        reverse( src, blockSize, REV_ROW );
        break;
    }
}


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
bool SHLedMatrix::setImage( QImage *image, quint16 xOffset ,quint16 yOffset )
{
QMutexLocker dLock( &accessMutex_ );
quint16 *dispPtr = 0;
quint16 *imgPtr = 0;

    //*** must be ready ***
    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    //*** verify parameters ***
    if ( image->size().width()  < ( DisplayXSize + xOffset ) ||
         image->size().height() < ( DisplayYSize + yOffset ) ||
         image->format() != QImage::Format_RGB16 )
    {
        lastError_ = "Invalid image";
        return false;
    }

    //*** copy all data to buffer ***
    for ( int row=yOffset; row<DisplayYSize+yOffset; row++ )
    {
        dispPtr = fbPtr_ + ( row * DisplayXSize );
        imgPtr = (quint16*)image->scanLine( row );
        memcpy( dispPtr, imgPtr + xOffset, DisplayLineLenBytes );
    }

    return true;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief scrollText - scrools the given text across the display
 * @param txt          - the text to scroll
 * @param pixelsPerSec - the speed of the scroll in pixels per second
 * @return - TRUE if succesful, else FALSE
 */
//******************************************************************************
bool SHLedMatrix::scrollText( QString txt, quint8 pixelsPerSec )
{
QFontMetrics fm( txtFont_ );

    if ( !ready_ )
    {
        lastError_ = "Device not initialized!!!";
        return false;
    }

    if ( isScrollingText_ )
    {
        lastError_ = "Already scrolling text";
        return false;
    }

    //*** delete image from last invocation ***
    if ( txtImg_ )
    {
        delete txtImg_;
    }

    //*** determine width of image needed to display text ***
    txtLen_ = fm.width( txt );

    //*** create image of correct size ***
    txtImg_ = new QImage( txtLen_+1, DisplayYSize, QImage::Format_RGB16 );

    //*** start drawing into image ***
    painter_.begin( txtImg_ );

    //*** draw background ***
    painter_.fillRect( 0, 0, txtLen_+1, DisplayYSize, Qt::black );

    //*** set font to use ***
    painter_.setFont( txtFont_ );

    //*** set the pen color ***
    painter_.setPen( Qt::blue );

    //*** draw text into image ***
    painter_.drawText( 0, DisplayYSize, txt );

    //*** done painting into image ***
    painter_.end();

    //*** initialize offset into image ***
    curTxtOffset_ = 0;

    //*** set flag ***
    isScrollingText_ = true;

    //*** start things off ***
    setImage( txtImg_, curTxtOffset_ );

    //*** calculate scrolling speed (timer period) ***
    int timerPeriodMsec = 1000 / pixelsPerSec;

    //*** start timer at specified interval ***
    txtTimer_->start( timerPeriodMsec );

    return true;
}

//******************************************************************************
//******************************************************************************
/**
 * @brief SHLedMatrix::handleScrollText
 */
//******************************************************************************
void SHLedMatrix::handleScrollText()
{
    //*** if we've finished scrolling, stop ***
    if ( !isScrollingText_ )
    {
        txtTimer_->stop();
        return;
    }

    //*** increment x offset ***
    curTxtOffset_++;

    //*** display next image section ***
    setImage( txtImg_, curTxtOffset_ );

    //*** check if we are done ***
    if ( curTxtOffset_ >= (txtLen_ - DisplayXSize ) )
    {
        isScrollingText_ = false;
        txtTimer_->stop();
    }

}


//******************************************************************************
//******************************************************************************
/**
 * @brief findFbDevice - finds the correct framebuffer device
 * @return - the file descriptor to the device, or -1 on error
 */
//******************************************************************************
int SHLedMatrix::findFbDevice()
{
QDir eDir(FB_DEV_DIR);
QStringList files;
QStringList filters;
int fd = -1;
struct fb_fix_screeninfo fixInfo;

    //*** framebuffer file filter ***
    filters << FB_DEV_PRENAME + "*";

    //*** get list of all framebuffer device files ***
    files = eDir.entryList( filters, QDir::System );

    //*** check each one till we find the right one ***
    foreach( QString s, files )
    {
        //*** create full path name ***
        QString fullPath = FB_DEV_DIR + QDir::separator() + s;

        //*** open the device ***
        if ( (fd = open( qPrintable(fullPath), O_RDWR )) < 0 )
        {
            continue;
        }

        //***  get the framebuffer name ***
        ioctl( fd, FBIOGET_FSCREENINFO, &fixInfo );

        //*** copy to QString for easier comparison ***
        QString eName = fixInfo.id;

        //*** check if we found it ***
        if ( eName == FB_NAME )
        {
            //*** return file descriptor for framebuffer ***
            return fd;
        }

        //*** not it, close file ***
        ::close(fd);
    }

    emit error( QString("Display: Could not find framebuffer device!!!") );

    //*** not found ***
    return INVALID_FB;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief setupFbDevice - initializes the device by setting up a memory map
 */
//******************************************************************************
void SHLedMatrix::setupFbDevice()
{
struct fb_var_screeninfo vInfo;
struct fb_fix_screeninfo fInfo;
bool expected = true;

    //*** get fixed screen information ***
    if ( ioctl( fbFd_, FBIOGET_FSCREENINFO, &fInfo ) == -1 )
    {
        lastError_ = "Error reading fixed information from framebuffer";
        emit error( QString("Display: Error reading fixed information from framebuffer") );
        return;
    }

    //*** get variable screen information ***
    if ( ioctl( fbFd_, FBIOGET_VSCREENINFO, &vInfo ) == -1 )
    {
        lastError_ = "Error reading variable information from framebuffer";
        emit error( QString("Display: Error reading variable information from framebuffer") );
        return;
    }

    //*** verify expected values ***
    if ( vInfo.xres != DisplayXSize || vInfo.yres != DisplayYSize ) expected = false;
    if ( vInfo.bits_per_pixel != DisplayBitsPerPixel ) expected = false;
    if ( vInfo.xoffset != 0 || vInfo.yoffset != 0 ) expected = false;

    if ( !expected )
    {
        lastError_ = "Framebuffer parameters are different than expected";
        emit error( QString("Display: Framebuffer parameters are different than expected") );
        return;
    }

    //*** set up memory mapping ***
    fbPtr_ = (quint16 *)mmap( 0,
                              DisplayMemSizeBytes,
                              PROT_READ | PROT_WRITE,
                              MAP_SHARED,
                              fbFd_,
                              0);

    //*** check results ***
    if ( fbPtr_ != MAP_FAILED )
    {
        //*** we have a valid memory mapped framebuffer ***
        validFbPtr_ = true;

        //*** we are ready to go ***
        ready_ = true;

        clear();
    }
    else
    {
        emit error( QString("Display: Error getting framebuffer memory map") );
    }
}


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
long SHLedMatrix::locationFromCoordinates( int x, int y )
{
long location = 0;

    //*** calculate the location within the framebuffer ***
    location = y * DisplayXSize + x;

    return location;
}


//******************************************************************************
//******************************************************************************
/**
 * @brief transpose - transpose the square matrix in place
 * @param matrix - pointer to the matrix
 * @param mSize  - x and y dimensions of the matrix
 */
//******************************************************************************
void SHLedMatrix::transpose( quint16 *matrix, int mSize )
{
int i,j,idx1,idx2;

    for ( i = 0; i<(mSize-1); i++ )
    {
        for ( j=1; j<(mSize-i); j++ )
        {
            idx1 = (i*mSize+i) + j;
            idx2 = (i*mSize+i) + j*mSize;
            qSwap( matrix[idx1], matrix[idx2] );
        }
    }
}


//******************************************************************************
//******************************************************************************
/**
 * @brief reverse - reverse the rows/columns of a matrix
 * @param mat    - pointer to matrix
 * @param mSize  - x and y dimensions of matrix
 * @param revDir - reverse rows or columns
 */
//******************************************************************************
void SHLedMatrix::reverse( quint16 *mat, int mSize, RevDir revDir )
{
int num = mSize / 2;
int x=0,y=0;
int idx1=0, idx2=0;

    if ( revDir == REV_COL )
    {
        for ( x=0; x<num; x++ )
        {
            for ( y=0; y<mSize; y++ )
            {
                idx1 = y*mSize + x;
                idx2 = y*mSize + (mSize - x - 1);
                qSwap( mat[idx1], mat[idx2] );
            }
        }
    }

    else
    {
        for ( y=0; y<num; y++ )
        {
            for ( x=0; x<mSize; x++ )
            {
                idx1 = y*mSize + x;
                idx2 = (mSize - y - 1) * mSize + x;
                qSwap( mat[idx1], mat[idx2] );
            }
        }
    }
}

//******************************************************************************
//******************************************************************************
/**
 * @brief SHLedMatrix::dumpImage - dumps image values to debug
 * @param image - image to dump
 */
//******************************************************************************
void SHLedMatrix::dumpImage( QImage *image )
{
    quint16 *iLine;
    for ( int i=0; i<8; i++ )
    {
        iLine = (quint16*)image->scanLine( i );
        QString buf;
        QString buffer;
        for ( int j=0; j<8; j++ )
        {
            buf.sprintf( "%04X ", iLine[j] );
            buffer += buf;
        }
        qDebug() << buffer;
    }

    qDebug() << "";
}





