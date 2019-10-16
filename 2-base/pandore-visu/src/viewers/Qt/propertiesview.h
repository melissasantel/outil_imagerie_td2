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
 * @file propertiesview.h
 *
 * Defines the class which shows in a window the information
 * properabout the input image.
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H


#include "imagemodel.h"

/**
 * This class is a window that diplays information about the 
 * current input image.
 */
class PropertiesView : public QMainWindow {
   
   Q_OBJECT
   
public :
   
   /**
    * Creates a window for displaying related information of the
    * current image.
    * @param model The model that contains image information. 
    */
   PropertiesView( const ImageModel *model );
  
};

#endif
