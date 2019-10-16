/* -*- mode: c++; c-basic-offset: 3 -*-
 *
 * PANDORE (PANTHEON Project)
 *
 * GREYC IMAGE
 * 6 Boulevard Maréchal Juin
 * F-14050 Caen Cedex France
 *
 * This file is free software. You can use it, distribute it
 * and/or modify it. However, the entire risk to the quality
 * and performance of this program is with you.
 *
 * For more information, refer to:
 * http://www.greyc.ensicaen.fr/EquipeImage/Pandore/
 */

/**
 * @author Nicolas Briand - 2005-23-05
 * @author Régis Clouard - 2005-10-25 (change drawing)
 * @author Régis Clouard - 2006-01-17 (add log transfrom)
 */

/**
 * @file imagemodel.h
 *
 * Defines classe that manipulates the image content from Viewer
 * and Controller
 */

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QtGui>
#include <pandore.h>

#include <stack>
#include <math.h>

#include "imagevisu.h"

extern bool _DRAWINGMODE;

/**
 * This structure defines the coordinate
 * and the color of a drawing point.
 * for 
 */
class DrawingPoint {
public:
   int x;
   int y;
   DrawingPoint() {}
   DrawingPoint(int xx,int yy):x(xx),y(yy) {}
};

/**
 * This class defines the user interface.
 * It containts the image data and the visualization parameters
 * such as the zoom, the plane, the brightness values...
 * Each related Viewer is a viewpoint on the image data.
 */
class ImageModel : public QMainWindow {
   
   Q_OBJECT
   
public:
   
   /**
    * The enumeration of the supported luts.
    */
   typedef enum {
      original, /**< The original lut. */  
      random,   /**< A lut generates randomly. */
      region,   /**< The lut that representes the different region of the image. */
      rainbow,  /**< The rainbow lut. */
      negatif,  /**< The lut where colors are inversed. */
      noise     /**< The lut that represente the noise of the image.*/
   } Lut;
   
   
   /**
    * The enumeration of the supported different types of data normalizations.
    */
   typedef enum {
      linear,       /**< The linear transform of the gray levels. */
      equalization, /**< The histogram equalization dynamic. */
      logarithmic,  /**< The logarithmic transform of the gray levels. */
      exponential   /**< The exponential transform of the gray levels. */
   } Normalization;
   
   /**
    * Creates a window that contains the menu, displays the image,
    * and the user can draw in the image.
    * @param ims The image that contains the source data.
    * @param file The name of input file.
    * @param imd Tne image that is used to draw.
    */
   ImageModel( const ImageSource *ims, const char *file, Imx3duc *imd=0 );
   
private :
   
   /** The menu bar. */
   QMenuBar *_menu;
   
   /** The source image which contains the real data. */
   const ImageSource *_imageSource;

   /** The logic image which contains the image to be displayed. */
   const ImageLogic *_imageLogic;       
   
   /** The physic image which contains the image actually displayed. */
   QImage _imagePhysic;

   /** The scrollarea that supports the image displayed. */
   QScrollArea *_scrollImage;
   
   /** The window for thresholding the display image. */
   QMainWindow *_thresholdWindow;
   
   /** The window that displays the properties of the input image. */
   QMainWindow *_propertiesWindow;
   
   /** The controller for changing plane of the display image. */
   QWidget *_planeBar;

   /** The controller for selecting the zoom. */
   QMenu *_zoomMenu;

   /** The controller for selecting the dynamic. */
   QMenu *_dynamicMenu;

   /** The controller for changing the point of view for 3D image. */
   QMenu *_rotateMenu;
   
   /** The histogram viewer. */
   QMainWindow *_histogramWindow;
   
   /** The row profile viewer. */
   QMainWindow *_profileRWindow;
   
   /** The column profile viewer. */
   QMainWindow *_profileCWindow;
   
   /** The controller for changing colors of the display image. */
   QMainWindow *_colorWindow;

   /** The pixel value viewer. */
   QMainWindow *_valuesWindow;

   /** The current zoom factor. */
   float _zoom;
   
