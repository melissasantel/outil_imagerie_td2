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
 * @file viewer.h
 *
 * Defines the base classe of all views.
 */

#ifndef VIEWER_H
#define VIEWER_H

/**
 * The interface of all views of the model data.
 * A view displays a part of the model data in a specific widget.
 * Examples of Viewer are:
 * <ul>
 * <li>Canvas
 * <li>Histogram
 * <li>Profile
 * <li>Values
 * </ul>
 */
class Viewer  {

public :
   
   /**
    * Updates the current display of the related data. 
    */  
   virtual void update( )=0;
};

#endif
