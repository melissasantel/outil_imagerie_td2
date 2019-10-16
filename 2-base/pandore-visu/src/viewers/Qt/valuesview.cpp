/* -*- mode: c++; c-basic-offset: 3 -*-
 *
 * PANDORE (PANTHEON Project)
 *
 * GREYC IMAGE
 * 6 Boulevard Marechal Juin
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
 * @author Régis Clouard - 2006-02-24 (Allow the cursor to go to the last row or column of the image).
 * @author Régis Clouard - 2007-04-04 (display coordinates inside the window)
 * @author Régis Clouard - Aug 13, 2011 (change font size)
 */

/**
 * @file valuesview.cpp
 *
 * Creates a window for displaying the pixel values
 * belonging to the selected area.
 */

#include "valuesview.h"

ValuesView::ValuesView( const ImageModel *model ) : _model(model){
   setAttribute(Qt::WA_QuitOnClose,false);
   Qt::WindowFlags flags = this->windowFlags();
   this->setWindowFlags(flags|Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
   QFont f=font();
   f.setPointSize(10);
   setFont(f);

   _hFont=font().pointSize();
   if (_hFont==-1)
      _hFont=font().pixelSize();
   else
      _hFont+=2;
   _yshift=_hFont;
   setMinimumWidth(10+(80*NBCOL)+2*(NBCOL-1));
   setMaximumWidth(10+(80*NBCOL)+2*(NBCOL-1));
   setMinimumHeight(_yshift+10+2*(NBROW-1)+(3*_hFont+1)*NBROW);
   setMaximumHeight(_yshift+10+2*(NBROW-1)+(3*_hFont+1)*NBROW);
   
   _title=_model->windowTitle()+(" - Values");
   switch(_model->getImageSource()->getTypeval()){
   case ImageSource::tchar:
   case ImageSource::tlong:
      strcpy(_format,"\%11.0f\0");
      break;
   case ImageSource::tfloat:
      strcpy(_format,"\%# 11.5g\0");
      break;
   }
   
   setWindowTitle(_title);
   connect(_model,SIGNAL(askForUpdate()),this,SLOT(update()));
}

void ValuesView::update( ){
   if (isVisible())
      repaint();
}

void ValuesView::paint( ){
   float zoom=_model->getZoom();
   int plane=_model->getPlane();
   long x=_model->CurrentPosX();
   long y=_model->CurrentPosY();
   const ImageSource *ims=_model->getImageSource();
   char tp[120];
   
   if (isVisible()) {
      QPainter painter(this);
      // Display coordinates
      painter.setPen(QColor(0,0,0));
      sprintf(tp,"[ x: %ld",x);
      painter.drawText(30,_hFont,QString(tp));

      sprintf(tp,", y: %ld",y);
      painter.drawText(100,_hFont,QString(tp));

      sprintf(tp,", z: %d ]",plane);
      painter.drawText(170,_hFont,QString(tp));

      for (int i=0; i<NBCOL; i++) {
	 for (int j=0; j<NBROW; j++) {
 	    // If some pixels are out of bounds.
 	    if ((int)(x+i)>=ims->Width() || (int)(y+j)>=ims->Height()) {
	       painter.fillRect(5+(82*i),_yshift+5+((3*_hFont+3)*j),80,(3*_hFont+1),Qt::white);
	       for (int b=0;b<ims->Bands();b++){
		  painter.setPen(QColor(0,0,0));
		  painter.drawText(5+(82*i),_yshift+5+2*_hFont+((3*_hFont+3)*j),QString("-"));
	       }
	    } else {
	       QRgb temp=(_model->getImagePhysic()).pixel((int)((x+i)*zoom),(int)((y+j)*zoom));
	       painter.fillRect(5+(82*i),_yshift+5+((3*_hFont+3)*j),80,(3*_hFont+1),QColor(temp));
	       char t[20];
	       
	       for (int b=0;b<ims->Bands();b++){
		  sprintf(t,_format,(*ims)[b][plane][y+j][x+i]);
		  if (qGray(temp)<127)
		     painter.setPen(QColor(255,255,255));
		  else 
		     painter.setPen(QColor(0,0,0));
		  if (!ims->isColored())
		     painter.drawText(5+(82*i),_yshift+4+2*_hFont+((3*_hFont+3)*j),QString(t));
		  else
		     painter.drawText(5+(82*i),_yshift+4+(b+1)*_hFont+((3*_hFont+3)*j),QString( t));
	       }
	    }
	 }
      }
   }
}

void ValuesView::paintEvent( QPaintEvent *e ){
   e->accept();
   paint();
}
