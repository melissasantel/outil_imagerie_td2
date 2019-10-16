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
 */

/**
 * @file colorselector.h
 *
 * Defines a controller for changing the color of the displayed image.
 */

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H


#include "imagemodel.h"
#include "controller.h"

/**
 * This class is a Controller for changing the colors of the displayed 
 * image like brightness, contrast, lut...
 */
class ColorSelector : public QMainWindow, public Controller {

   Q_OBJECT

private :

   /** The buttons for displaying only the red band. */
   QCheckBox *_red;

   /** The buttons for displaying only the green band. */
   QCheckBox *_green;

   /** The button for displaying only the blue band. */
   QCheckBox *_blue;

   /** The label that contains the word brightness. */
   QLabel *_brightTitle;

   /** The slider for selecting the brightness. */
   QSlider *_brightSlider;

   /** The value of the brightness. */
   QLCDNumber *_brightNumber;

   /** The label that contains the word contrast. */
   QLabel *_contrastTitle;
   
   /** The slider for selecting the contrast. */
   QSlider *_contrastSlider;

   /** The value of the contrast. */
   QLCDNumber *_contrastNumber;

   /** The button for resetting all modifications. */
   QPushButton *_reset;
   
   /** The group of buttons that contains the different choise of lut. */
   QButtonGroup *_luts;

   /** The button for selecting the orignal lut. */
   QAbstractButton *_original;

   /** The button for selecting the negatif lut. */
   QAbstractButton *_negatif;

   /** The button for selecting the random lut. */
   QAbstractButton *_random;

   /** The button for selecting the region lut. */
   QAbstractButton *_region;

   /** The button for selecting the rainbow lut. */
   QAbstractButton *_rainbow;

   /** The button for selecting the noise lut. */
   QAbstractButton *_noise;
      
private slots :

/**
 * Returns to the initial settings.
 */
void resetSettings( );

   /**
    * Select the lut from the button clicked.
    * @param button The clicked button,
    */
   void selectLut( QAbstractButton *button );

public:
   
   /**
    * Creates a window for changing colors of the displayed image.
    * @param model The model that contains the data.
    */
   ColorSelector( const ImageModel *model );

   /**
    * Deletes this window.
    */
   ~ColorSelector(){
      delete _reset;
      delete _contrastNumber;
      delete _contrastSlider;
      delete _brightNumber;
      delete _brightSlider;
      delete _blue;
      delete _green;
      delete _red;
      delete _brightTitle;
      delete _contrastTitle;
      delete _luts;
      delete _original;
      delete _negatif;
      delete _region;
      delete _random;
      delete _rainbow;
      delete _noise;
   }

   signals:
   
   /**
    * Emited when the button reset is pressed.
    */
   void reset( );

   /**
    * Emited when a new lut is selected.
    * @param id The index of the lut.
    */
   void newLut( int id );

};

#endif
