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

/*
 * @file profileview.h
 *
 * Creates a window for displaying an histogram,
 * where the values of pixel of the column or the row selected are represented.
 */

#ifndef PROFIL_H
#define PROFIL_H

#include "imagemodel.h"
#include "viewer.h"


/**
 * This class is a window with histogram that displays the values of pixel.
 */
class ProfileView : public QMainWindow, public Viewer {

   Q_OBJECT

private:
   
   /** The model that calls the view. */
   const ImageModel *_model;

   /** The flag for profile for row or column. */
   bool _row;

   /** The height of one histogram. */
   int _h;

   /** The height in pixel of the font. */
   int _hFont;

   /** The title of the window without the positon of the row or the column. */
   QString _title;
   
   /**
    * Paints the widget.
    */
   void paint( );
   

public:
   
   /**
    * Creates the window for displaying histograms.
    * @param model The model that calls the Viewer.
    * @param row true : profil on row, false : profil on column.
    */
   ProfileView( const ImageModel *model, bool row );

public slots:

/**
 * Update the histogram from threshold, column and row selected.
 */
void update();


protected:

   /**
    * Called when the window must be repaint.
    * @param e The paint event.
    */
   void paintEvent( QPaintEvent *e );

};

#endif