   /** The current X position of the cursor. */
   long _currentPosX;
   
   /** The current Y position of the cursor. */ 
   long _currentPosY;
   
   /** The current plane for 3D images. */
   long _currentPlane;

   /** The low threshold value. */
   float _thresholdL[3];
   
   /** The high threshold value. */
   float _thresholdH[3];

   /** The flag for displaying the red band. */
   int _red;

   /** The flag for displaying the green band. */
   int _green;

   /** The flag for displaying the blue band. */
   int _blue;

   /** The current brightness value. */
   int _brightness;
   
   /** The current contrast value. */
   int _contrast;

   /** The current lut. */
   Uchar _lut[3][MAXCOLS];

   /** The flag for displaying the cursor. */
   bool _cursor;

   /** The index of the chosen normalization. */
   int _normalization;

   /** The pointer on the output image. */
   Imx3duc *_imageDessin;

   /** The image that contains the drawn points. */
   Imx3duc *_imageDraw;

   /** The image that contains the last drawing. */
   Imx3duc *_lastDraw;

   /** The stack of the last drawing points - used by undo() */
   std::stack<DrawingPoint> _lastDrawingPoints;

   /** The flag between pencil or erase mode. */
   bool _pencilStatus;

   /** The direction of the original x axis in the new locate; */
   int _vectX[3];

   /** The direction of the original y axis in the new locate; */
   int _vectY[3];

   /** The direction of the original z axis in the new locate; */
   int _vectZ[3];

   /**
    * Rotate the image draw around the x axis.
    */
   void rotateDessinX();

   /**
    * Rotate the image draw around the y axis.
    */
   void rotateDessinY();

   /**
    * Rotate the image draw around the z axis.
    */
   void rotateDessinZ();

private slots:

/**
 * Shows the histogram window.
 */
void histo();

   /**
    * Shows the row profil window.
    */
   void row();

   /**
    * Shows the column profil window.
    */
   void column();

   /**
    * Shows the pixel values window.
    */
   void values();

   /**
    * Shows the properties window.
    */
   void properties();

   /**
    * Shows the colors window.
    */
   void colors();

   /**
    * Shows the thresholding window.
    */
   void thresholding();

   /**
    * Opens a new image in the application.
    */
   void openImage();

   /**
    * Savs the current drawing in a bmp image.
    */
   void saveAsImage();

   /**
    * Closes the application.
    */
   void closeApplication();

   /**
    * Display or not the cursor.
    */
   void changeCursorStatus();

   /**
    * Changes the drawing between pencil / erase.
    */
   void changePencilStatus() { _pencilStatus = !_pencilStatus; } 

   /**
    * Deletes the last drawing.
    */
   void undoDraw();

   /**
    * Clear the image drawing.
    */
   void clearDraw();

   /**
    * Draws a line onto the current image,
    * from the an initial point to a final point.
    * @param xb the initial x cordinate.
    * @param yb the initial y cordinate.
    * @param xe the final x cordinate.
    * @param ye the final y cordinate.
    * @param col the color.
    */
   void drawLine( int xb, int yb, int xe, int ye, Uchar col );

private:
   
   /**
    * Builds the menu with all items.
    */
   void buildMenu();

   /**
    * Sets the title of the window with the given filename
    * without its extension and directory.
    * @param file The absolute filename.
    */
   void CreateTitle( const char *file );

   /**
    * Builds the physic image from the source image, the logic image,
    * the plane, the invert flag, the colors flags, and the zoom factor.
    */
   void buildImagePhysic();
   
   /**
    * Builds the logic image from the source image.
    */
   void buildImageLogic();

   /**
    * Emits the Update signal when data or visualization parameters
    * have changed.
    */
   void callUpdate();

   /**
    * Builds the triplet RGB from the brightness and contrast values.
    * @param r The red value.
    * @param g The green value.
    * @param b The blue value.
    */
   QRgb setRgb( Uchar r, Uchar g, Uchar b );

