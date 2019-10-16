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
 * @author Régis Clouard - 2006-01-17
 * @author Régis Clouard - Jun 05, 2010 (add color thresholding)
 */

/**
 * @file thresholdselector.h
 *
 * Defines a class for thresholding the displayed gray image.
 */

#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "imagemodel.h"
#include "controller.h"

#define MAXNSTEP 500	// Max number of step in the slider

/**
 * This class is a Controller that contains two sliders for
 * thresholding gray image, one for low thresholding and one for high thresholding.
 */
class ThresholdSelector: public QMainWindow, public Controller {
   Q_OBJECT
   
private :
   
   /** The low threshold boundary. */
   float _thresholdL[3];
   
   /** The high threshold boundary. */
   float _thresholdH[3];
   
   /** The normalization value for float values. */
   int _normseuil;
   
   /** The slider for selecting the high thresholding. */
   QSlider *_sliderH[3];
   
   /** The Slider for selecting the low thresholding. */
   QSlider *_sliderL[3];
   
   /** The number displayed for the low thresholding. */
   QLabel *_numberL[3];
   
   /** The number displayed for the high thresholding. */
   QLabel *_numberH[3];
   
   /** The format of the displayed numbers. */
   char _format[16];
   
   /** The step for big float. */
   float _step;
   
   /** The minimum pixel value of the image. */
   float _min;
	     
private slots:
   
   void thresholdL0( int threshold );
   void thresholdL1( int threshold );
   void thresholdL2( int threshold );
   void thresholdH0( int threshold );
   void thresholdH1( int threshold );
   void thresholdH2( int threshold );

   /**
    * Thresholds the displayed image from the low threshold.
    * @param band an image band.
    * @param threshold The value of the threshold.
    */
   void thresholdL( int band, int threshold );
   
   /**
    * Thresholds the displayed image from the high threshold.
    * @param band an image band.
    * @param threshold The value of the threshold.
    */
   void thresholdH( int band, int threshold );
   
public:
   
   /**
    * Creates a window for selecting the thresholding
    * with two sliders.
    * @param model The model that contains the image data.
    */
   ThresholdSelector( const ImageModel *model );
   
   /**
    * Deletes this controller.
    */
   ~ThresholdSelector() {
      for (int b=0; b<3; b++) {
	 if (_sliderH[b]!=0) delete _sliderH[b];
	 if (_sliderL[b]!=0) delete _sliderL[b];
	 if (_numberL[b]!=0) delete _numberL[b];
	 if (_numberH[b]!=0) delete _numberH[b];
      }
   }
   
signals:
   
   /** Emited when changing the low thresholding. */
   void lowThreshold( int, float );
   
   /** Emited when changing the hight thresholding. */
   void highThreshold( int, float );

};

#endif
