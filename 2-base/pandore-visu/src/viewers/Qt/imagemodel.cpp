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
 * @author Régis Clouard - 2005-10-25 (change drawing facilities)
 * @author Régis Clouard - 2006-01-17 (add logarithmic and exponential transform)
 * @author Régis Clouard - 2006-02-15 (perform some optimizations)
 * @author Régis Clouard - 2006-02-15 (add thresholding in and thresholding out facilities)
 * @author Régis Clouard - 2007-04-02 (fix bug : case NaN)
 * @author Régis Clouard - Jun 05, 2010 (add "save as")
 */
   
/**
 * @file imagemodel.cpp
 *
 * Defines classe that manipulates image content
 * with observers and tools.
 */

#include <QtGui>
#include <QFileDialog>
#include <limits>
#include "imagemodel.h"
#include "canvasview.h"
#include "planeselector.h"
#include "thresholdselector.h"
#include "zoomselector.h"
#include "dynamicselector.h"
#include "colorselector.h"
#include "valuesview.h"
#include "propertiesview.h"
#include "histogramview.h"
#include "profileview.h"
#include "rotateselector.h"

extern char *nomentree;

ImageModel::ImageModel( const ImageSource *ims, const char *file,Imx3duc *imd ): _imageSource(ims), _imageLogic(0), _lastDrawingPoints() {
   setAttribute(Qt::WA_QuitOnClose,true);
   setMinimumSize(220,150);
   if (imd!=0) {
      _imageDessin=imd;
      _imageDraw=new Img3duc(_imageDessin->Depth(),_imageDessin->Height(),_imageDessin->Width());
      _lastDraw=new Img2duc(_imageDessin->Height(),_imageDessin->Width());
      Uchar *d=_imageDessin->Vector(0);
      Uchar *s=_imageDraw->Vector(0);
      for (;s<_imageDraw->Vector(0)+_imageDraw->VectorSize();) {
	 *(s++)=*(d++);
      }
      _vectX[0]=_vectY[1]=_vectZ[2]=1;
      _vectX[1]=_vectX[2]=_vectY[0]=_vectY[2]=_vectZ[0]=_vectZ[1]=0;
   }
   
   CreateTitle(file);
   if (isnan(_imageSource->getMinval()) ||
       isnan(_imageSource->getMaxval()) ||
       (_imageSource->getMinval()==-std::numeric_limits<float>::infinity()) ||
       (_imageSource->getMaxval()==+std::numeric_limits<float>::infinity())) {
      std::cout << "Invalid image data: NaN pixel value detected" << std::endl;
      exit(0);
   }
    
   // Set default values
   _normalization=ImageModel::linear;

   buildImageLogic();
   _cursor=true;
   _currentPlane=0;
   _currentPosX=0;
   _currentPosY=0;
   _zoom=1.0F;
   _red=1;
   _green=1;
   _blue=1;
   _brightness=0;
   _contrast=0;
   _thresholdL[0]=_thresholdL[1]=_thresholdL[2]=_imageSource->getMinval();
   _thresholdH[0]=_thresholdH[1]=_thresholdH[2]=_imageSource->getMaxval();
     
   buildOriginalLut();
   buildMenu();
   buildImagePhysic(); 
   QWidget *central=new QWidget(this);
   setCentralWidget(central);

   QVBoxLayout *position=new QVBoxLayout(central);
   _planeBar=new PlaneSelector(this,central);
   position->addWidget(_planeBar);

   _scrollImage=new QScrollArea(central);
   //    _scrollImage=new MyScrollArea(central);
   CanvasView *imageShow=new CanvasView(this,this);
   _scrollImage->setWidget(imageShow);
   _scrollImage->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
   _scrollImage->setHorizontalScrollBarPolicy (Qt::ScrollBarAsNeeded);
   _scrollImage->viewport()->setBackgroundRole(QPalette::Dark);
   position->addWidget(_scrollImage);
 
   QImage icon=_imagePhysic.scaled(64,64,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
   setWindowIcon(QPixmap::fromImage(icon));
   _propertiesWindow=new PropertiesView(this);
   _colorWindow=new ColorSelector(this);
   _valuesWindow=new ValuesView(this);
   _histogramWindow=new HistogramView(this);
   _thresholdWindow=new ThresholdSelector(this);
   _profileRWindow=new ProfileView(this,true);
   _profileCWindow=new ProfileView(this,false);
   
   const  int width =  MIN(QApplication::desktop()->width()-20, _imageLogic->Width()+_scrollImage->verticalScrollBar()->height());
   const  int height = MIN(QApplication::desktop()->height()-20, _imageLogic->Height()+2*_scrollImage->horizontalScrollBar()->height()+ _planeBar->height()+15);
   resize(width,height);
   setFocus();
}
    
void ImageModel::CreateTitle( const char *filename ) {
   int i=strlen(filename);
   for (;i>=0;i--) {
      if (filename[i]=='/' || filename[i]=='\\') 
	 break;
   }
   setWindowTitle(QString(&filename[++i]));
}

void ImageModel::buildMenu( ) {
   _menu=menuBar();
   QMenu *file=new QMenu("File");
   file->addAction(tr("&Open"),this, SLOT(openImage()));
   file->addAction(tr("&Save As"),this, SLOT(saveAsImage()));
   file->addSeparator();
   file->addAction(tr("&Properties"),this, SLOT(properties()));
   file->addSeparator();
   file->addAction(tr("&Quit"),this, SLOT(closeApplication()));

   _menu->addMenu(file);   
   QMenu *views = new QMenu(tr("Views"));
   views->addAction(tr("&Values"),this, SLOT(values()));
   views->addAction(tr("&Histogram"),this, SLOT(histo()));
   QMenu *profile = new QMenu(tr("Profile"));
   profile->addAction(tr("&Row"),this, SLOT(row()));
   profile->addAction(tr("&Column"),this, SLOT(column()));
   views->addMenu(profile);
   views->addSeparator();
   _zoomMenu = new ZoomSelector(this);
   views->addMenu(_zoomMenu);

   views->addSeparator();
   QAction *cursor=views->addAction(tr("&Cursor"),this, SLOT(changeCursorStatus()));
   _menu->addMenu(views);

   QMenu *tools = new QMenu(tr("Tools"));
   tools->addAction(tr("&LookUp table"),this, SLOT(colors()));
   _dynamicMenu=new DynamicSelector(this); 
   QAction *dy=tools->addMenu(_dynamicMenu);
   QAction *th=tools->addAction(tr("&Thresholding"),this, SLOT(thresholding()));
   _rotateMenu= new RotateSelector(this);
    QAction *ro=tools->addMenu(_rotateMenu);
   _menu->addMenu(tools);
   
   if (_DRAWINGMODE) {
      QMenu *drawing = new QMenu(tr("Drawing"));
      drawing->addAction(tr("Undo"),this,SLOT(undoDraw()));
      drawing->addAction(tr("Clear"),this,SLOT(clearDraw()));
      QAction *drawMode=drawing->addAction(tr("Pencil mode"),this, SLOT(changePencilStatus()));
      _menu->addMenu(drawing);
      drawMode->setCheckable(true);
      drawMode->setChecked(true);
   }
   cursor->setCheckable(true);
   cursor->setChecked(_pencilStatus=true);
   dy->setEnabled( _imageSource->isDynamisable());
//   th->setEnabled(!_imageSource->isColored());
}

void ImageModel::buildOriginalLut( ) {
   for (int i=0; i<MAXCOLS; i++) {
      if (!_imageSource->isGraph() && !_imageSource->isRegion())
	 _lut[0][i]=_lut[1][i]=_lut[2][i]=(Uchar)i;
      else 
	 buildRegionLut();
   }
}
    
void ImageModel::buildRandomLut( ) {
   for (int i=0; i<MAXCOLS; i++) {
      _lut[0][i]=(Uchar)rand() % MAXCOLS;
      _lut[1][i]=(Uchar)rand() % MAXCOLS;
      _lut[2][i]=(Uchar)rand() % MAXCOLS;
   }
}

const int COLORS_FOR_REGION = 22;

/**
 * Modifies the color with the value i.
 */
inline Uchar ShiftColor( Uchar color, int i) {
   int x = i/COLORS_FOR_REGION;
   return abs(color - 10*x);
}

void ImageModel::buildRegionLut( ) {
   _lut[0][0]=0;
   _lut[1][0]=0;
   _lut[2][0]=0;

   for (int i=1; i<MAXCOLS; i++) { 
      switch(i%COLORS_FOR_REGION) {
      case 0:{
	 // red
	 _lut[0][i]=ShiftColor(255,i);
	 _lut[1][i]=0;
	 _lut[2][i]=0;
      }
    	 break;
      case 1:{
	 // cyan
	 _lut[0][i]=0;
	 _lut[1][i]=ShiftColor(255,i);
	 _lut[2][i]=ShiftColor(255,i);;
      }
	 break;
      case 2:{
	 // blue
	 _lut[0][i]=0;
	 _lut[1][i]=0;
	 _lut[2][i]=ShiftColor(255,i);;
      }
	 break;
      case 3:{
	 // green
	 _lut[0][i]=0;
	 _lut[1][i]=ShiftColor(127,i);
	 _lut[2][i]=0;
      }
	 break;
      case 4:{
	 // dark olive green
	 _lut[0][i]=ShiftColor(85,i);
	 _lut[1][i]=ShiftColor(107,i);
	 _lut[2][i]=ShiftColor(47,i);
      }
	 break;
      case 5:{
	 _lut[0][i]=ShiftColor(127,i);
	 _lut[1][i]=0;
	 _lut[2][i]=ShiftColor(127,i);
      }
	 break;
      case 6:{
	 // chocolat
	 _lut[0][i]=ShiftColor(210,i);
	 _lut[1][i]=ShiftColor(105,i);
	 _lut[2][i]=ShiftColor(30,i);
      }
	 break;
      case 7:{
	 // gray
	 _lut[0][i]=ShiftColor(150,i);
	 _lut[1][i]=ShiftColor(150,i);
	 _lut[2][i]=ShiftColor(150,i);
      }
	 break;
      case 8:{
	 // violet
	 _lut[0][i]=ShiftColor(238,i);
	 _lut[1][i]=ShiftColor(130,i);
	 _lut[2][i]=ShiftColor(238,i);
      }
	 break;
      case 9:{
	 // brown
	 _lut[0][i]=ShiftColor(165,i);
	 _lut[1][i]=ShiftColor(42,i);
	 _lut[2][i]=ShiftColor(42,i);
      }
	 break;
      case 10:{
	 // orange
	 _lut[0][i]=ShiftColor(255,i);
	 _lut[1][i]=ShiftColor(165,i);
	 _lut[2][i]=0;
      }
	 break;
      case 11:{
	 // gold4
	 _lut[0][i]=ShiftColor(139,i);
	 _lut[1][i]=ShiftColor(117,i);
	 _lut[2][i]=0;
      }
	 break;
      case 12:{
	 // mistyrose2
	 _lut[0][i]=ShiftColor(138,i);
	 _lut[1][i]=ShiftColor(213,i);
	 _lut[2][i]=ShiftColor(210,i);
      }
	 break;
      case 13:{
	 // white
	 _lut[0][i]=ShiftColor(255,i);
	 _lut[1][i]=ShiftColor(255,i);
	 _lut[2][i]=ShiftColor(255,i);
      }
	 break;
      case 14:{
	 // magenta
	 _lut[0][i]=ShiftColor(255,i);
	 _lut[1][i]=0;
	 _lut[2][i]=ShiftColor(255,i);
      }
	 break;
      case 15:{
	 _lut[0][i]=ShiftColor(127,i);
	 _lut[1][i]=0;
	 _lut[2][i]=0;
      }
    	 break;
      case 16:{
	 _lut[0][i]=ShiftColor(255,i);
	 _lut[1][i]=ShiftColor(255,i);
	 _lut[2][i]=0;
      }
	 break;

      case 17:{
	 // blue
	 _lut[0][i]=0;
	 _lut[1][i]=0;
	 _lut[2][i]=ShiftColor(127,i);
      }
	 break;
      case 18:{
	 _lut[0][i]=ShiftColor(127,i);
	 _lut[1][i]=ShiftColor(127,i);
	 _lut[2][i]=0;
      }
	 break;
      case 19:{
	 // green
	 _lut[0][i]=0;
	 _lut[1][i]=ShiftColor(255,i);
	 _lut[2][i]=0;
      }
	 break;
      case 20:{
	 _lut[0][i]=0;
	 _lut[1][i]=ShiftColor(127,i);
	 _lut[2][i]=ShiftColor(127,i);
      }
	 break;
      case 21:{
	 _lut[0][i]=ShiftColor(30,i);
	 _lut[1][i]=ShiftColor(105,i);
	 _lut[2][i]=ShiftColor(210,i);
      }
	 break;
      }
   }
}
    
void ImageModel::buildRainbowLut( ) {
   for (int i=0; i<MAXCOLS; i++) {
      int j = MAXCOLS-1-i;
      double d =cos((double)((j-MAXCOLS*0.16) * (M_PI/MAXCOLS)));
      d = (d < 0.0) ? 0.0 : d;
      _lut[2][i] = (Uchar)(d*MAXCOLS);
      d = (d=cos((double)((j-MAXCOLS*0.52) * (M_PI/MAXCOLS)))) < 0.0 ? 0.0 :d;
      _lut[1][i] = (Uchar)(d*MAXCOLS);
      d = (d=cos((double)((j-MAXCOLS*0.82) * (M_PI/MAXCOLS)))) < 0.0 ? 0.0 :d;
      double e =cos((double)((j  *(M_PI/MAXCOLS))));
      e = (e < 0.0)? 0.0 : e;
      _lut[0][i] = (Uchar)(d*MAXCOLS + e *(MAXCOLS/2));
   }
}
    
void ImageModel::buildNoiseLut( ) {
   for (int i=0; i<MAXCOLS; i++) {
      switch(i%3) {
      case 0:
    	 _lut[0][i] = 255;
    	 _lut[1][i] = 0;
    	 _lut[2][i] = 0;
    	 break;
      case 1:
    	 _lut[0][i] = 0;
    	 _lut[1][i] = 255;
    	 _lut[2][i] = 0;
    	 break;
      case 2:
    	 _lut[0][i] = 0;
    	 _lut[1][i] = 0;
    	 _lut[2][i] = 255;
    	 break;
      }
   }
}
    
void ImageModel::buildNegatifLut( ) {
   for (int i=0; i<MAXCOLS; i++)
      _lut[0][i]=_lut[1][i]=_lut[2][i]=255-i;
}

void ImageModel::buildImageLogic( ) {
   if (_imageLogic)
      delete _imageLogic;
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   _imageLogic=new ImageLogic(_imageSource->Bands(),_imageSource->Size());
   if (_imageSource->isGraph()|| _imageSource->isRegion() ){
      regionEncoding();
   } else {
      switch(_normalization) {
      case ImageModel::linear:
      	 linearTransform(MAXUCHAR);
         break;
      case ImageModel::logarithmic:
      	 logTransform(MAXUCHAR);
         break;
      case ImageModel::exponential:
       	 expTransform(MAXUCHAR);
	 break;
      case ImageModel::equalization:
       	 histogramEqualization(MAXUCHAR);
	 break;
      }
   }
   QApplication::restoreOverrideCursor();
}

void ImageModel::regionEncoding( ) {
   Float *ps=_imageSource->Vector(0); 
   Uchar *pl=_imageLogic->Vector(0);
   const Float *end=ps+_imageSource->VectorSize();
   for (;ps<end;ps++,pl++) {
      unsigned long v =(unsigned long)*ps;
      *pl = (Uchar)(v>0)? ((v-1) % (MAXUCHAR-1)) +1: 0;
      //       *pl = (Uchar)(v>0)? (MAXUCHAR-1) +1: 0;
   }
}

void ImageModel::linearTransform( int outbands ) { 
   Float smin=_imageSource->getMinval();
   Float smax=_imageSource->getMaxval();
      
   // Calculate slope (for all bands).
   Float slope;
   if (smin>=0.0F) {
      smin=0.0F;
      if (smax <=outbands) 
    	 slope=1.0F;
      else 
    	 slope =(float)outbands/smax;
   } else {
      if (smax==smin)
    	 slope=1.0F;
      else 
    	 slope=(float)(outbands)/(smax-smin);
   }
    
   // Linear transform.
   for (int b=0;b<_imageSource->Bands();b++) {      
      Float *ps=_imageSource->Vector(b); 
      Uchar *pl=_imageLogic->Vector(b);
      // Rem: With this constant -> we earn half time.
      const Float *end=ps+_imageSource->VectorSize();
      for (;ps<end;ps++,pl++) {
    	 *pl = (Uchar)(slope*(*ps-smin));
      }
   }
}
    
void ImageModel::logTransform( int outbands ) {
   Float smin=_imageSource->getMinval();
   Float smax=_imageSource->getMaxval();
    
   const double c=((double)outbands / (log(smax-smin+1.0)));
   for (int b=0;b<_imageSource->Bands();b++) {
      Float *ps=_imageSource->Vector(b); 
      Uchar *pl=_imageLogic->Vector(b);
      const Float *end=ps+_imageSource->VectorSize();
      for (;ps<end;ps++,pl++)
    	 *pl=(Uchar)(c*log(*ps-smin+1.0));
   }
}
    
void ImageModel::expTransform( int outbands ) {
   Float smin=_imageSource->getMinval();
   Float smax=_imageSource->getMaxval();
    
   const double c=(((double)smax-smin) / (log((double)outbands+1.0)));
   for (int b=0;b<_imageSource->Bands();b++) {
      Float *ps=_imageSource->Vector(b); 
      Uchar *pl=_imageLogic->Vector(b);
      const Float *end=ps+_imageSource->VectorSize();
      for (;ps<end;ps++,pl++)
    	 *pl=(Uchar)(exp((double)(*ps-smin)/c) -1.0);
   }
}
    
void ImageModel::histogramEqualization( int outbands ) {
   std::map<Float,float> histo;
   std::map<Float,float> histoc;
       
   // Compute histogram and min-max values.
   for (int b=0;b<_imageSource->Bands();b++) {
      Float *ps=_imageSource->Vector(b);
      const Float *end=ps+_imageSource->VectorSize();
      for (;ps<end;ps++)
    	 histo[*ps]++;
          
      // Compute cumulated histogram.
      // (discard h[min] -> set histoc[min]=0)
      // This is usefull to begin exactly the output histogram
      // at min (either minout or minval)
      float sum=0;
      std::map<Float,float>::const_iterator h;
      h=histo.begin();
      float h0=histo[h->first];
      histoc[h->first]=0.0F;
      h++;
      float total=_imageSource->VectorSize()-h0;
      for (;h != histo.end();h++) {
    	 sum=histoc[h->first]=sum+h->second;
    	 histoc[h->first]=histoc[h->first] / total;
      }
      ps=_imageSource->Vector(b);
      Uchar *pl=_imageLogic->Vector(b);
      for (;ps<end;ps++,pl++)
    	 *pl=(Uchar)(histoc[*ps]*(float)outbands);
          
      histo.clear();
      histoc.clear();
   }
}
    
    
/*
 * Thresholding:
 * if thresholdLow <= ThresholdHigh then thresholding out.
 * if thresholdLow > ThresholdHigh then thresholding in.
 */
void ImageModel::buildImagePhysic( ) {
   // Tip: Use b0, b1 and b2 for each plane: b0=b1=b2 for gray images.
   int b0=0, b1=0, b2=0;
   const int w=_imageLogic->Width();
   const int h=_imageLogic->Height();

   if (_imageLogic->Bands()>1) { b1=1; b2=2; }
   _imagePhysic=QImage(w,h,QImage::Format_RGB32);

   for (int y=0; y<h;y++) {
      uint *p = reinterpret_cast<uint *>(_imagePhysic.scanLine(y));
      for (int x=0; x<w;x++) {
     	 int r=0; int g=0; int b=0;

	 Float pixel=(*_imageSource)[b0][_currentPlane][y][x];
	 if (_thresholdL[b0] <= _thresholdH[b0]) {
	    if (pixel<_thresholdL[b0] || pixel>_thresholdH[b0]) {
	       r=_lut[0][0]*_red;
	    } else {
	       r=_lut[0][(*_imageLogic)[b0][_currentPlane][y][x]]*_red;
	    }
	 } else {
	    if (pixel>_thresholdL[b0] || pixel<_thresholdH[b0]) {
	       r=_lut[0][0]*_red;
	    } else {
	       r=_lut[0][(*_imageLogic)[b0][_currentPlane][y][x]]*_red;
	    }
	 }
	 pixel=(*_imageSource)[b1][_currentPlane][y][x];
	 if (_thresholdL[b1] <= _thresholdH[b1]) {
	    if (pixel<_thresholdL[b1] || pixel>_thresholdH[b1]) {
	       g=_lut[1][0]*_green;
	    } else {
	       g=_lut[1][(*_imageLogic)[b1][_currentPlane][y][x]]*_green;
	    }
	 } else {
	    if (pixel>_thresholdL[b1] || pixel<_thresholdH[b1]) {
	       g=_lut[1][0]*_green;
	    } else {
	       g=_lut[1][(*_imageLogic)[b1][_currentPlane][y][x]]*_green;
	    }
	 }
	 pixel=(*_imageSource)[b0][_currentPlane][y][x];
	 if (_thresholdL[b2] <= _thresholdH[b2]) {
	    if (pixel<_thresholdL[b2] || pixel>_thresholdH[b2]) {
	       b=_lut[2][0]*_blue;
	    } else {
	       b=_lut[2][(*_imageLogic)[b2][_currentPlane][y][x]]*_blue;
	    }
	 } else {
	    if (pixel>_thresholdL[b2] || pixel<_thresholdH[b2]) {
	       b=_lut[2][0]*_blue;
	    } else {
	       b=_lut[2][(*_imageLogic)[b2][_currentPlane][y][x]]*_blue;
	    }
	 }
    	 *(p++) = setRgb(r,g,b);
      }
   }
   // Recreate the image from the factor zoom
   _imagePhysic=_imagePhysic.scaled((int)(w*_zoom),(int)(h*_zoom),Qt::KeepAspectRatioByExpanding,Qt::FastTransformation);
}
    
QRgb ImageModel::setRgb( Uchar r, Uchar g, Uchar b ) {
   float mn=r,mx=r; 
   int maxVal=0; 
   float h=0;
   float s;
   int v;
   if (g > mx) { mx=g;maxVal=1;} 
   if (b > mx) { mx=b;maxVal=2;}  
   if (g < mn) mn=g; 
   if (b < mn) mn=b;  
   float  delta = mx - mn; 
   v =(int)mx;  
   if ( mx != 0 ) {
      s = delta / mx;  
      if (s==0.0f) 
    	 h=-1;  
      else {  
    	 switch (maxVal) { 
    	 case 0:{h = ( g - b ) / delta;break;}         // yel < h < mag 
    	 case 1:{h = 2 + ( b - r ) / delta;break;}     // cyan < h < yel 
    	 case 2:{h = 4 + ( r - g ) / delta;break;}     // mag < h < cyan 
    	 } 
      }
   } 
   else { 
      s = 0; 
      h = 0;  
   } 
   h *= 60; 
   if ( h < 0 ) h += 360; 
   //apply brightness
   v+=(int)(0.5*_brightness);
   if (v>=MAXCOLS)
      v=MAXCOLS-1;
   if (v<0)
      v=0;
   int i; 
   float f, p, q, t,hTemp; 
   if ( s == 0.0 || h == -1.0) {// s==0? Totally unsaturated = grey so R,G and B all equal value 
      r = g = b = v;
      //apply contrast
      if (r<127) {
    	 if (r-_contrast<0) r=0;
    	 else if (r-_contrast>127) r=127;
    	 else r-=_contrast;
      } else if (r>127) {
    	 if (r+_contrast<127) r=127;
    	 else if (r+_contrast>=MAXCOLS) r=MAXCOLS-1;
    	 else r+=_contrast;
      }
      g=b=r;
      return qRgb(r,g,b); 
   } 
   hTemp = h/60.0f; 
   i = (int)floor( hTemp );                 // which sector 
   f = hTemp - i;                      // how far through sector 
   p = v  *( 1 - s ); 
   q = v  *( 1 - s  *f ); 
   t = v  *( 1 - s  *( 1 - f ) ); 
       
   switch( i ) { 
   case 0:{ r = (Uchar)v; g = (Uchar)t; b = (Uchar)p; break; } 
   case 1:{ r = (Uchar)q; g = (Uchar)v; b = (Uchar)p; break; } 
   case 2:{ r = (Uchar)p; g = (Uchar)v; b = (Uchar)t; break; } 
   case 3:{ r = (Uchar)p; g = (Uchar)q; b = (Uchar)v; break; }  
   case 4:{ r = (Uchar)t; g = (Uchar)p; b = (Uchar)v; break; } 
   case 5:{ r = (Uchar)v; g = (Uchar)p; b = (Uchar)q; break; } 
   }
   //apply contrast
   if (r<127) {
      if (r-_contrast<0) r=0;
      else if (r-_contrast>127) r=127;
      else r-=_contrast;
   } else if (r>127) {
      if (r+_contrast<127) r=127;
      else if (r+_contrast>=MAXCOLS) r=MAXCOLS-1;
      else r+=_contrast;
   } if (g<127) {
      if (g-_contrast<0) g=0;
      else if (g-_contrast>127) g=127;
      else g-=_contrast;
   } else if (g>127) {
      if (g+_contrast<127) g=127;
      else if (g+_contrast>=MAXCOLS) g=MAXCOLS-1;
      else g+=_contrast;
   }
   if (b<127) {
      if (b-_contrast<0)	 b=0;
      else if (b-_contrast>127) b=127;
      else b-=_contrast;
   } else if (b>127) {
      if (b+_contrast<127) b=127;
      else if (b+_contrast>=MAXCOLS) b=MAXCOLS-1;
      else b+=_contrast;
   }
   return qRgb(r, g, b);
}
    
void ImageModel::callUpdate() {
   emit askForUpdate();
}

void ImageModel::openImage( ) {
   QWidget *OpenFile(Pobject *objs, Pobject **objd);
   QString imageFile = QFileDialog::getOpenFileName(this,
    						    tr("Choose an image file"),
    						    tr("."),
    						    tr("Images (*.pan *.bmp *.png *.jpg *.tif)"));
   if (!imageFile.isEmpty() && QFile::exists(imageFile)) {
      nomentree=strdup(imageFile.toAscii().data());
      Pobject *objs=LoadFile(nomentree);
      if (objs==NULL) {
	 QImage *data = new QImage(nomentree);
	 if (data->width()>0) {
	    if (data->allGray()) {
	       Img2duc *ims = new Img2duc(data->height(),data->width());
	       objs = ims;
	       for (int y=0; y<data->height(); y++) {
		  for (int x=0; x<data->width(); x++) {
		     QRgb pixel = data->pixel(x,y);
		     (*ims)[y][x] = qRed(pixel);
		  }
	       }
	    } else {
	       Imc2duc *ims = new Imc2duc(data->height(),data->width());
	       objs = ims;
	       for (int y=0; y<data->height(); y++) {
		  for (int x=0; x<data->width(); x++) {
		     QRgb pixel = data->pixel(x,y);
		     (*ims)(0,y,x) = qRed(pixel);
		     (*ims)(1,y,x) = qGreen(pixel);
		     (*ims)(2,y,x) = qBlue(pixel);
		  }
	       }
	    }
	 }
      }
      if (objs!=NULL) {
	 (void)OpenFile(objs,(Pobject**)&_imageDessin);
	 // 	 delete this; 
      }
   }
}

void ImageModel::saveAsImage( ) {
   QString fileName = QFileDialog::getSaveFileName(this, //parent
						   tr("Save current image"),  // Caption
						   ".", // dir
						   tr("Image Files (*.pan *.bmp *.png *.jpg *.jpeg *.tif *.tiff)") // Filter
						   );
   if (fileName ==0) return; // cancel button
   
   QFileInfo fi(fileName);
   QString extension = fi.completeSuffix().toLower();
   if (QString::compare(extension,"bmp")==0) {
      _imagePhysic.save(fileName,"BMP");
   } else if ((QString::compare(extension,"jpg")==0)
	      || (QString::compare(extension,"jpeg")==0)) {
      _imagePhysic.save(fileName,"JPG");
   } else if ((QString::compare(extension,"tif")==0)
	      || (QString::compare(extension,"tiff")==0)) {
      _imagePhysic.save(fileName,"TIF");
   } else if (QString::compare(extension,"png")==0) {
      _imagePhysic.save(fileName,"PNG");
   } else if (QString::compare(extension,"pan")==0) {
      if (_imageLogic->Bands() == 1) { 
	 Img2duc *tmp = new Img2duc(_imageLogic->Props());
	 for (int y=0; y<_imagePhysic.height(); y++) {
	    for (int x=0; x<_imagePhysic.width(); x++) {
	       QRgb pixel = _imagePhysic.pixel(x,y);
	       (*tmp)[y][x] = qRed(pixel);
	    }
	 }
	 tmp->SaveFile(fileName.toAscii());
	 delete tmp;
      } else {
	 Imc2duc *tmp = new Imc2duc(_imageLogic->Props());
	 for (int y=0; y<_imagePhysic.height(); y++) {
	    for (int x=0; x<_imagePhysic.width(); x++) {
	       QRgb pixel = _imagePhysic.pixel(x,y);
	       (*tmp)(0,y,x) = qRed(pixel);
	       (*tmp)(1,y,x) = qGreen(pixel);
	       (*tmp)(2,y,x) = qBlue(pixel);
	       
	    }
	 }
	 tmp->SaveFile(fileName.toAscii());
	 delete tmp;
      }
   } else {
      QMessageBox::critical(this, "Save current image",
			    QString("Unsupported file extension:")
			    + extension
			    +"\nOnly pan, jpg, bmp, tiff, and png formats are supported.\n"
			    +"Image not saved.");
   }
}

void ImageModel::closeApplication( ) {
   QApplication::closeAllWindows();
}

void ImageModel::properties( ) {
   _propertiesWindow->show();
   _propertiesWindow->raise();
}

void ImageModel::thresholding( ) {
   _thresholdWindow->show();
   _thresholdWindow->raise();
}
 
void ImageModel::colors( ) {
   _colorWindow->show();
   _colorWindow->raise();
}

void ImageModel::values( ) {
   _valuesWindow->show();
   _valuesWindow->raise();
}
    
void ImageModel::histo( ) {
   _histogramWindow->show();
   _histogramWindow->raise();
}
    
void ImageModel::row( ) {
   _profileRWindow->show();
   _profileRWindow->raise();
}
    
void ImageModel::column( ) {
   _profileCWindow->show();
   _profileCWindow->raise();
}
    
void ImageModel::changeZoom(float zoom) {
   if (_zoom==zoom)
      return;
   _zoom=zoom;
   buildImagePhysic();
   callUpdate();
   int x,y;
   if (_imagePhysic.width() <= _scrollImage->width() || (int)(_currentPosX*_zoom)<_scrollImage->width()/2)
      x=0;
   else if (_imagePhysic.width()-(int)(_currentPosX*_zoom) >=_scrollImage->width()/2) {
      x=(int)(_currentPosX*_zoom)-_scrollImage->width()/2;
   } else
      x=(_imagePhysic.width()-_scrollImage->width()+_scrollImage->verticalScrollBar()->width()+2);
    
   if (_imagePhysic.height() <= _scrollImage->height() || (int)(_currentPosY*_zoom)<_scrollImage->height()/2)
      y=0;
   else if (_imagePhysic.height()-(int)(_currentPosY*_zoom) >=_scrollImage->height()/2)
      y=(int)(_currentPosY*_zoom)-_scrollImage->height()/2;
   else
      y=(_imagePhysic.height()-_scrollImage->height()+_scrollImage->horizontalScrollBar()->height()+2);
   _scrollImage->horizontalScrollBar()->setValue(x);
   _scrollImage->verticalScrollBar()->setValue(y);
}  

void ImageModel::changePlane(int p) {
   _currentPlane=(long)p;
   buildImagePhysic();
   callUpdate(); 
}   

void ImageModel::changeThresholdL(int band, float thresholdL) {
   _thresholdL[band]=thresholdL;
   buildImagePhysic();
   callUpdate();
}

void ImageModel::changeThresholdH(int band, float thresholdH) {
   _thresholdH[band]=thresholdH;
   buildImagePhysic();
   callUpdate();
}

void ImageModel::changeDynamic(int id) {
   _normalization=id;
   buildImageLogic();
   buildImagePhysic();
   callUpdate();
}

void ImageModel::rotateX( ) {
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   if (_DRAWINGMODE) {
      rotateDessinX();
      delete _lastDraw;
      _lastDraw= new Imx3duc(1,1,_imageSource->Depth(), _imageSource->Width());
      cleanLastDraw();
   }
   ImageSource *temp = new ImageSource(_imageSource->Bands(), _imageSource->Height(), _imageSource->Depth(), _imageSource->Width());
   for (int b=0; b<temp->Bands(); b++) {
      float *pt=temp->Vector(b);
      for (int z=0; z<temp->Depth();z++)
    	 for (int y=0; y<temp->Height(); y++)
    	    for (int x=0; x<temp->Width(); x++)
    	       *(pt++)=(*_imageSource)(b,y,_imageSource->Height()-(z+1),x);
   }
   temp->setInfo(_imageSource->getInfo());
   temp->setMaxval(_imageSource->getMaxval());
   temp->setMinval(_imageSource->getMinval());
   temp->setGraph(_imageSource->isGraph());
   temp->setRegion(_imageSource->isRegion());
   temp->setColored(_imageSource->isColored());
   temp->setTypeval(_imageSource->getTypeval());
   temp->setDynamisable(_imageSource->isDynamisable());
   delete _imageSource;
   _imageSource=temp;
   _currentPosX=0;
   _currentPosY=0;
   emit changeNumberPlane(_imageSource->Depth());
   buildImageLogic();
   buildImagePhysic();
   QApplication::restoreOverrideCursor();
   callUpdate();
   callUpdate();
}
    
void ImageModel::rotateY( ) {
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   if (_DRAWINGMODE) {
      rotateDessinY();
      delete _lastDraw;
      _lastDraw= new Imx3duc(1,1,_imageSource->Height(), _imageSource->Depth());
      cleanLastDraw();
   }
   ImageSource *temp = new ImageSource(_imageSource->Bands(), _imageSource->Width(), _imageSource->Height(), _imageSource->Depth());
   for (int b=0; b<temp->Bands(); b++) {
      float *pt=temp->Vector(b);
      for (int z=0; z<temp->Depth();z++)
    	 for (int y=0; y<temp->Height(); y++)
    	    for (int x=0; x<temp->Width(); x++)
    	       *(pt++)=(*_imageSource)(b,_imageSource->Depth()-(x+1),y,z);
   }
   temp->setInfo(_imageSource->getInfo());
   temp->setMaxval(_imageSource->getMaxval());
   temp->setMinval(_imageSource->getMinval());
   temp->setGraph(_imageSource->isGraph());
   temp->setRegion(_imageSource->isRegion());
   temp->setColored(_imageSource->isColored());
   temp->setTypeval(_imageSource->getTypeval());
   temp->setDynamisable(_imageSource->isDynamisable());
   delete _imageSource;
   _imageSource=temp;
   _currentPosX=0;
   _currentPosY=0;
   emit changeNumberPlane(_imageSource->Depth());
   buildImageLogic();
   buildImagePhysic();
   QApplication::restoreOverrideCursor();
   callUpdate();
   callUpdate();
}
    
void ImageModel::rotateZ( ) {
   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   if (_DRAWINGMODE) {
      rotateDessinZ();
      delete _lastDraw;
      _lastDraw= new Imx3duc(1,1,_imageSource->Width(), _imageSource->Height());
      cleanLastDraw();
   }
   ImageSource *temp = new ImageSource(_imageSource->Bands(),_imageSource->Depth(),_imageSource->Width(),_imageSource->Height());
   for (int b=0; b<temp->Bands(); b++) {
      float *pt=temp->Vector(b);
      for (int z=0; z<temp->Depth();z++)
	 for (int y=0; y<temp->Height(); y++)
	    for (int x=0; x<temp->Width(); x++)
	       *(pt++)=(*_imageSource)(b,z,x,_imageSource->Width()-1-y);
   }
   temp->setInfo(_imageSource->getInfo());
   temp->setMaxval(_imageSource->getMaxval());
   temp->setMinval(_imageSource->getMinval());
   temp->setGraph(_imageSource->isGraph());
   temp->setRegion(_imageSource->isRegion());
   temp->setColored(_imageSource->isColored());
   temp->setTypeval(_imageSource->getTypeval());
   temp->setDynamisable(_imageSource->isDynamisable());
   delete _imageSource;
   _imageSource=temp;
   _currentPosX=0;
   _currentPosY=0;
   emit changeNumberPlane(_imageSource->Depth());
   buildImageLogic();
   buildImagePhysic();
   QApplication::restoreOverrideCursor();
   callUpdate();
   callUpdate();
}
    
void ImageModel::setRed(bool on) {
   _red=(on?1:0);
   buildImagePhysic();
   callUpdate();
}
    
    
void ImageModel::setBlue(bool on) {
   _blue=(on?1:0);
   buildImagePhysic();
   callUpdate();
}
    
    
void ImageModel::setGreen(bool on) {
   _green=(on?1:0);
   buildImagePhysic();
   callUpdate();
}
    
void ImageModel::resetColor( ) {
   _red=_green=_blue=1;
   _brightness=0;
   _contrast=0;
   buildOriginalLut();
   buildImagePhysic();
   callUpdate();
}

void ImageModel::changeLut(int id) {
   switch(id) {
   case ImageModel::original:
      buildOriginalLut();
      break;
   case ImageModel::random:
      buildRandomLut();
      break;
   case ImageModel::region:
      buildRegionLut();
      break;
   case ImageModel::rainbow:
      buildRainbowLut();
      break;
   case ImageModel::negatif:
      buildNegatifLut();
      break;
   case ImageModel::noise:
      buildNoiseLut();
      break;
   }
   buildImagePhysic();
   callUpdate();
}
    
    
void ImageModel::setBrightness(int val) {
   _brightness=val;
   buildImagePhysic();
   callUpdate();
}

void ImageModel::setContrast(int val) {
   _contrast=val;
   buildImagePhysic();
   callUpdate();
}

void ImageModel::keyPressEvent(QKeyEvent *e) {
   switch(e->key()) {
   case Qt::Key_Left :
      if (CurrentPosX()>0) {
    	 --(_currentPosX);
    	 callUpdate();
      }
      break;
   case Qt::Key_Right :
      if (CurrentPosX()<=_imageLogic->Width()-6) {
    	 ++(_currentPosX);
    	 callUpdate();
      }
      break;
   case Qt::Key_Up :
      if (CurrentPosY()>0) {
    	 --(_currentPosY);	 
    	 callUpdate();
      }
      break;
   case Qt::Key_Down :
      if (CurrentPosY()<=_imageLogic->Height()-6) {
    	 ++(_currentPosY);
    	 callUpdate();
      }
      break;
   case Qt::Key_Plus :
      emit zoom(true);
      break;
   case Qt::Key_Minus :
      emit zoom(false);
      break;
   case Qt::Key_O :
      openImage();
      break;
   case Qt::Key_S :
      saveAsImage();
      break;
   case Qt::Key_Q :
      closeApplication();
      break;
   case Qt::Key_T:
      thresholding();
      break;
   case Qt::Key_P:
      properties();
      break;
   case Qt::Key_L:
      colors();
      break;
   case Qt::Key_V:
      values();
      break;
   case Qt::Key_H:
      histo();
      break;
   case Qt::Key_R:
      row();
      break;
   case Qt::Key_C:
      column();
      break;
   case Qt::Key_X:
      if (_imageSource->Depth()>1)
    	 rotateX();
      break;
   case Qt::Key_Y:
      if (_imageSource->Depth()>1)
    	 rotateY();
      break;
   case Qt::Key_Z:
      rotateZ();
      break;
   case Qt::Key_U:
      undoDraw();
      break;
   default :
      break;
   }
}
    
/**
 *Rotations
 */ 
void ImageModel::rotateDessinX( ) {
   cleanLastDraw();
   Imx3duc *temp = new Img3duc( _imageDraw->Height(), _imageDraw->Depth(), _imageDraw->Width());
   Uchar *pt=temp->Vector(0);
   for (int z=0; z<temp->Depth();z++)
      for (int y=0; y<temp->Height(); y++)
    	 for (int x=0; x<temp->Width(); x++)
    	    *(pt++)=(*_imageDraw)(0,y,_imageDraw->Height()-(z+1),x);
   delete _imageDraw;
   _imageDraw=temp;
   int vectTemp[3];
   vectTemp[0]=_vectY[0];vectTemp[1]=_vectY[1];vectTemp[2]=_vectY[2];
   _vectY[0]=-_vectZ[0];_vectY[1]=-_vectZ[1];_vectY[2]=-_vectZ[2];
   _vectZ[0]=vectTemp[0]; _vectZ[1]=vectTemp[1]; _vectZ[2]=vectTemp[2];
}
    
void ImageModel::rotateDessinY( ) {
   cleanLastDraw();
   Imx3duc *temp = new Img3duc( _imageDraw->Width(), _imageDraw->Height(), _imageDraw->Depth());
   Uchar *pt=temp->Vector(0);
   for (int z=0; z<temp->Depth();z++)
      for (int y=0; y<temp->Height(); y++)
    	 for (int x=0; x<temp->Width(); x++)
    	    *(pt++)=(*_imageDraw)(0,_imageDraw->Depth()-(x+1),y,z);
   delete _imageDraw;
   _imageDraw=temp;
   int vectTemp[3];
   vectTemp[0]=_vectZ[0];vectTemp[1]=_vectZ[1];vectTemp[2]=_vectZ[2];
   _vectZ[0]=-_vectX[0];_vectZ[1]=-_vectX[1];_vectZ[2]=-_vectX[2];
   _vectX[0]=vectTemp[0]; _vectX[1]=vectTemp[1]; _vectX[2]=vectTemp[2];
}
     
void ImageModel::rotateDessinZ( ) {
   cleanLastDraw();
   Imx3duc *temp = new Img3duc( _imageDraw->Depth(),_imageDraw->Width(), _imageDraw->Height());
   Uchar *pt=temp->Vector(0);
   for (int z=0; z<temp->Depth();z++)
      for (int y=0; y<temp->Height(); y++)
    	 for (int x=0; x<temp->Width(); x++)
    	    *(pt++)=(*_imageDraw)(0,z,x,_imageDraw->Width()-(y+1));
   delete _imageDraw;
   _imageDraw=temp;
   int vectTemp[3];
   vectTemp[0]=_vectX[0];vectTemp[1]=_vectX[1];vectTemp[2]=_vectX[2];
   _vectX[0]=-_vectY[0];_vectX[1]=-_vectY[1];_vectX[2]=-_vectY[2];
   _vectY[0]=vectTemp[0]; _vectY[1]=vectTemp[1]; _vectY[2]=vectTemp[2];
}
     
void ImageModel::changeCursorStatus( ) {
   _cursor= !_cursor;
   callUpdate();
}

/**
 * Drawing facilities
 */
void ImageModel::drawLine( int xb, int yb, int xe, int ye, Uchar coul ) {
   int x,y,max,min;
   float pente;
       
   if (xb!=xe)
      pente=(float)(yb-ye)/(xb-xe);
   else
      pente=MAXFLOAT;
       
   if ((pente<-1) || (pente>1)) {
      if (ye<yb) {
    	 min=ye; max=yb;
      } else {
    	 min=yb; max=ye;
      }
      for (y=max;y>=min;y--) {
    	 x=(int)((y-ye)/pente +xe);
    	 (*_imageDraw)(0,_currentPlane,y,x)=coul;
      }
   }else{
      if (xe<xb) {
    	 min=xe; max=xb;
      } else {
    	 min=xb; max=xe;
      }
      for (x=max;x>=min;x--) {
    	 y=(int)(pente*(x-xe)+ye);
    	 (*_imageDraw)(0,_currentPlane,y,x)=coul;
      }
   }
}
    
void ImageModel::drawLine( int xb, int yb,int xe, int ye ) {
   if (_pencilStatus) {
      drawLine(xb,yb,xe,ye,255);
      _lastDrawingPoints.push(DrawingPoint(xe,ye));
   } else {
      drawLine(xb,yb,xe,ye,0);
   }
}
    
void ImageModel::cleanLastDraw( ) {
   while (!_lastDrawingPoints.empty()) _lastDrawingPoints.pop();
}
    
void ImageModel::clearDraw( ) {
   Uchar *p=&((*_imageDraw)(0,_currentPlane,0,0));
   memset(p,0,_imageDraw->Width()*_imageDraw->Height());
   cleanLastDraw();
   emit draw();
}
    
void ImageModel::undoDraw( ) {
   if (!_lastDrawingPoints.empty()) {
      DrawingPoint ptb, pte;
      pte=_lastDrawingPoints.top();
      while (!_lastDrawingPoints.empty()) {
	 ptb=pte;
	 pte=_lastDrawingPoints.top();
	 _lastDrawingPoints.pop();
	 drawLine(ptb.x, ptb.y, pte.x, pte.y, 0);
      }
      emit draw();
   }
}