   /**
    * Equalizes the histogram to reduce
    * the number of colors from maxval-minval to outbands.
    * @param outbands The number of levels for the output image.
    */
   void histogramEqualization( int outbands );

   /**
    * Normalizes values from 0 to outbands of the input data vector
    * into the output image.
    * @param outbands The number of levels for the output image.
    */
   void linearTransform( int outbands );

   /**
    * Builds the image logic from a region map labels.
    */
   void regionEncoding( );

   /**
    * Normalizes values from 0 to outbands of the input data vector
    * into the output image by using the exp transform. That is:
    * output=exp(input/c)-1 where c is a normalization factor
    * to map values between 0 and outband.
    * @param outbands The number of levels for the output image.
    */
   void expTransform( int outbands );

   /**
    * Normalizes values from 0 to outbands of the input data vector
    * into the output image by using the log transform. That is:
    * output=c.log(1+input) where c is a normalization factor
    * to map values between 0 and outband.
    * @param outbands The number of levels for the output image.
    */
   void logTransform( int outbands );

   /**
    * Creates the original lut.
    */
   void buildOriginalLut();

   /**
    * Creates the random lut.
    */
   void buildRandomLut();


   /**
    * Creates the region lut.
    */
   void buildRegionLut();

   /**
    * Creates the rainbow lut.
    */
   void buildRainbowLut();


   /**
    * Creates the negatif lut.
    */
   void buildNegatifLut();


   /**
    * Creates the noise lut.
    */
   void buildNoiseLut();

public : 
   
   /**
    * Quits the application.
    */
   ~ImageModel() {
      delete _planeBar;
      delete _thresholdWindow;
      delete _propertiesWindow;
      delete _colorWindow;
      delete _valuesWindow;
      delete _dynamicMenu;
      delete _zoomMenu;
      delete _histogramWindow;
      delete _profileRWindow;
      delete _profileCWindow;
      delete _menu;
      delete _scrollImage;
      delete _rotateMenu;
      delete _imageSource;
      delete _imageLogic;
      if (_DRAWINGMODE) {
	 delete _lastDraw;
	 QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	 if(_vectX[2]==1) {
	    rotateDessinY();
	    rotateDessinY();
	    rotateDessinY();
	 } else if(_vectX[2]==-1)
	    rotateDessinY();
	 else if(_vectY[2]==1)
	    rotateDessinX();
	 else if(_vectY[2]==-1) {
	    rotateDessinX();
	    rotateDessinX();
	    rotateDessinX();
	 } else if(_vectZ[2]==-1) {
	    rotateDessinY();
	    rotateDessinY();
	 }
	 if(_vectX[1]==-1) {
	    rotateDessinZ();
	    rotateDessinZ();
	    rotateDessinZ();
	 } else if(_vectX[1]==1) {
	    rotateDessinZ();
	 }else if(_vectY[1]==-1) {
	    rotateDessinZ();
	    rotateDessinZ();
	 } 
	 
	 Uchar *d=_imageDessin->Vector(0);
	 Uchar *s=_imageDraw->Vector(0);
	 for(;s<_imageDraw->Vector(0)+_imageDraw->VectorSize();)
	    *(d++)=*(s++);
	 delete _imageDraw;
	 QApplication::restoreOverrideCursor();
      }
   }

   /**
    * @return The pointer on the image source.
    */
   const ImageSource* getImageSource() const{
      return _imageSource;
   }

   /**
    * @return The pointer on the image logic.
    */
   const ImageLogic* getImageLogic() const{
      return _imageLogic;
   }

   /**
    * @return The x position of the cursor.
    */
   long CurrentPosX() const{
      return _currentPosX;
   }

   /**
    * @return The y position of the cursor.
    */
   long CurrentPosY() const{
      return _currentPosY;
   }

   /**
    * @return The current zoom factor.
    */
   float getZoom() const{
      return _zoom;
   }
   
   /**
    * @return The low threshold factor.
    */
   float getThresholdL( int band ) const{
      return _thresholdL[band];
   }

