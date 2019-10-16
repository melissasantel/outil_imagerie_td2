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
 * @author Nicolas Briand - 2005-10-05
 * @author Régis Clouard - 2005-10-25 (change drawing facilities)
 */

/**
 * @file canvasview.cpp
 *
 * Defines the class that paints the physic image.
 */

#include "canvasview.h"

/**
 * Tries to open a window from the the size of the input image.
 */
CanvasView::CanvasView( ImageModel *model, QWidget *parent ): QWidget(parent), _model(model) {
   _currentPosX=0;
   _currentPosY=0;
   _pixmap=new QPixmap(QPixmap::fromImage(_model->getImagePhysic(),Qt::ColorOnly));
   this->resize ((int)(_pixmap->width()),(int)(_pixmap->height()));
   connect(_model,SIGNAL(askForUpdate()),this,SLOT(update()));
   connect(_model,SIGNAL(draw()),this,SLOT(repaint()));
}

void CanvasView::update() {
   if (_currentPosX!=_model->CurrentPosX() || _currentPosY!=_model->CurrentPosY()){
      // Just redraw the cursor.
      _currentPosX=_model->CurrentPosX();
      _currentPosY=_model->CurrentPosY();
   } else {
      delete _pixmap;
      // Build a new pixmap from new visualisation parameters.
      QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
      _pixmap=new QPixmap(QPixmap::fromImage(_model->getImagePhysic(),Qt::ColorOnly));
      this->resize ((int)(_pixmap->width()),(int)(_pixmap->height()));
      QApplication::restoreOverrideCursor();
   }
   if (isVisible())
      repaint();
}

void CanvasView::paint( QPaintEvent *e ) {
   QPainter painter(this);
   painter.drawPixmap(e->rect().topLeft(),*_pixmap,e->rect());
   painter.setPen(QColor(255,0,0));
   float z=_model->getZoom();
   // Draw the cursor.
   if (_model->cursorIsVisible()) {
      painter.drawLine((int)(_currentPosX*z),0,(int)(_currentPosX*z),_pixmap->height());
      painter.drawLine(0,(int)(_currentPosY*z),_pixmap->width(),(int)(_currentPosY*z));
      painter.drawRect((int)(_currentPosX*z),(int)(_currentPosY*z),(int)(6*z),(int)(6*z));
   }

   // Draw the current drawing.
   if (_DRAWINGMODE) {
      Imx3duc *imd=_model->getImageDraw();
      long plane=_model->getPlane();
      int h=(int)((e->rect().top()+e->rect().height())/z);
      if(h>imd->Height())
	 h=imd->Height();
      int w=(int)((e->rect().left()+e->rect().width())/z);
      if(w>imd->Width())
	 w=imd->Width();

      for (int y=(int)(e->rect().top()/z); y<h; y++)
	 for(int x=(int)(e->rect().left()/z); x<w; x++)
	    if((*imd)(0,plane,y,x)!=0)
	       for(int i=0; i<z; i++)
		  for(int j=0; j<z; j++) {
 		     painter.drawPoint((int)(x*z+i),(int)(y*z+j));
		  }
   }
}

void CanvasView::paintEvent( QPaintEvent *e ) {
   paint(e);
}

void CanvasView::mousePressEvent( QMouseEvent *e ) {
   setFocus();
   float z=_model->getZoom();
   int x=(int)(e->x()/z);
   int y=(int)(e->y()/z);
   if (_DRAWINGMODE) {
      _lastDrawingX=x;
      _lastDrawingY=y;
      _model->cleanLastDraw(); // For undo!
      _model->drawLine(x,y,x,y);
      if (z<1.0F) z=1.0F;
      repaint((int)(x*z),(int)(y*z),(int)z,(int)z);
   } else {
      if (x!=_currentPosX || y!=_currentPosY)
	 _model->setPosition(x,y);
   }
}

void CanvasView::mouseMoveEvent( QMouseEvent *e ) {
   if (_DRAWINGMODE) {
      float z=_model->getZoom();
      Imx3duc *imd=_model->getImageDraw();
      int x=(int)(e->x()/z);
      int y=(int)(e->y()/z);
      if (x<0 || y<0 || x>= imd->Width()|| y>= imd->Height())
	 return;
      _model->drawLine(_lastDrawingX,_lastDrawingY,x,y);

      // redraw only the line.
      if (z<1.0F) z=1.0F;
      int xr,yr,wr,hr;
      if (_lastDrawingX>=x) {
	 xr=(int)(x*z);
	 wr=(int)((_lastDrawingX-x+1)*z);
      } else {
	 xr=(int)(_lastDrawingX*z);
	 wr=(int)((x-_lastDrawingX+1)*z);
      }
      if (_lastDrawingY>=y) {
	 yr=(int)(y*z);
	 hr=(int)((_lastDrawingY-y+1)*z);
      } else {
	 yr=(int)(_lastDrawingY*z);
	 hr=(int)((y-_lastDrawingY+1)*z);
      }
      repaint(xr,yr,wr,hr);
      _lastDrawingX=x;
      _lastDrawingY=y;
   }
}

void CanvasView::mouseReleaseEvent( QMouseEvent *e ) {
   if (_DRAWINGMODE) {
      float z=_model->getZoom();
      long x=(int)(e->x()/z);
      long y=(int)(e->y()/z);
      if (x!=_currentPosX || y!=_currentPosY)
	 _model->setPosition(x,y);
   }
}

void CanvasView::keyPressEvent( QKeyEvent *e ) {
   switch(e->key()){
   case Qt::Key_Left :
      _model->setPosition(_currentPosX-1,_currentPosY);
      break;
   case Qt::Key_Right :
      _model->setPosition(_currentPosX+1,_currentPosY);
      break;
   case Qt::Key_Up :
      _model->setPosition(_currentPosX,_currentPosY-1);
      break;
   case Qt::Key_Down :
      _model->setPosition(_currentPosX,_currentPosY+1);
      break;
   default:
      e->ignore();
   }
}
