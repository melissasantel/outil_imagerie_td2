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
 * @file colorselector.cpp
 * @brief Defines a controller for acting on the color of the display image.
 */

/**
 * @author Nicolas Briand - 2005-01-06
 */

#include "colorselector.h"

ColorSelector::ColorSelector( const ImageModel *model ){
   setAttribute(Qt::WA_QuitOnClose,false);
   Qt::WindowFlags flags = this->windowFlags();
   this->setWindowFlags(flags|Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

   QString temp=model->windowTitle();
   temp.append(" - Colors");
   setWindowTitle(temp);
   QWidget *central = new QWidget(this);
   setCentralWidget(central);
   QVBoxLayout *position = new QVBoxLayout(central);
  
   _brightTitle=new QLabel("brightness",central);
   _brightTitle->setAlignment(Qt::AlignHCenter);
   position->addWidget(_brightTitle);
   _brightNumber=new QLCDNumber(4,central);
   _brightNumber->setSegmentStyle(QLCDNumber::Flat);
   _brightNumber->display(0);
   position->addWidget(_brightNumber);
   _brightSlider = new QSlider(Qt::Horizontal,central);
   _brightSlider->setMinimum(-127);
   _brightSlider->setMaximum(127);
   _brightSlider->setValue(0);
   _brightSlider->setSingleStep(1);
   _brightSlider->setTracking(false);
   position->addWidget(_brightSlider);
  
   _contrastTitle=new QLabel("contrast",central);
   _contrastTitle->setAlignment(Qt::AlignHCenter);
   position->addWidget(_contrastTitle);
   _contrastNumber=new QLCDNumber(4,central);
   _contrastNumber->setSegmentStyle(QLCDNumber::Flat);
   _contrastNumber->display(0);
   position->addWidget(_contrastNumber);
   _contrastSlider = new QSlider(Qt::Horizontal,central);
   _contrastSlider->setMinimum(-127);
   _contrastSlider->setMaximum(127);
   _contrastSlider->setValue(0);
   _contrastSlider->setSingleStep(1);
   _contrastSlider->setTracking(false);
   position->addWidget(_contrastSlider);
  
   QWidget *buttons=new QWidget(central);
  
   QHBoxLayout *color=new QHBoxLayout(buttons);
   _red = new QCheckBox("Red",buttons);
   color->addWidget(_red);
   _green = new QCheckBox("Green",buttons);
   color->addWidget(_green);
   _blue = new QCheckBox("Blue",buttons);
   color->addWidget(_blue);
   position->addWidget(buttons);
   _red->setChecked(true);
   _green->setChecked(true);
   _blue->setChecked(true);

   QWidget *lut=new QWidget(central);
   _luts=new QButtonGroup(central);
   _luts->setExclusive(true);

   QGridLayout *grid=new QGridLayout(lut);
   _original=new QRadioButton("Original  ",lut);
   _luts->addButton(_original);
   grid->addWidget(_original,0,0);
   _negatif=new QRadioButton("Negatif  ",lut);
   _luts->addButton(_negatif);
   grid->addWidget(_negatif,0,1);
   _random=new QRadioButton("Random  ",lut);
   _luts->addButton(_random);
   grid->addWidget(_random,0,2);
   _region=new QRadioButton("Region  ",lut);
   _luts->addButton(_region);
   grid->addWidget(_region,1,0);
   _rainbow=new QRadioButton("Rainbow  ",lut);
   _luts->addButton(_rainbow);
   grid->addWidget(_rainbow,1,1);
   _noise=new QRadioButton("Noise  ",lut);
   _luts->addButton(_noise);
   grid->addWidget(_noise,1,2);
   _original->setChecked(true);
   position->addWidget(lut);
  
   _reset=new QPushButton("Reset",central);
   position->addWidget(_reset);
 
  
   connect(_red,SIGNAL(toggled(bool)),model,SLOT(setRed(bool)));
   connect(_green,SIGNAL(toggled(bool)),model,SLOT(setGreen(bool)));
   connect(_blue,SIGNAL(toggled(bool)),model,SLOT(setBlue(bool)));
   connect(_brightSlider,SIGNAL(valueChanged(int)),
	   _brightNumber,SLOT(display(int)));
   connect(_contrastSlider,SIGNAL(valueChanged(int)),
	   _contrastNumber,SLOT(display(int)));
   connect(_brightSlider,SIGNAL(valueChanged(int)),
	   model, SLOT(setBrightness(int)));
   connect(_contrastSlider,SIGNAL(valueChanged(int)),
	   model, SLOT(setContrast(int)));
   connect(_luts,SIGNAL(buttonClicked(QAbstractButton*)),
	   this, SLOT(selectLut(QAbstractButton*)));
   connect(_reset,SIGNAL(pressed()),this,SLOT(resetSettings()));
   connect(this,SIGNAL(reset()),model,SLOT(resetColor()));
   connect(this,SIGNAL(newLut(int)),model,SLOT(changeLut(int)));
}

void ColorSelector::selectLut( QAbstractButton *button ) {
   if(button==_original)
      emit newLut(ImageModel::original);
   else if(button==_negatif)
      emit newLut(ImageModel::negatif);
   else if(button==_region)
      emit newLut(ImageModel::region);
   else if(button==_random)
      emit newLut(ImageModel::random);
   else if(button==_rainbow)
      emit newLut(ImageModel::rainbow);
   else if(button==_noise)
      emit newLut(ImageModel::noise);
}

void ColorSelector::resetSettings( ) {
   _brightSlider->blockSignals(true);
   _brightSlider->setValue(0);
   _brightNumber->display(0);
   _brightSlider->blockSignals(false);
   _contrastSlider->blockSignals(true);
   _contrastSlider->setValue(0);
   _contrastNumber->display(0);
   _contrastSlider->blockSignals(false);
   _red->blockSignals(true);
   _green->blockSignals(true);
   _blue->blockSignals(true);
   _red->setChecked(true);
   _green->setChecked(true);
   _blue->setChecked(true);
   _red->blockSignals(false);
   _green->blockSignals(false);
   _blue->blockSignals(false);
   if(!_original->isChecked())
      _original->setChecked(true);
   emit reset();
}
