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
 * @author Régis Clouard - 2006-01-17 (fix bug on long images)
 * @author Régis Clouard - Jun 05, 2010 (threshold for color images)
 */

/**
 * @file thresholdselector.cpp
 *
 * Defines a class for thresholding the displayed gray image.
 */

#include "thresholdselector.h"

ThresholdSelector::ThresholdSelector( const ImageModel *model ) {
   setAttribute(Qt::WA_QuitOnClose,false);
   Qt::WindowFlags flags = this->windowFlags();
   this->setWindowFlags(flags|Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

   QString temp=model->windowTitle();
   temp.append(" - Thresholding");
   setWindowTitle(temp);

   const ImageSource *ims=model->getImageSource();
   _min=ims->getMinval();
   _step=(ims->getMaxval()-ims->getMinval())/MAXNSTEP;

   QWidget *central=new QWidget(this);
   setCentralWidget(central);
   QVBoxLayout *layout=new QVBoxLayout(central);

   for (int b=ims->Bands(); b<3; b++ ) {
      _sliderH[b]=0;
      _sliderL[b]=0;
      _numberL[b]=0;
      _numberH[b]=0;
   }
   char t[20];
   switch (ims->getTypeval()) {
   case ImageSource::tchar:
   case ImageSource::tlong:
      strcpy(_format,"\%.0f");
      for (int b=0; b<ims->Bands(); b++ ) {
	 _thresholdL[b]=ims->getMinval();
	 _thresholdH[b]=ims->getMaxval();
	 _normseuil=1;
	 sprintf(t,_format,_thresholdL[b]);
	 _numberL[b]=new QLabel(t,central);
	 layout->addWidget(_numberL[b]);
 
	 _sliderL[b]=new QSlider(Qt::Horizontal, central);
 	 _sliderL[b]->setMinimum((int)_thresholdL[b]);
 	 _sliderL[b]->setMaximum((int)_thresholdH[b]);
 	 _sliderL[b]->setSingleStep(1);
 	 _sliderL[b]->setPageStep(1);
	 _sliderL[b]->setValue((int)_thresholdL[b]);
 	 _sliderL[b]->setTracking(false);
	 layout->addWidget(_sliderL[b]);
	 
	 sprintf(t,_format,_thresholdH[b]);
	 _numberH[b]=new QLabel(t,central);
	 _numberH[b]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	 layout->addWidget(_numberH[b]);

	 _sliderH[b]=new QSlider(Qt::Horizontal,central);
 	 _sliderH[b]->setMinimum((int)_thresholdL[b]);
 	 _sliderH[b]->setMaximum((int)_thresholdH[b]);
 	 _sliderH[b]->setSingleStep(1);
 	 _sliderH[b]->setPageStep(1);
 	 _sliderH[b]->setValue((int)_thresholdH[b]);
 	 _sliderH[b]->setTracking(false);
	 layout->addWidget(_sliderH[b]);
      }
      break;

   case ImageSource::tfloat:
      strcpy(_format,"\%.3g");
      for (int b=0; b<ims->Bands(); b++ ){
	 _thresholdL[b]=ims->getMinval();
	 _thresholdH[b]=ims->getMaxval();
	 _normseuil=0;
	 sprintf(t,_format,_thresholdL[b]);
	 _numberL[b]=new QLabel(t,central);
	 layout->addWidget(_numberL[b]);

	 _sliderL[b]=new QSlider(Qt::Horizontal,central);
	 _sliderL[b]->setMinimum(0);
	 _sliderL[b]->setMaximum(MAXNSTEP);
	 _sliderL[b]->setSingleStep(1);
	 _sliderL[b]->setPageStep(1);
 	 _sliderL[b]->setValue(0);
 	 _sliderL[b]->setTracking(false);
	 layout->addWidget(_sliderL[b]);
	 
	 sprintf(t,_format,_thresholdH[b]);
	 _numberH[b]=new QLabel(t,central);
	 _numberH[b]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	 layout->addWidget(_numberH[b]);
	 
	 _sliderH[b]=new QSlider(Qt::Horizontal,central);
	 _sliderH[b]->setMinimum(0);
	 _sliderH[b]->setMaximum(MAXNSTEP);
	 _sliderH[b]->setSingleStep(1);
	 _sliderH[b]->setPageStep(1);
 	 _sliderH[b]->setValue(MAXNSTEP);
 	 _sliderH[b]->setTracking(false);
	 layout->addWidget(_sliderH[b]);
      }
      break;
   }
   
#ifdef DEBUG
   printf("min =%f max = %f\n",_thresholdL[0],_thresholdH[0]);
   printf("min =%f max = %f\n",_thresholdL[1],_thresholdH[1]);
   printf("min =%f max = %f\n",_thresholdL[2],_thresholdH[2]);
#endif

   connect(_sliderL[0],SIGNAL(valueChanged(int)),this,SLOT(thresholdL0(int)));
   connect(_sliderH[0],SIGNAL(valueChanged(int)),this,SLOT(thresholdH0(int)));
   if (ims->Bands()>1) {
      connect(_sliderL[1],SIGNAL(valueChanged(int)),this,SLOT(thresholdL1(int)));
      connect(_sliderH[1],SIGNAL(valueChanged(int)),this,SLOT(thresholdH1(int)));
      connect(_sliderL[2],SIGNAL(valueChanged(int)),this,SLOT(thresholdL2(int)));
      connect(_sliderH[2],SIGNAL(valueChanged(int)),this,SLOT(thresholdH2(int)));
   }
   connect(this,SIGNAL(lowThreshold(int,float)),model,SLOT(changeThresholdL(int,float)));
   connect(this,SIGNAL(highThreshold(int,float)),model,SLOT(changeThresholdH(int,float)));
}

void ThresholdSelector::thresholdL0( int threshold ) {
   thresholdL(0,threshold);
}
void ThresholdSelector::thresholdH0( int threshold ) {
   thresholdH(0,threshold);
}
void ThresholdSelector::thresholdL1( int threshold ) {
   thresholdL(1,threshold);
}
void ThresholdSelector::thresholdH1( int threshold ) {
   thresholdH(1,threshold);
}
void ThresholdSelector::thresholdL2( int threshold ) {
   thresholdL(2,threshold);
}
void ThresholdSelector::thresholdH2( int threshold ) {
   thresholdH(2,threshold);
}

void ThresholdSelector::thresholdL( int band, int threshold ) {
   char t[20];

   if (_normseuil!=0) {
      _thresholdL[band]=(float)(threshold/_normseuil);
   } else {
      _thresholdL[band]=(float)(threshold*_step+_min);
   }
   sprintf(t,_format,_thresholdL[band]);
   _numberL[band]->setText(QString(t));
   emit lowThreshold(band,_thresholdL[band]);
}

void ThresholdSelector::thresholdH( int band, int threshold ) {
   char t[20];

   if (_normseuil!=0) {
      _thresholdH[band]=(float)(threshold/_normseuil);
   } else {
      _thresholdH[band]=(float)(threshold*_step+_min);
   }
   sprintf(t,_format,_thresholdH[band]);
   _numberH[band]->setText(QString(t));
   emit highThreshold(band,_thresholdH[band]);
}
