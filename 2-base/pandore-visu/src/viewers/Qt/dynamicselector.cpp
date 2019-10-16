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
 * @file dynamicselector.cpp
 *
 * A menu for changing the image dynamic.
 */

#include "dynamicselector.h"

DynamicSelector::DynamicSelector( const ImageModel *model ) : QMenu("&Dynamic") {
   _st=addAction(tr("&Linear transform"));
   _st->setCheckable(true);
   _eq=addAction(tr("&Histogram equalization"));
   _eq->setCheckable(true);
   _lt=addAction(tr("&Logarithmic transform"));
   _lt->setCheckable(true);
   _et=addAction(tr("&Exponential transform"));
   _et->setCheckable(true);
   
   _currentSelection=_st;
   _currentSelection->setChecked(true);
       
   connect(this, SIGNAL(triggered(QAction*)), this, SLOT(checkItem(QAction*)));
   connect(this, SIGNAL(dynamic(int)), model,SLOT(changeDynamic(int)));
}

void DynamicSelector::checkItem( QAction *act ) {
   act->setChecked(true);
   if (act!=_currentSelection) {
      _currentSelection->setChecked(false);
      _currentSelection=act;
      if (act==_st)
	 emit dynamic(ImageModel::linear);
      else if (act==_eq)
	 emit dynamic(ImageModel::equalization);
      else if (act==_lt)
	 emit dynamic(ImageModel::logarithmic);
      else 
	 emit dynamic(ImageModel::exponential);
   }
}
