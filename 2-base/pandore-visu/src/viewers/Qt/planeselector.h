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
 * @author Nicolas Briand - 2005-01-06
 * @author Régis Clouard - 2006-01-17
 */

/**
 * @file planeselector.h
 *
 * Defines a controller for changing the current plane 
 * of a 3D image.
 */

#ifndef PLANESELECTOR_H
#define PLANESELECTOR_H

#include "imagemodel.h"
#include "controller.h"

/**
 * This class contains a slider for changing the current plane
 * of the displayed image.
 */
class PlaneSelector : public QWidget, public Controller {
   Q_OBJECT

private:

   /** The slider for changing the plane. */
   QSlider *_slider;

   /** The displayed plane number. */
   QLCDNumber *_number;


private slots:

/**
 * Sets the keyboard focus to the slider.
 */
void addFocus( );

public:
   
   /**
    * Creates a slider to change the current plane.
    * @param model The model that calls the controller.
    * @param parent The parent widget.
    */
   PlaneSelector( const ImageModel *model, QWidget *parent );

   /**
    * Deletes this controller.
    */
   ~PlaneSelector(){
      delete _slider;
      delete _number;
   }

public slots:

/**
 * Changes the maximum value, called when the image is rotated.
 * @param nbPlane The new number of planes.
 */
void updateNbPlane( long nbPlane );
	    

protected:
   
   /**
    * Calls when the mouse is pressed.
    * Gives the keyboard focus to the slider.
    * @param e The mouse event.
    */
   void mousePressEvent( QMouseEvent *e );
};

#endif
