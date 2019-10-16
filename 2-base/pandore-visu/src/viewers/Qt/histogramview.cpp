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
 * @author Regis Clouard - 2006-01-16 (fix bug on paint())
 */

/**
 * @file histogramview.cpp
 *
 * Creates a window for displaying an histogram, where
 * the values of pixels are represented according to their occurences. 
 */

#include "histogramview.h"

/**
 * Need initialization of _currentPlane to -1.
 */
HistogramView::HistogramView( const ImageModel* model ): _model(model),_thresholdL(0),_thresholdH(0),_currentPlane(-1) {
   Qt::WindowFlags flags = this->windowFlags();
   this->setWindowFlags(flags|Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);
   QFont f=font();
   f.setPointSize(10);
   setFont(f);

   setAttribute(Qt::WA_QuitOnClose,false);
   setMinimumWidth(80+HISTWIDTH+40);
   setMinimumHeight(10+(15+HISTHEIGHT+40)*_model->getImageSource()->Bands());
   _status=statusBar();

   // Title
   QString t=_model->windowTitle();
   t.append(" - Histogram");
   setWindowTitle(t);
   
   if (_model->getImageSource()->getTypeval()==ImageSource::tfloat)
      strcpy(_format,"\%.3g");
   else
      strcpy(_format,"\%.0f");
   connect(_model,SIGNAL(askForUpdate()), this, SLOT(update()));
   // Force update() to build the first histogram (need that currentPlane=-1).
   update();
}

/**
 * Modifies the current histogram from a new plane or new threshold bounds.
 */
void HistogramView::update( ) {
   if (_thresholdL!=_model->getThresholdL(0) || _thresholdH!=_model->getThresholdH(0) || _currentPlane!=_model->getPlane()) {
      const ImageSource *ims=_model->getImageSource();
      const float min=ims->getMinval();
      const float max=ims->getMaxval();
      _thresholdL=_model->getThresholdL(0);
      _thresholdH=_model->getThresholdH(0);
      _currentPlane=_model->getPlane();
      
      if (_thresholdH>max)
	 _thresholdH=max; // Possible? TO BE DELETE

#ifdef DEBUG
      fprintf(stderr,"Threshold low=%f Threshold high=%f\n",_thresholdL,_thresholdH);
#endif

      if (ims->getTypeval() == ImageSource::tchar) {
	 _hbeg = 0;
 	 _hend = HISTWIDTH-1;
	 _step = 1.0F;
      } else if (ims->getTypeval() == ImageSource::tfloat) {
	 _hbeg=_thresholdL;
	 _hend=_thresholdH;
	 _step=(_hend-_hbeg+1)/HISTWIDTH;
      } else { // slong
	 // Calculate "bin" width.
	 if (min>=0 && (int)max<HISTWIDTH) { // Best case = Uchar case!
	    _hbeg = 0;
	    _hend = HISTWIDTH-1;
	    _step = 1.0F;
	 } else if (_thresholdL>=0 && (int)_thresholdH<HISTWIDTH) {
	    _hbeg = 0;
	    _hend = HISTWIDTH-1;
	    _step = 1.0F;
	 } else if ((int)(_thresholdH-_thresholdL)<= HISTWIDTH) {
	    _hbeg = _thresholdL;
	    _hend = _thresholdL+HISTWIDTH;
	    _step = 1.0F;
	 } else { // worse case.
	    _hbeg=_thresholdL;
	    _hend=_thresholdH;
	    _step=(_hend-_hbeg+1)/HISTWIDTH;
	 }
      }
      
#ifdef DEBUG
      printf("hbeg=%f hend=%f step(bin width)=%f\n",_hbeg,_hend,_step);
#endif
      
      for (int b=0; b<ims->Bands(); b++) {  
	 int i;
	 memset(_histDraw[b], 0, HISTWIDTH*sizeof(int));
	 //compute histogram
	 const Float *ps= &(*ims)[b][_currentPlane][0][0];
	 for (i=0; i<ims->Height()*ims->Width(); i++,ps++) {
	    if (*ps >= _thresholdL && *ps <= _thresholdH)
	       _histDraw[b][(int)((*ps - _hbeg)/_step)]++;
	 }
	 
	 _hmax[b]=_histDraw[b][0];
	 _imax[b]=0;
	 for (int w=1;w<HISTWIDTH;w++) {
	    if (_histDraw[b][w]>_hmax[b]) {
	       _hmax[b]=_histDraw[b][w];
	       _imax[b]=w;
	    }
#ifdef DEBUG
	    printf("histo[%d]=%d\n",w,_histDraw[b][w]);
#endif
	 }
	 if (isVisible())
	    repaint();
      }
   }
}