   /**
    * @return The high threshold factor.
    */
   float getThresholdH( int band ) const{
      return _thresholdH[band];
   }

   /**
    * @return The current plane.
    */
   long getPlane() const{
      return _currentPlane;
   }
   
   /**
    * Sets the x and y positions of cursor and updates the related views.
    * @param x The x-coordinate in input image coordinate without zoom factor.
    * @param y The y-coordinate in input image coordinate without zoom factor.
    */
   void setPosition( long x, long y ) {
      if (x<_imageSource->Width() && x>=0)
	 _currentPosX=x;
      if (y<_imageSource->Height() && y>=0)
	 _currentPosY=y;
      callUpdate();
   }

   /**
    * @return The physical image.
    */
   QImage getImagePhysic( ) const {
      return _imagePhysic;
   }
   
   /**
    * Sets all values of _lastDraw to zero.
    */
   void cleanLastDraw( );

   /**
    * @return The flag for displaying the cursor.
    */
   bool cursorIsVisible( ) {
      return _cursor;
   }

   /**
    * @return The draw image.
    */
   Imx3duc* getImageDraw() const {
      return _imageDraw;
   }

   /**
    * Draws a line between the last drawing point
    * (xb,yb) and the current point (xe,ye);
    * @param xb The x position of the start point.
    * @param yb The y position of the start point.
    * @param xe The x position of the end point.
    * @param ye The y position of the end point.
    */
   void drawLine( int xb, int yb, int xe, int ye );

   signals:
 
   /** 
    * Emited when updating all Viewer.
    */
   void askForUpdate();

   /**
    * Emited when the user wants to change the zoom factor with the keyboard.
    * @param plus True:increment the zoom. False: decrement the zoom.
    */
   void zoom( bool plus );

   /**
    * Emited when the user rotate the image and so change the number of plane.
    * @param nbPlane: The number of plane.
    */
   void changeNumberPlane( long nbPlane );

   /** Emited when something is draw. */
   void draw();
    
public slots:

/**
 * Displays or not the red color.
 * @param on True/False.
 */
void setRed( bool on );

   /**
    * Displays or not the green color.
    * @param on True/False.
    */
   void setGreen( bool on );

   /**
    * Displays or not the blue color.
    * @param on True/False.
    */
   void setBlue( bool on );

   /**
    * Sets the values for thresholding the display image,
    * and updates all related views.
    * @param band the band number.
    * @param thresholdL The low threshold value.
    */
   void changeThresholdL( int band, float thresholdL );

   /**
    * Sets the values for thresholding the display image,
    * and updates all related views.
    * @param band the band number.
    * @param thresholdH The high threshold value.
    */
   void changeThresholdH( int band, float thresholdH );

   /**
    * Changes the zoom factor
    * and updates display.
    * @param zoom The zoom factor.
    */
   void changeZoom( float zoom );

   /**
    * Changes the current plane of the image
    * @param p The new plane number.
    */
   void changePlane( int p );

   /**
    * Changes the current lut.
    * @param id The index of the lut.
    */
   void changeLut( int id );

   /**
    * Sets the brightness value to the specified value.
    * Rebuilds the display image from visualization parameters.
    * @param val The value of the brightness.
    */
   void setBrightness( int val );

   /**
    * Sets the contrast value to the specified value.
    * Rebuilds the display image from visualization parameters.
    * @param val The value of the contrast.
    */
   void setContrast( int val );

   /**
    * Rebuilds the image logic from a new normalization.
    * @param id The index of the chosen normalization.
    */
   void changeDynamic( int id );

   /**
    * Rotate the source image around the x axis.
    */
   void rotateX();

   /**
    * Rotate the source image around the y axis.
    */
   void rotateY();

   /**
    * Rotate the source image around the z axis.
    */
   void rotateZ();


   /**
    * Sets the colors configuration to the default settings.
    */
   void resetColor();


protected :

   /**
    * Called when a key is pressed.
    * Opens a window, or a menu, or moves the cursor.
    * @param e The key event
    */
   void keyPressEvent( QKeyEvent *e );  
};


#endif
