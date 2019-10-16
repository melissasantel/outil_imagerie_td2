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
 * @author Regis Clouard - 2006-01-17 (add log and exp transform)
 */

/**
 * @file dynamicselector.h
 *
 * Defines a menu for changing the image dynamic. This corresponds 
 * to different normalizations of data, for instance the linear transfrom
 * the histogram equalization and the logarithmic transfrom
 * (mainly for for frequency image).
 */

#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "imagemodel.h"
#include "controller.h"

/**
 * This class is a menu that is used to select the typed of normalization
 * between linear transfrom and histogram equalization
 * and logarithmic transform.
 */
class DynamicSelector : public QMenu, public Controller {

   Q_OBJECT

private:

   /** The index of the current normalization type. */
   QAction *_currentSelection;
   
   /** The action to select the linear transfrom normalization. */
   QAction *_st;

   /** The action to select the histogram equalization normalization. */
   QAction *_eq;
   
   /** The action to select the logarithmic transfom normalization. */
   QAction *_lt;

   /** The action to select the exponential transfom normalization. */
   QAction *_et;
   
private slots:

/**
 * Checks the selected item and actives the related normalization.
 * @param act The action of the chosen normalization.
 */
void checkItem( QAction* act );
   
public:

   /**  
    * Creates a menu for selecting the normalization type.
    * @param model The model that contains the data to be normalized.
    */
   DynamicSelector( const ImageModel *model );

   signals:

   /**
    * Signal emited when the selection is changed.
    * @param id The index of choosen normalisation.
    */
   void dynamic( int id );
};

#endif
