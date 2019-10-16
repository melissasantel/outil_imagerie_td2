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
 * @file zoomselector.h
 * 
 * Defines a submenu for selecting the factor zoom.
 */

#ifndef ZOOM_H
#define ZOOM_H

#include "imagemodel.h"
#include "controller.h"


/**
 * This class is a Controller for selecting the zoom factor in a menu.
 * The current zoom factor is marked as checked in the submenu.
 */
class ZoomSelector : public QMenu, public Controller {

   Q_OBJECT

private:
   
   /** The index in the menu of the current zoom. */
   QAction *_currentZoom;

   /** The action for seting the zoom factor to 400%. */
   QAction *_z400;

   /** The action for seting the zoom factor to 300%. */
   QAction *_z300;

   /** The action for seting the zoom factor to 200%. */
   QAction *_z200;

   /** The action for seting the zoom factor to 100%. */
   QAction *_z100;

   /** The action for seting the zoom factor to 50%. */
   QAction *_z50;

   /** The action for seting the zoom factor to 33%. */
   QAction *_z33;

   /** The action for seting the zoom factor to 25%. */
   QAction *_z25;

   
public:

   /**
    * Creates a menu for selecting the zoom.
    * @param model The model that contains the data.
    */
   ZoomSelector(const ImageModel *model);

   signals:
   
   /**
    * Emited for changing the zoom factor of the model.
    * @param factor The new factor zoom.
    */
   void newZoom(float factor);

private slots:

/**
 * Sets the selected zoom item checked and emits the related signal.
 * @param id The index of item selected.
 */
void zoomSelected(QAction* id);

   /**
    * Increments or decrements the zoom factor.
    * @param increment True: increment. False: decrement.
    */
   void keyZoom(bool increment);
};

#endif
