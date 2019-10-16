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
 * @author Régis Clouard - 2006-01-17
 */

/**
 * @file planeselector.cpp
 *
 * Defines a controller for changing the current plane of a 3D image.
 */

#include "planeselector.h"

PlaneSelector::PlaneSelector( const ImageModel *model, QWidget* parent ): QWidget(parent) {
   QGridLayout *grille=new QGridLayout(this);
   _slider=new QSlider(Qt::Horizontal, this);
   _slider->setFixedHeight(10);
   _slider->setMaximum(model->getImageSource()->Depth()-1);
   _slider->setMinimum(0);
   _slider->setValue(0);
   _slider->setSingleStep(1);
   _slider->setPageStep (1);
   grille->addWidget(_slider,0,0,0);
   
   _number=new QLCDNumber(4,this);
   _number->setSegmentStyle(QLCDNumber::Flat);
   _number->display((int)model->getPlane());
   grille->addWidget(_number,0,1,0);
   if((model->getImageLogic())->Depth()==1){
      _number->display(1);
      setEnabled(false);
   }
   
   // Sets to the displayed number with the new value of the plane.
   connect(_slider,SIGNAL(valueChanged(int)), _number,SLOT(display(int)));
   connect(_slider,SIGNAL(valueChanged(int)), model,SLOT(changePlane(int)));
   connect(model,SIGNAL(changeNumberPlane(long)), this,SLOT(updateNbPlane(long)));
   connect(_slider,SIGNAL(sliderPressed()),this,SLOT(addFocus())); 
}  

void PlaneSelector::addFocus() {
   _slider->setFocus();
}

void PlaneSelector::mousePressEvent( QMouseEvent *e ) {
   e->accept();
   _slider->setFocus();
}

void PlaneSelector::updateNbPlane( long nbPlane ) {
   // Puts the maximum values of slider to the number of planes of the new image.
   _slider->setMaximum((int)nbPlane-1); 
   _slider->setValue(0);
}
