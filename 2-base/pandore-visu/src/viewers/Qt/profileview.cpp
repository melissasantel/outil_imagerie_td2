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
 * @file profileview.cpp
 *
 * Creates a window for displaying an image profile,
 * where the values of pixel of the column or the row  selected are represented
 * as a bar.
 */

#include "profileview.h"

ProfileView::ProfileView( const ImageModel *model, bool row ): _model(model), _row(row) {
   const ImageSource *ims=_model->getImageSource();
   bool colored=ims->isColored();
   
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

   if (_row) {
      setMinimumWidth(ims->Width()+10);
      if (colored) {
	 _h=64;
	 setMinimumHeight(_h*3+20+_hFont);
      } else {
	 _h=128;
	 setMinimumHeight(_h+10);}
   } else {
      setMinimumHeight(ims->Height()+10+_hFont);
      if (colored) {
	 _h=64;
	 setMinimumWidth(_h*3+20);
      } else {
	 _h=128;
	 setMinimumWidth(_h+10);
      }
   }
   
   _title=_model->windowTitle()+" - Profile";
   
   setWindowTitle(QString(_title));
   connect(_model,SIGNAL(askForUpdate()), this,SLOT(update()));
}


void ProfileView::update( ) {
   if(isVisible())
      repaint();
}

void ProfileView::paint( ) {
   QPainter painter(this);
   painter.eraseRect(0,0,width(),height());
   const ImageSource* ims=_model->getImageSource();
   int plane=_model->getPlane();
   long posX=_model->CurrentPosX();
   long posY=_model->CurrentPosY();
   float max=ims->getMaxval();
   float min=ims->getMinval();
   float step;
   if(max==min)
      step=_h;
   else
      step=_h/(max-min);
   
   char t1[127];
   if (_row){
      sprintf(t1,"row: %ld",posY);
   } else {
      sprintf(t1,"column: %ld",posX);
   }
   painter.setPen(QColor(0,0,0));
   painter.drawText(30,_hFont,QString(t1));

   for (int b=0; b<ims->Bands();b++){
      if (ims->isColored())
	 if (b==0)
	    painter.setPen(QColor(255,0,0));
	 else if (b==1)
	    painter.setPen(QColor(0,255,0));
	 else 
	    painter.setPen(QColor(0,0,255));
      else
	 painter.setPen(QColor(0,0,0));
      if (_row)
	 for (int x=0;x<ims->Width();x++) {
	    float niv=(*ims)[b][plane][posY][x];
	    painter.drawLine(5+x,_hFont+(b+1)*(_h+5),5+x,_hFont+(int)((float)((b+1)*(_h+5))-(niv-min)*step));
	 }
      else
	 for (int y=0;y<ims->Height();y++) {
	    float niv=(*ims)[b][plane][y][posX];
	    painter.drawLine(5+(_h+5)*b,_hFont+5+y,(int)(5+(_h+5)*b+(niv-min)*step),_hFont+5+y);
	 }
   }
} 

void ProfileView::paintEvent(QPaintEvent *e) {
   e->accept();
   paint();
}
