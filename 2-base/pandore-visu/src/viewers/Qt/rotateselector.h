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
 * @file rotateselector.h
 *
 * Defines a submenu for rotating a 3D image around the y axis.
 */

#ifndef ROTATE_H
#define ROTATE_H

#include "imagemodel.h"
#include "controller.h"

/**
 * This class is a menu that allow the rotation of 3D image around the y axis.
 */
class RotateSelector : public QMenu, public Controller {

   Q_OBJECT

private:

   /** The action for rotating around the x axis. */
   QAction *_x;

   /** The action for rotating around the y axis. */
   QAction *_y;

   /** The action for rotating around the z axis. */
   QAction *_z;

public:

   
   /**  
    * Creates a menu for selecting the current view axis .
    * @param model The model that contains the data.
    */
   RotateSelector( const ImageModel *model );

};

#endif
