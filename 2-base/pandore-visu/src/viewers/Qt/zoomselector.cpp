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
 * @file zoomselector.cpp
 * Menu for selecting the factor zoom.
 */

#include "zoomselector.h"

ZoomSelector::ZoomSelector( const ImageModel *model ):QMenu("Zoom") {
   _z400=addAction("400%");
   _z400->setCheckable(true);
   _z300=addAction("300%");
   _z300->setCheckable(true);
   _z200=addAction("200%");
   _z200->setCheckable(true);
   _z100=addAction("100%");
   _z100->setCheckable(true);
   _z50=addAction("50%");
   _z50->setCheckable(true);
   _z33=addAction("33%");
   _z33->setCheckable(true);
   _z25=addAction("25%");
   _z25->setCheckable(true);
   _currentZoom=_z100;
   _currentZoom->setChecked(true);

   connect(model,SIGNAL(zoom(bool)),this,SLOT(keyZoom(bool)));
   connect(this,SIGNAL(triggered(QAction*)),this,SLOT(zoomSelected(QAction*)));
   connect(this,SIGNAL(newZoom(float)),model,SLOT(changeZoom(float)));
}

void ZoomSelector::zoomSelected( QAction *id ){
   id->setChecked(true);
   if (id!=_currentZoom){
      _currentZoom->setChecked(false);
      _currentZoom=id;
      if (_currentZoom== _z400)
	 emit newZoom(4.0F);
      else if (_currentZoom==_z300)
	 emit newZoom(3.0F);
      else if (_currentZoom==_z200)
	 emit newZoom(2.0F);
      else if (_currentZoom==_z100)
	 emit newZoom(1.0F);
      else if (_currentZoom==_z50)
	 emit newZoom(0.5F);
      else if (_currentZoom==_z33)
	 emit newZoom(0.33F);
      else if (_currentZoom==_z25)
	 emit newZoom(0.25F);
   }
}

void ZoomSelector::keyZoom( bool increment ){
   if (increment){
      if (_currentZoom==_z300)
	 zoomSelected(_z400);
      else if (_currentZoom==_z200)
	 zoomSelected(_z300);
      else if (_currentZoom==_z100)
      	 zoomSelected(_z200);
      else if (_currentZoom==_z50)
	 zoomSelected(_z100);
      else if (_currentZoom==_z33)
	 zoomSelected(_z50);
      else if (_currentZoom==_z25)
	 zoomSelected(_z33);
   } else {
      if (_currentZoom== _z400)
	 zoomSelected(_z300);
      else if (_currentZoom==_z300)
	 zoomSelected(_z200);
      else if (_currentZoom==_z200)
	 zoomSelected(_z100);
      else if (_currentZoom==_z100)
      	 zoomSelected(_z50);
      else if (_currentZoom==_z50)
      	 zoomSelected(_z33);
      else if (_currentZoom==_z33)
	 zoomSelected(_z25);
   }
}
