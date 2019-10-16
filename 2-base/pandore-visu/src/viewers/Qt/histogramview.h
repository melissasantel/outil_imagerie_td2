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
 * @file histogramview.h
 *
 * Creates a window for displaying an histogram,where
 * the values of pixels are represented according to their occurences. 
 */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "imagemodel.h"
#include "viewer.h"

/**
 * This class displayed a histogram, where the values of pixels are represented
 * according to their occurences.
 */
class HistogramView : public QMainWindow, public Viewer {
   Q_OBJECT

private:
   
   /** The model that contains the data. */
   const ImageModel *_model;

   /** The width of the histogram. */
   const static int HISTWIDTH=256;
   
   /** The height of the histogram. */
   const static int HISTHEIGHT=100;

   /** The table that contains the values of the histogram. */
   int _histDraw[3][HISTWIDTH];
   
   /** The begin of histogram. */
   float _hbeg;

   /** The end of histogram. */
   float _hend;

   /** The max value. */
   int _hmax[3];

   /** The index of the max value. */
   int _imax[3];

   /** The step between two bars of the histogram. */
   float _step;

   /** The format of the displayed numbers. */
   char _format[16];

   /** The low threshold. */
   float _thresholdL;

   /** The high threshold. */
   float _thresholdH;
   
   /** The current plane displayed. */
   long _currentPlane;

   /** The status bar that displays the values of one bar of the histogram . */
   QStatusBar *_status;

   /** 
    * Paints histogram.
    */
   void paint( );

private slots:

/**
 * Calls for updating the view.
 * Draws the histogram from thresholding and plane.
 */
void update( );

protected:
   
   /**
    * Calls when the window has to be repaint.
    * @param e The paint event.
    */
   void paintEvent( QPaintEvent *e );

   /**
    * Calls when the mouse buttons are pressed.
    * @param e The mouse event.
    */
   void mousePressEvent( QMouseEvent *e );
   
   /**
    * Calls when the mouse buttons are release.
    * @param e The mouse event.
    */
   void mouseReleaseEvent( QMouseEvent *e );

   /**
    * Calls when the mouse buttons are clicked and moved.
    * @param e The mouse event.
    */
   void mouseMoveEvent( QMouseEvent *e );

public:
   
   /**
    * Builds the window that contains the histogram.
    * @param model The model that contains the data.
    */
   HistogramView( const ImageModel *model );

   /**
    * Destroys the widget.
    */
   ~HistogramView( ){
      delete _status;
   }
  
};

#endif
