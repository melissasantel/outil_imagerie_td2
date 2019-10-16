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
 */

/**
 * @file canvasview.h
 *
 * Defines the class that displays the physic image.
 */

#ifndef CANVAS_H
#define CANVAS_H


#include "viewer.h"
#include "imagemodel.h"

/**
 * The CanvasView class provides a 2D area 
 * that displays the physic image with the current vizualisation parameters.
 */
class CanvasView : public QWidget, public Viewer {
   Q_OBJECT

private :
   
   /** The model that contains the image data. */
   ImageModel *_model;

   /** The Qt Pixmap which is displayed. */
   QPixmap *_pixmap;

   /** The current X position of the cursor. */
   int _currentPosX;
   
   /** The current Y position of cursor. */ 
   int _currentPosY;

   /** The x coordinate of the last drawing point. */ 
   int _lastDrawingX;

   /** The y coordinate of the last drawing point. */ 
   int _lastDrawingY;

   /**
    * Repaints the pixmap in the rectangle given by the PaintEvent.
    * @param e The paint event that contains the rectangle.
    */
   void paint( QPaintEvent *e=0 );


protected :
 
   /**
    * Repaints the current content of the widget.
    * @param paintEvent	The event handler.
    */   
   void paintEvent( QPaintEvent* paintEvent );

   /**
    * Called when a mouse button is released.
    * Sets the cursor coordinate to the mouse pointer location.
    * @param e The mouse event.
    */
   void mouseReleaseEvent( QMouseEvent *e );

   /**
    * Called when a mouse button is pressed and the mouse moved.
    * 
    * @param e The mouse event.
    */
   void mouseMoveEvent( QMouseEvent *e );


   /**
    * Called when a mouse button is pressed.
    * Sets the cursor coordinates to the mouse pointer location.
    * @param e The mouse event.
    */
   void mousePressEvent( QMouseEvent *e );

   /**
    * Called when a key is pressed.
    * Translates the cursor according to key value with a step of one pixel.
    * @param e The key event.
    */
   void keyPressEvent( QKeyEvent *e );

public :
   
   /**
    * Creates the window that supports the image.
    * @param model The model contains data.
    * @param parent The parent widget.
    */
   CanvasView( ImageModel *model, QWidget *parent );
   
   /**
    * Deletes this view.
    */
   ~CanvasView( ) {
      delete _pixmap;
   }
   
private slots:

/**
 * Updates the image from new vizualisation parameters.
 */
void update( );

};

#endif
