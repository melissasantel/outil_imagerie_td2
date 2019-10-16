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
 * @file rotateselector.cpp
 *
 * Defines a submenu for rotating a 3D image around the y axis.
 */

#include "rotateselector.h"

RotateSelector::RotateSelector( const ImageModel *model ): QMenu("&Rotate") {
   _x=addAction(tr("90° /&X"),model,SLOT(rotateX()));
   _y=addAction(tr("90° /&Y"),model,SLOT(rotateY()));
   _z=addAction(tr("90° /&Z"),model,SLOT(rotateZ()));

   if (model->getImageSource()->Depth()==1) {
      _x->setDisabled(true);
      _y->setDisabled(true);
   }
}