/**
 * Needs computation of _histDraw.
 */
void HistogramView::paint( ) {
   QPainter painter(this);
   painter.eraseRect(0,0,width(),height());

   for (int b=0; b<_model->getImageSource()->Bands(); b++) {
      
      const int X0=80;
      const int Y0=115+(15+HISTHEIGHT+40)*b;
      
      // HISTOGRAM(S)
      if (_hmax[b]!=0) {
	 for (int i=0; i<HISTWIDTH; i++) {
	    if (_model->getImageSource()->Bands() == 1) // Gray levels.
	       painter.setPen(QColor(0,0,255));
	    else { // Colors
	       if (b==0)
		  painter.setPen(QColor(255,0,0));
	       else if (b==1)
		  painter.setPen(QColor(0,255,0));
	       else 
		  painter.setPen(QColor(0,0,255));
	    }
	    int val=(int)((float)(_histDraw[b][i]*100.0)/(float)_hmax[b]);
	    painter.drawLine(X0+i,Y0,X0+i,Y0-val);
	 }
      }
      // AXES:  keep them here to overload the border line of the histogram...
      painter.setPen(QColor(0,0,0));
      char t[20];
      // 1) y-axis
      painter.drawLine(X0-1,Y0-HISTHEIGHT-5,X0-1,Y0+4);
      // min legend
      painter.drawText(0,Y0-8,65,Y0-8,Qt::AlignRight,QString("0"));

      // 2) x-axis
      painter.drawLine(X0-4,Y0,X0+HISTWIDTH+4,Y0);
      // xtics (min and max)
      painter.drawLine(HISTWIDTH+80,Y0+1,HISTWIDTH+80,Y0+5);
      sprintf(t,_format,_hbeg);
      painter.drawText(0,Y0,X0,Y0+10,Qt::AlignRight,QString(t));
      sprintf(t,_format,_hend);
      painter.drawText(0,Y0,X0+HISTWIDTH+16,Y0+10,Qt::AlignRight,QString(t));

      // MAX VALUES
      painter.setPen(QColor(255,0,0));
      // max y-axis
      painter.drawLine(X0-6,Y0-HISTHEIGHT,X0-2,Y0-HISTHEIGHT);
      sprintf(t,"%d",_hmax[b]);
      painter.drawText(0,4+Y0-115,X0-8,25+Y0-115,Qt::AlignRight,QString(t));
      // max x-axis
      painter.drawLine(X0+_imax[b],Y0+1,X0+_imax[b],Y0+6);
      sprintf(t,_format,_imax[b]*_step+_hbeg);
      painter.drawText(0,Y0+11,X0+20+_imax[b],Y0+30,Qt::AlignRight,QString(t));
   }
}

/**
 *
 */
void HistogramView::paintEvent( QPaintEvent *e ) {
   e->accept();
   paint();
}

/**
 * 
 */
void HistogramView::mousePressEvent( QMouseEvent *e ) {
   const int X0=80;

   int x=e->x()-X0;
   int y=e->y();
   
   if (x<0) x=0;
   if (x>=HISTWIDTH) x=HISTWIDTH-1;
   
   float hx1=x*_step+_hbeg;
   
   char t[120];
   char t2[64];
   
   if ( _step==1.0F ) {
      sprintf(t2,"histogram[%s] = %%d",_format);
      if ( y<=115 && y>=15 )
	 sprintf(t,t2,hx1,_histDraw[0][x]);
      else if ( y>=170 && y<=270 )
	 sprintf(t,t2,hx1,_histDraw[1][x]);
      else if ( y>=325 && y<=425 )
	 sprintf(t,t2,hx1,_histDraw[2][x]);
      else
	 strcpy(t,"\0");
   } else {
      float hx2=hx1+_step;
      sprintf(t2,"histogram[%s,%s] = %%d",_format,_format);
      if(y<=115 && y>=15)
	 sprintf(t,t2,hx1,hx2,_histDraw[0][x]);
      else if(y>=170 && y<=270)
	 sprintf(t,t2,hx1,hx2,_histDraw[1][x]);
      else if(y>=325 && y<=425)
	 sprintf(t,t2,hx1,hx2,_histDraw[2][x]);
      else
	 strcpy(t,"\0");
   }
   _status->showMessage(tr(t));
}

/**
 *
 */
void HistogramView::mouseMoveEvent( QMouseEvent *e ) {
   mousePressEvent(e);
}

/**
 *
 */
void HistogramView::mouseReleaseEvent( QMouseEvent *e ){
   e->accept();
   _status->clearMessage();
}
