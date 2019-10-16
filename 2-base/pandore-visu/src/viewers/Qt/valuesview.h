/* -*- mode: c++; c-basic-offset: 3 -*-
 *
 * PANDORE (PANTHEON Project)
 *
 * GREYC IMAGE
 * 6 Boulevard Marechal Juin
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
 * @author Régis Clouard - 2007-04-04 (display coordinates inside the window)
 */


/**
 * @file valuesview.h
 *
 * Creates a window that displays the pixel values
 * of the current selected area. 
 */

#ifndef VALUES_H
#define VALUES_H

#include "imagemodel.h"
#include "viewer.h"

/**
 * This class displays the value and the related color of the pixels within 
 * the current selected area. This area is AxA pixels rectangle and its upper
 * left corner is located from the mouse pointer coordinates.
 */
class ValuesView : public QMainWindow, public Viewer {

   Q_OBJECT

private :
 
   /** The model that contains the image data. */
   const ImageModel *_model;

   /** The title of the window without the position of the cursor. */
   QString _title;

   /** The height in pixel of the font. */
   int _hFont;

   /** The vertical shift between the top of the window and the values rectangle. */
   int _yshift;

   /** The number of rows of the selected area. */
   static const int NBROW=6;

   /** The number of column of the selected area. */
   static const int NBCOL=6;
   
   /** The number format for displaying a pixel value. */
   char _format[16];

   /**
    * Paints the widget.
    */
   void paint();

protected:
   
   /**
    * Called when something must be drawn.
    * Repaints the colors cases and the values.
    * @param e The paint event.
    */
   void paintEvent( QPaintEvent *e );
   
public :
   
   /**
    * Creates a window where the pixels values are displayed.
    * @param model The model that contains the data.
    */
   ValuesView( const ImageModel *model );

private slots:

/**
 * Updates the widget content from the new pixel values.
 */
void update( );

};

#endif
