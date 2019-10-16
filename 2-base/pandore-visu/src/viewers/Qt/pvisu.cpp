/* -*- c-basic-offset: 3 -*-
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
 * @author Régis Clouard - 1999-11-10
 * @author Régis Clouard - 2001-04-25 (version 3.00)
 * @author Régis Clouard - 2002-12-12 (version 4.00)
 * @author Régis Clouard - 2003-01-11 (+ imx)
 * @author Régis Clouard - 2003-05-07 (= dessin 3d)
 * @author Nicolas Briand - 2005-10-05 (Qtviewer)
 * @author Régis Clouard - 2006-01-17 (add Imx)
 * @author Régis Clouard - 2006-02-15 (Perform some code optimization)
 * @author Régis Clouard - 2007-06-15 (fix bufg on Imx2sf, Imx3dsf)
 * @author Régis Clouard - Jun 21, 2010 (add: open bmp, png, jpg ... images)
 */

/**
 * @file pvisu.cpp
 *
 * Contains the main, builds the images and runs the application
 */

#include <QApplication>
#include <QWidget>
#include <math.h>
#include <iostream>

//#define DEBUG

#ifdef DEBUG
#include <time.h>
clock_t temps_deb;
#endif

bool _DRAWINGMODE;

#include "imagevisu.h"
#include "imagemodel.h"
#include "infoview.h"

char size[120];
char etc[120];
Char *nomentree;

char *progname;

/**
 * Returns the name of the given colorspace.
 * @param c	The colorspace.
 * @return	the name of the colorspace.
 */
const char *ColorName( PColorSpace c ) {
   switch(c) {
   case RGB: return "RGB";
   case XYZ: return "XYZ";
   case LUV: return "LUV";
   case LAB: return "LAB";
   case HSL: return "HSL";
   case AST: return "AST";
   case I1I2I3: return "I1I2I3";
   case LCH: return "LCH";
   case WRY: return "WRY";
   case RNGNBN: return "RNGNBN";
   case YCBCR: return "YCBCR";
   case YCH1CH2: return "YCH1CH2";
   case YIQ: return "YIQ";
   case YUV: return "YUV";       
   default: return "unknown...";
   }
}

/**
 * Draws a line in imd, from coordinate pti to ptf with color color.
 */
void DrawLine( ImageSource *imd, Point3d pti, Point3d ptf, Float color ) {
   Long xmin,ymin,zmin;
   Long xmax,ymax,zmax;
   Long i,j,k;
   Long d;
   float penteyx,pentezx;
   float penteyz;
  
   if (pti.x==ptf.x) {
      penteyx=pentezx=MAXFLOAT;
      xmin=xmax=ptf.x;
   } else {
      d=ptf.x-pti.x;
      penteyx=(float)(ptf.y-pti.y)/d;
      pentezx=(float)(ptf.z-pti.z)/d;
      if (pti.x<ptf.x) { xmin=pti.x; xmax=ptf.x; }
      else { xmin=ptf.x; xmax=pti.x; }
   }
   if (pti.y==ptf.y) {
      penteyx=penteyz=0.0;
      ymin=ymax=ptf.x;
   } else {
      if (pti.y<ptf.y) { ymin=pti.y; ymax=ptf.y; }
      else { ymin=ptf.y; ymax=pti.y; }
   }
   if (pti.z==ptf.z) {
      penteyz=MAXFLOAT;
      pentezx=0.0;
      zmin=zmax=ptf.z;
   } else {
      penteyz=(float)(ptf.y-pti.y)/(ptf.z-pti.z);
      if (pti.z<ptf.z) { zmin=pti.z; zmax=ptf.z; }
      else { zmin=ptf.z; zmax=pti.z; }
   }
  
   if ((pentezx<-1)||(pentezx>1))
      if ((penteyx<-1)||(penteyx>1))
	 if (((pentezx<-1)&&(penteyx<pentezx)) || ((pentezx>1)&&(penteyx>pentezx)))
	    for (j=ymin;j<=ymax;j++) {
	       d=j-ptf.y;
	       k=(int)(d/penteyx+ptf.x);
	       i=(int)(d/penteyz+ptf.z);
	       (*imd)(0,i,j,k)=color;
	    }
	 else
	    for (i=zmin;i<=zmax;i++) {
	       d=i-ptf.z;
	       j=(int)(d*penteyz+ptf.y);
	       k=(int)(d/pentezx+ptf.x);
	       (*imd)(0,i,j,k)=color;
	    }
      else
	 for (k=xmin;k<=xmax;k++) {
	    d=k-ptf.x;
	    j=(int)(d*penteyx+ptf.y);
	    i=(int)(d*pentezx+ptf.z);
	    (*imd)(0,i,j,k)=color;
	 }
   else
      if ((penteyx<-1)||(penteyx>1))
	 for (j=ymin;j<=ymax;j++) {
	    d=j-ptf.y;
	    k=(int)(d/penteyx+ptf.x);
	    i=(int)(d/penteyz+ptf.z);
	    (*imd)(0,i,j,k)=color;
	 }
      else
	 for (k=xmin;k<=xmax;k++) {
	    d=k-ptf.x;
	    j=(int)(d*penteyx+ptf.y);
	    i=(int)(d*pentezx+ptf.z);
	    (*imd)(0,i,j,k)=color;
	 }
}


/**
 * @brief Visualization without input file.
 *
 * Creates the source image with <tt>ims</tt> data
 * and displays it thanks to the given graphical application <tt>app</tt>.
 * In case of pdraw program, creates the output
 * image <tt>imd</tt> with user's drawing.
 * @return	SUCCESS or FAILURE
 */
QWidget* Visu( ) {
   ImageSource *imsource=new ImageSource(1,1,256, 256);
   imsource->setType("");
   imsource->setContent("");
   imsource->setSize("");
   imsource->setPixel("");
   imsource->setMinval(0);
   imsource->setMaxval(MAXUINT1);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(false);
  
   Float *d=imsource->Vector(0);
   for(;d<imsource->Vector(0)+imsource->VectorSize();)
      *(d++)=0;
   
   QWidget *visual;
   visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/**
 * Creates the source image with <tt>ims</tt> data
 * and displays it thanks to the given graphical application <tt>app</tt>.
 * In case of dessin program, creates the output
 * image <tt>imd</tt> with user's drawing.
 * Optimized.
 *
 * @param ims the input image
 * @param imageDessin the output image (for dessin only)
 * @return	SUCCESS or FAILURE
 */
QWidget *Visu( Img2duc &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,1,ims.Height(),ims.Width());
   imsource->setType("Img2duc");
   imsource->setContent("Image, Gray level, 2D");
   sprintf(size,"%ld rows x %ld cols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel(" integer 8-bits");
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(false);
   
   Float *d=imsource->Vector(0);
   Uchar *s=ims.Vector();
   const Uchar *end=s+ims.VectorSize();
   Uchar min=*s;
   Uchar max=*s;
   for(;s<end;) {
      if (*s>max) max=*s;
      else if (*s<min) min=*s;
      *(d++)=(Float)*(s++);
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else {
      visual=new ImageModel(imsource,nomentree);
   }
   visual->show();
   ims.Delete();
   return visual;
}

/*
 * Optimized
 */
QWidget *Visu( Img3duc &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Img3duc");
   imsource->setContent("Image, Gray level, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 8-bits");
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(false);
   
   Float *d=imsource->Vector(0);
   Uchar *s=ims.Vector();
   const Uchar *end=s+ims.VectorSize();
   Uchar min=*s;
   Uchar max=*s;
   for(;s<end;) {
      if (*s>max) max=*s;
      else if (*s<min) min=*s;
      *(d++)=(Float)*(s++);
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);

   ims.Delete();
   QWidget *visual;
   
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imc2duc &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   imsource->setType("Imc2duc");
   imsource->setContent("Image, Color level, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 8-bits");
   sprintf(etc, "Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(true);
   
   Uchar min=ims(0,0,0);
   Uchar max=ims(0,0,0);
   for (int b=0; b< ims.Bands(); b++) {
      Uchar *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      Uchar *end=sx+ims.VectorSize();
      for(;sx<end;) {
      if (*sx>max) max=*sx;
      else if (*sx<min) min=*sx;
	 *(dx++)=(Float)*(sx++);
      }
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else {
      visual=new ImageModel(imsource,nomentree);
   }
   visual->show(); 
   ims.Delete();
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imc3duc &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Img3duc");
   imsource->setContent("Image, Color level, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 8-bits");
   sprintf(etc, "Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(true);
   
#ifdef DEBUG
   printf("    visu 0 %f\n",((float)(clock()-temps_deb)/CLOCKS_PER_SEC));
#endif
   Uchar min=ims(0,0,0,0);
   Uchar max=ims(0,0,0,0);
   for (int b=0; b< ims.Bands(); b++) {
      Uchar *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      Uchar *end=sx+ims.VectorSize();
      for(;sx<end;) {
      if (*sx>max) max=*sx;
      else if (*sx<min) min=*sx;
	 *(dx++)=(Float)*(sx++);
      }
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);

#ifdef DEBUG
   printf("    visu 1 %f\n",((float)(clock()-temps_deb)/CLOCKS_PER_SEC));
#endif
   
   ims.Delete();
   QWidget *visual;
 
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
#ifdef DEBUG
   printf("   visu 2 %f\n",((float)(clock()-temps_deb)/CLOCKS_PER_SEC));
#endif

   visual->show();

   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Img2dsl &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,1,ims.Height(), ims.Width());
   imsource->setType("Img2dsl");
   imsource->setContent("Image, Gray level 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
 
   Float *d=imsource->Vector(0);
   Long *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   const Long *end=s+ims.VectorSize();
   for(;s<end; s++, d++) {
      if ((*(d)=(Float)*(s)) > max )
	 max=*d;
      else if (*d < min)
	 min=*d;
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Img3dsl &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Img3dsl");
   imsource->setContent("Image, Gray level 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);

   Float *d=imsource->Vector(0);
   Long *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   const Long *end=s+ims.VectorSize();
   for(;s<end; s++, d++) {
      if ((*(d)=*(s)) > max)
	 max=*d;
      else if (*d < min)
	 min=*d;
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}


/*
 * Optimized.
 */
QWidget *Visu( Imc2dsl &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   imsource->setType("Imc2dsl");
   imsource->setContent("Image, Color,  2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   sprintf(etc, "Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(true);
   
   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands(); b++ ) { 
      Long *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Long *end=sx+ims.VectorSize();
      for(;sx<end;) {
	 if (*sx>max) max=*sx;
	 else if (*sx<min) min=*sx;
	 *(dx++)=(Float)*(sx++);
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   
   ims.Delete();  
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized
 */
QWidget *Visu( Imc3dsl &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Imc3dsl");
   imsource->setContent("Image, Color,  3D");
   sprintf(size,"%ld rows x %ld ncols x %ld depth",(long)ims.Height(),(long)ims.Width(),(long)ims.Depth());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   sprintf(etc,"Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(true);
   
   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands(); b++) {
      Long *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Long *end=sx+ims.VectorSize();
      for(;sx<end;) {
	 if (*sx>max) max=*sx;
	 else if (*sx<min) min=*sx;
	 *(dx++)=(Float)*(sx++);
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   ims.Delete();  
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized
 */
QWidget *Visu( Img2dsf &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,1,ims.Height(), ims.Width());
   imsource->setType("Img2dsf");
   imsource->setContent("Image, Gray level 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(false);
   
   Float *d=imsource->Vector(0);
   Float *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   const Float *end=s+ims.VectorSize();
   for(;s<end; d++) {
      if ((*(d)=*(s++)) > max)
	 max=*d;
      else if (*d < min)
	 min=*d;
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */ 
QWidget *Visu( Img3dsf &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Img3dsf");
   imsource->setContent("Image, Gray level 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(false);
   
   Float *d=imsource->Vector(0);
   Float *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   const Float *end=s+ims.VectorSize();
   for(;s<end; d++) {
      if ((*(d)=*(s++)) > max)
	 max = *d;
      else if (*d < min)
	 min = *d;
   }
   imsource->setMinval(min);
   imsource->setMaxval(max);
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show();
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imc2dsf &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   imsource->setType("Imc2dsf");
   imsource->setContent("Image, Color,  2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   sprintf(etc,"Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(true);

   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands(); b++) {
      Float *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Float *end=sx+ims.VectorSize();
      for(;sx<end; dx++) {
	 if ((*(dx)=(Float)*(sx++)) > max)
	    max=*dx;
	 else if (*dx < min)
	    min=*dx;
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin); 
   }else
      visual=new ImageModel(imsource,nomentree);
   visual->show();
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imc3dsf &ims, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Imc3dsf");
   imsource->setContent("Image, Color,  3D");
   sprintf(size,"%ld rows x %ld ncols x %ld depth",(long)ims.Height(),(long)ims.Width(),(long)ims.Depth());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   sprintf(etc,"Color space</th><td>%s",ColorName(ims.ColorSpace()));
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(true);
   
   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands();b++) {
      Float *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Float *end=ims.Vector(b)+ims.VectorSize();
      for(;sx<end;) {
	 if (*sx>max) max=*sx;
	 else if (*sx<min) min=*sx;
	 *(dx++)=*(sx++);
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);      
   visual->show();

   return visual;
}

QWidget *Visu( Reg2d &rgs, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,1,rgs.Height(), rgs.Width());
   imsource->setType("Reg2d");
   imsource->setContent("Region map, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)rgs.Height(),(long)rgs.Width());
   imsource->setSize(size);
   imsource->setLabel("integer 32-bits");
   sprintf(etc,"%ld",(long)rgs.Labels());
   imsource->setLabelMax(etc);
   imsource->setPixel(0);
   imsource->setMinval(0);
   imsource->setMaxval(rgs.Labels());
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
   imsource->setDynamisable(false);
   imsource->setRegion(true);

   Float *d=imsource->Vector(0);
   Ulong *s=rgs.Vector();
   for(;s<rgs.Vector()+rgs.Width()*rgs.Height(); *(d++)=(Float)*(s++)) ;

   rgs.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show();
   return visual;
}

QWidget *Visu( Reg3d& rgs, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,rgs.Depth(),rgs.Height(), rgs.Width());
   imsource->setType("Reg3d");
   imsource->setContent("Region map, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)rgs.Depth(),(long)rgs.Height(),(long)rgs.Width());
   imsource->setSize(size);
   imsource->setLabel("integer 32-bits");
   sprintf(etc,"%ld",(long)rgs.Labels());
   imsource->setLabelMax(etc);
   imsource->setPixel(0);
   imsource->setMinval(0);
   imsource->setMaxval(rgs.Labels());
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
   imsource->setDynamisable(false);
   imsource->setRegion(true);

   Float *d=imsource->Vector(0);
   Ulong *s=rgs.Vector();
   for(;s<rgs.Vector()+rgs.Depth()*rgs.Width()*rgs.Height(); *(d++)=(Float)*(s++)) ;

   rgs.Delete();

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show();
   return visual;
}

QWidget *Visu( Graph2d &grs, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,1,grs.Height(), grs.Width());
   imsource->setType("Graph2d");
   imsource->setContent("Graph, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)grs.Height(),(long)grs.Width());
   imsource->setSize(size);
   imsource->setNode("float, Edge: float");
   sprintf(etc,"%ld",(long)grs.Size());
   imsource->setTotalNodes(etc);
   imsource->setPixel(0);
   imsource->setGraph(true);
   imsource->setMinval(0);
   imsource->setMaxval(grs.Size());
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
   imsource->setDynamisable(false);

   int n;
   Float *d=imsource->Vector(0);
   for(;d<imsource->Vector(0)+grs.Width()*grs.Height(); *(d++)=0) ;

   // First of all : the links with color=1.
   for (n=0; n<grs.Size(); n++) {
      if (grs[n]) {
	 for (GEdge *ls=grs[n]->Neighbours();ls;ls=ls->Next()) {
 	    DrawLine(imsource,Point3d(0,grs[n]->seed.y,grs[n]->seed.x),
		     Point3d(0,grs[ls->Node()]->seed.y,grs[ls->Node()]->seed.x),1.0);
	 }
      }
   }
   // Then the nodes.
   for (n=0; n<grs.Size(); n++)
      if (grs[n])
	 (*imsource)(0,0,grs[n]->seed.y,grs[n]->seed.x)=n; //grs[n]->value;

   grs.Delete();   

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);

   visual->show();

   return visual;
}

QWidget *Visu( Graph3d& grs, Imx3duc *imageDessin ) {
   ImageSource *imsource=new ImageSource(1,grs.Depth(),grs.Height(), grs.Width());
   imsource->setType("Graph3d");
   imsource->setContent("Graph, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)grs.Depth(),(long)grs.Height(),(long)grs.Width());
   imsource->setSize(size);
   imsource->setNode("float, Edge: float");
   sprintf(etc,"%ld",(long)grs.Size());
   imsource->setTotalNodes(etc);
   imsource->setPixel(0);
   imsource->setMinval(0);
   imsource->setMaxval(grs.Size());
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
   imsource->setDynamisable(false);
   imsource->setGraph(true);
   int n;   
   Float *d=imsource->Vector(0);
   for(;d<imsource->Vector(0)+grs.Depth()*grs.Width()*grs.Height(); *(d++)=0) ;
   
   // First of all : the links.
   for (n=0; n<grs.Size(); n++) {
      if (grs[n]) {
	 for (GEdge *ls=grs[n]->Neighbours();ls;ls=ls->Next()) {
	    DrawLine(imsource,grs[n]->seed,grs[ls->Node()]->seed, 1.0 /* ls->weight */);
	 }
      }
   }
   // Then the nodes.
   for (n=0; n<grs.Size(); n++)
      if (grs[n])
	 (*imsource)(0,grs[n]->seed.z,grs[n]->seed.y,grs[n]->seed.x)=n; // grs[n]->value;

   grs.Delete();   
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show();
   return visual;
}

QWidget *Visu( Img1duc &ims, Imx3duc *imageDessin ) {
   // Seek for the min and max values.  
   Img1duc::ValueType *s=ims.Vector();
   Float max=*s;
   const Float min=0;
   for (;s<ims.Vector()+ims.VectorSize(); s++) {
      if ((Float)(*s) > max)
	 max=(Float)(*s);
   }

   int height=500;
   if (max-min+1 < height)
      height=max-min+1;
   
   ImageSource *imsource=new ImageSource(1,1,height+1,ims.Width());
   imsource->setType("Img1duc");
   imsource->setContent("Image, Gray level, 1D");
   sprintf(size,"%ld ncols",(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel(" integer 8-bits");
   imsource->setMinval(0);
   imsource->setMaxval(MAXUINT1);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(false);

   for(int x=0; x<imsource->Width(); x++)
      for(int y=0; y<imsource->Height(); y++)
	 (*imsource)(0,0,y,x)=0;
   
   Float step=(Float)height/(Float)(fabs(max-min+1));
   int y=0,y1;
   y=(int)(height-1-((ims[0]-min)*step));
   for(int i=1; i<ims.Width();i++) {
      y1=(int)(height-1-((ims[i]-min)*step));
      DrawLine(imsource,Point3d(0,y,i-1),Point3d(0,y1,i),255.0F);
      y=y1;
   }
   ims.Delete();

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

QWidget *Visu( Img1dsl &ims, Imx3duc *imageDessin ) {
   // Seek for the min and max values.  
   Long *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   for (;s<ims.Vector()+ims.VectorSize(); s++) {
      if ((Float)(*s) > max)
	 max=(Float)(*s);
      if ((Float)(*s) < min)
	 min=(Float)(*s);
   }

   int maxHeight=500;
   int height;
   if (min>=0) {
      if (max+1<maxHeight) height=max+1;
      else if (max-min+1<maxHeight) height=max-min+1;
      else height=maxHeight;
   } else {
      if (abs(max-min+1) < maxHeight)
	 height=abs(max-min+1);
      else height=maxHeight;
   }

   ImageSource *imsource=new ImageSource(1,1,height+1,ims.Width());
   imsource->setType("Img1dsl");
   imsource->setContent("Image, Gray level 1D");
   sprintf(size,"%ld ncols",(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(false);
   
   for(int x=0; x<imsource->Width(); x++)
      for(int y=0; y<imsource->Height(); y++)
	 (*imsource)(0,0,y,x)=0;
   
   imsource->setMaxval(MAXUINT1);
   imsource->setMinval(0);

   Float step=(Float)height/(Float)(fabs(max-min+1));
   int y=0,y1;
   y=(int)(height-1-((ims[0]-min)*step));
   for(int i=1; i<ims.Width();i++) {
      y1=(int)(height-1-((ims[i]-min)*step));
      DrawLine(imsource,Point3d(0,y,i-1),Point3d(0,y1,i),255.0F);
      y=y1;
   }
   ims.Delete();

   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

QWidget *Visu( Img1dsf &ims, Imx3duc *imageDessin ) {
   Float *s=ims.Vector();
   Float max=*s;
   Float min=*s;
   for(;s<ims.Vector()+ims.VectorSize(); s++) {
      if ((*s) > max)
	 max=*s;
      if ((*s) < min)
	 min=*s;
   }

   ImageSource *imsource=new ImageSource(1,1,256, ims.Width());
   imsource->setType("Img1dsf");
   imsource->setContent("Image, Gray level 1D");
   sprintf(size,"%ld ncols",(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(false);
   imsource->setMaxval(MAXUINT1);
   imsource->setMinval(0);

   int height=500;
   if (max-min+1 < height)
      height=max-min+1;
   if (height <=1) height=500;
   
   for(int x=0; x<imsource->Width(); x++)
      for(int y=0; y<imsource->Height(); y++)
	 (*imsource)(0,0,y,x)=0;
   
   Float step=255.0F/(Float)(fabs(max-min));
   for(int i=0; i<ims.Width();i++)
      (*imsource)(0,0,255-(int)((ims[i]-min)*step),i)=255;
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree);
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imx2duc &ims, Imx3duc *imageDessin ) {
   // can't display more than 3 bands...
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx2duc (2D multispectral image of bytes)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*)\n");
      strcat(text,"or into a 3D image where each band becomes a plane.\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }
   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,1,ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   
   imsource->setType("Imx2duc");
   imsource->setContent("Image, Multispectral level, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 8-bits");
   sprintf(etc, "Number of bands</th><td>%d",ims.Bands());
   imsource->setEtc(etc);
   imsource->setMinval(0);
   imsource->setMaxval(MAXUINT1);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(true);
   
   for (int b=0; b<ims.Bands(); b++) {
      Uchar *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Uchar *end=sx+ims.VectorSize();
      for(;sx<end;) {
 	 *(dx++)=(Float)*(sx++);
      }
   }
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imx2dsl &ims, Imx3duc *imageDessin ) {
   // can't display more than 3 bands...
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx2dsl (2D multispectral image of longs)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*)\n");
      strcat(text,"or into a 3D image where each spectrum becomes a plane.\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }
   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,1,ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   
   imsource->setType("Imx2dsl");
   imsource->setContent("Image, Multispectral level, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   sprintf(etc, "Number of bands</th><td>%d",ims.Bands());
   imsource->setEtc(etc);
   imsource->setMinval(0);
   imsource->setMaxval(MAXLONG);
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(true);
   
   for (int b=0; b<ims.Bands(); b++) {
      Long *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Long *end=sx+ims.VectorSize();
      for(;sx<end;) {
 	 *(dx++)=(Float)*(sx++);
      }
   }
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/*
 * Optimized
 */
QWidget *Visu( Imx2dsf &ims, Imx3duc *imageDessin ) {
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx2duc (2D multispectral image of floats)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*)\n");
      strcat(text,"or into a 3D image where each spectrum becomes a plane.\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }

   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,1,ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,1,ims.Height(),ims.Width());
   
   imsource->setType("Imx2dsf");
   imsource->setContent("Image, Multispectral level, 2D");
   sprintf(size,"%ld rows x %ld ncols",(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   sprintf(etc, "Number of bands</th><td>%d",ims.Bands());
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(true);
   
   Float max=*ims.Vector(0);
   Float min=max;
   int bands = (ims.Bands()>3)? 3:ims.Bands();
   for (int b=0; b<bands; b++) {
      Float *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Float *end=sx+ims.VectorSize();
      for(;sx<end; dx++) {
 	 if ((*(dx)=*(sx++)) > max)
	    max=*dx;
	 else if (*dx < min)
	    min=*dx;
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imx3duc &ims, Imx3duc *imageDessin ) {
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx3duc (3D multispectral image of bytes)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*).\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }

   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   imsource->setType("Imx3duc");
   imsource->setContent("Image, Multispectral level, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 8-bits");
   sprintf(etc, "Number of bands</th><td>%d",ims.Bands());
   imsource->setEtc(etc);
   imsource->setMinval(0);
   imsource->setMaxval(MAXUINT1);
   imsource->setTypeval(ImageSource::tchar);
   imsource->setColored(true);
  
   for (int b=0; b<ims.Bands(); b++) {
      Uchar *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Uchar *end=sx+ims.VectorSize();
      for(;sx<end;) {
 	 *(dx++)=(Float)*(sx++);
      }
   }
   ims.Delete();
  
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/*
 * Optimized
 */
QWidget *Visu( Imx3dsl &ims, Imx3duc *imageDessin ) {
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx3dsl (3D multispectral image of longs)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*).\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }
   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   
   imsource->setType("Imx3dsl");
   imsource->setContent("Image, Multispectral level, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("integer 32-bits");
   sprintf(etc, "Number of bands</th><td>%ld",(long)ims.Bands());
   imsource->setEtc(etc);
   imsource->setMinval(0);
   imsource->setMaxval(MAXLONG);
   imsource->setTypeval(ImageSource::tlong);
   imsource->setColored(true);

   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands(); b++) {
      Long *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Long *end=sx+ims.VectorSize();
      for(;sx<end;) {
	 if (*sx>max) max=*sx;
	 else if (*sx<min) min=*sx;
	 *(dx++)=(Float)*(sx++);
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   ims.Delete();
   
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/*
 * Optimized.
 */
QWidget *Visu( Imx3dsf &ims, Imx3duc *imageDessin ) {
   if (ims.Bands()<=0 || ims.Bands() > 3) {
      QWidget *visual;
      char text[5120]="Type: Imx3dsf (3D multispectral image of floats)\n";
      char tmp[100];
      sprintf(tmp,"Number of planes: %d\n",ims.Bands());
      strcat(text,tmp);
      strcat(text,"\nThis image can't be displayed as such.\n");
      strcat(text,"Convert it into several viewable images (with pimx2*).\n");
      visual=new InfoView(text,nomentree); 
      visual->show();
      return visual;
   }

   ImageSource *imsource;
   if (ims.Bands() == 1)
      imsource=new ImageSource(1,ims.Depth(),ims.Height(),ims.Width());
   else
      imsource=new ImageSource(3,ims.Depth(),ims.Height(),ims.Width());
   
   imsource->setType("Imx3dsf");
   imsource->setContent("Image, Multispectral level, 3D");
   sprintf(size,"%ld planes x %ld rows x %ld ncols",(long)ims.Depth(),(long)ims.Height(),(long)ims.Width());
   imsource->setSize(size);
   imsource->setPixel("float 32-bits");
   sprintf(etc, "Number of bands</th><td>%d",ims.Bands());
   imsource->setEtc(etc);
   imsource->setTypeval(ImageSource::tfloat);
   imsource->setColored(true);

   Float max=*ims.Vector(0);
   Float min=max;
   for (int b=0; b<ims.Bands(); b++) {
      Float *sx=ims.Vector(b);
      Float *dx=imsource->Vector(b);
      const Float *end=sx+ims.VectorSize();
      for(;sx<end;dx++) {
	 if ((*(dx)=*(sx++)) > max)
	    max=*dx;
	 else if (*dx < min)
	    min=*dx;
      }
   }
   imsource->setMaxval(max);
   imsource->setMinval(min);
   
   ims.Delete();
  
   QWidget *visual;
   if (_DRAWINGMODE) {
      *imageDessin=0;
      visual=new ImageModel(imsource,nomentree,imageDessin);
   } else
      visual=new ImageModel(imsource,nomentree); 
   visual->show(); 
   return visual;
}

/**
 * Collection.
 */
QWidget *Visu( Collection &cols ) {
   QWidget *visual;
   visual=new InfoView(cols,nomentree); 
   visual->show();
   return visual;
}

/**
 * Reads input file in objs[0],
 * and builds output file in objd[0] (for dessin only).
 * @param objs the array of input images.
 * @param objd the array of output images.
 * @return the process number of the viewer or FAILURE.
 */
QWidget *OpenFile( Pobject *objs, Pobject **objd ) {
   QWidget *visual=NULL;

#ifdef DEBUG
   temps_deb=clock(); // DEBUG
#endif

   switch(objs->Type()) {
   case Po_Img2duc:{
      Img2duc* const ims=(Img2duc*)objs;
      Imx3duc* imd=NULL;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];     
      }
      visual = Visu(*ims,imd);
   } break;
   case Po_Img2dsl:{
      Img2dsl* const ims=(Img2dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img2dsf:{
      Img2dsf* const ims=(Img2dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Reg2d:{
      Reg2d* const rgs=(Reg2d*)objs;
      if ( rgs->Width()<=0 || rgs->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input region map size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(rgs->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*rgs,imd);
   }break;
   case Po_Reg3d:{
      Reg3d* const rgs=(Reg3d*)objs;
      if ( rgs->Width()<=0 || rgs->Height()<=0 || rgs->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input region map size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(rgs->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*rgs,imd);
   }break;
   case Po_Graph2d:{
      Graph2d* const grs=(Graph2d*)objs;
      if ( grs->Width()<=0 || grs->Height()<=0 ){
	 std::cerr<<"Error "<<progname<<": Bad input graph size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(grs->Height(),grs->Width());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*grs,imd);
   }break;
   case Po_Graph3d:{
      Graph3d* const grs=(Graph3d*)objs;
      if ( grs->Width()<=0 || grs->Height()<=0 || grs->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input graph size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(grs->Depth(),grs->Height(),grs->Width());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*grs,imd);
   }break;
   case Po_Imc2duc:{
      Imc2duc* const ims=(Imc2duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imc2dsl:{
      Imc2dsl* const ims=(Imc2dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imc2dsf:{
      Imc2dsf* const ims=(Imc2dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img3duc:{
      Img3duc* const ims=(Img3duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img3dsl:{
      Img3dsl* const ims=(Img3dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img3dsf:{
      Img3dsf* const ims=(Img3dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imc3duc:{
      Imc3duc* const ims=(Imc3duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imc3dsl:{
      Imc3dsl* const ims=(Imc3dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imc3dsf:{
      Imc3dsf* const ims=(Imc3dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img1duc:{
      Img1duc* const ims=(Img1duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(256,ims->Width());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img1dsl:{
      Img1dsl* const ims=(Img1dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(256,ims->Width());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Img1dsf:{
      Img1dsf* const ims=(Img1dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(256,ims->Width());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imx2duc: {
      Imx2duc* const ims=(Imx2duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img2duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   } break;
   case Po_Imx2dsl: {
      Imx2dsl* const ims=(Imx2dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      // can't display more than 3 bands...
      if (ims->Bands()<=0 || ims->Bands() > 3) return NULL;
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
 	 objd[0]=new Img2duc(ims->Size());
 	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imx2dsf: {
      Imx2dsf* const ims=(Imx2dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 ) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      // can't display more than 3 bands...
      if (ims->Bands()<=0 || ims->Bands() > 3) return NULL;
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
 	 objd[0]=new Img2duc(ims->Size());
 	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imx3duc: {
      Imx3duc* const ims=(Imx3duc*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      // can't display more than 3 bands...
      if (ims->Bands()<=0 || ims->Bands()> 3) return NULL;
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imx3dsl: {
      Imx3dsl* const ims=(Imx3dsl*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      // can't display more than 3 bands...
      if (ims->Bands()<=0 || ims->Bands() > 3) return NULL;
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   }break;
   case Po_Imx3dsf: {
      Imx3dsf* const ims=(Imx3dsf*)objs;
      if ( ims->Width()<=0 || ims->Height()<=0 || ims->Depth()<=0) {
	 std::cerr<<"Error "<<progname<<": Bad input image size\n";
	 exit(2);
      }
      // can't display more than 3 bands...
      if (ims->Bands()<=0 || ims->Bands() > 3) return NULL;
      Imx3duc* imd=NULL;
      if (_DRAWINGMODE) {
	 objd[0]=new Img3duc(ims->Size());
	 imd=(Imx3duc*)objd[0];
      }
      visual = Visu(*ims,imd);
   } break;
   case Po_Collection: {
      Collection* const cols=(Collection*)objs;
      visual = Visu(*cols);
   } break;
   default: break;
   }
#ifdef DEBUG
   printf("temps %f\n",((float)(clock()-temps_deb)/CLOCKS_PER_SEC));
#endif

   return visual;
}

#ifdef MAIN

/*
 * Modify only the following constants, and the function calls.
 */
int main( int argc, char *argv[] ) {
   // Check Qt options.
   int argcQt=1;
   char **argvQt = new char*[argc+1];
   argvQt[0]=argv[0];
   int argcVisu=1;
   char **argvVisu=new char*[argc+1];
   argvVisu[0]=argv[0];
   progname=argv[0];

   // Parses the command line to get the Qt options on the one hand
   // and the visu options on the other hand.
   // Qt options -> all parameters such -xxxx other than -p and -h
   // Visu options: -p -h and the remainder.
   bool noFork=false;

   for (int i=1; i<argc; ) {
      if (argv[i][0] == '-') {
	 if (!strcmp(argv[i],"-h")||!strcmp(argv[i],"-p")||!strcmp(argv[i],"-h")) {
	    argvVisu[argcVisu++]=argv[i++];
	 } else if (!strcmp(argv[i],"-nofork")) {
	    noFork=true; i++;
	 } else {
	    argvQt[argcQt++]=argv[i++];
	    argvQt[argcQt++]=argv[i++];
	 }
      } else {
	 argvVisu[argcVisu++]=argv[i++];
      }
   }

   // Whether the program is named visu or dessin
   // the number of output images and usage are different.
#define PROGNAME "pdraw"
   int realFOUTC;
   char usage[256];
   int realFINC;

   if (strlen(argvVisu[0])>= strlen(PROGNAME) &&
       !strcmp(&argvVisu[0][strlen(argvVisu[0])-strlen(PROGNAME)],PROGNAME)) {
      //cas dessin
      realFINC=1;
      realFOUTC=1;
      _DRAWINGMODE=true;
      sprintf(usage,"usage: %s [-qt_option]* [im_in|-] [im_out|-]", argvVisu[0]);
   } else {
      // cas visu
#ifdef STANDALONE
      if (argcVisu>1)
	 realFINC=1;
      else
	 realFINC=0;
#else
      realFINC=1;
#endif
      realFOUTC=0;
      _DRAWINGMODE=false;
      sprintf(usage,"usage: %s [-qt_option value]* [im_in|-]",argvVisu[0]);
   }

   char error[255];

   // Check the number of arguments or -p option.
   // Print PROTOTYPE : (name - number of parameters - number of inputs - number of outputsk).
   if ((argcVisu>=2)&&(!strcmp(argvVisu[1],"-p"))) {
      sprintf(error,"%s %d %d %d",argv[0], 0,realFINC,realFOUTC);
      std::cout<<error<<std::endl;
      exit(0);
   }

   // Check the number of arguments or -h option.
   // Print USAGE.
   if ((argcVisu<=0) || ((argcVisu>=2)&&(!strcmp(argvVisu[1],"-h")))) {
      sprintf(error, usage, argv[0]);
      std::cerr<<error<<std::endl;
      exit(0);
   }

#ifdef _WIN32
   if (!strcmp(argv[0],PROGNAME) && argc<2) {
      Exit(FAILURE);
   }
#endif

#undef PROGNAME

#ifndef STANDALONE
   if (! (_DRAWINGMODE || noFork)) {
      if (argcVisu==1) {
	 Long pid=fork();
	 switch (pid) {
	    case 0: // child
	       execlp(argv[0], argv[0], "-nofork", NULL);
	       exit(0);
	       break;
	    case -1:
	       exit(-1);
	       break;
	 }
      } else {
	 for (int i=1; i<argcVisu; i++ ) {
	    Long pid=fork();
	    switch (pid) {
	       case 0: // child
		  execlp(argv[0], argv[0], "-nofork", argvVisu[i], NULL);
		  exit(0);
		  break;
	       case -1:
		  exit(-1);
		  break;
	    }
	 }
      }
      Exit(SUCCESS);
   }
#endif

   // Get the input image name or stdin
   Pobject *mask=NULL;
   Pobject *objin[1];
   Pobject *objs[1];
   Pobject *objout[1];
   Pobject *objd[1];

   if (argcVisu>1) nomentree=argvVisu[1];
   else nomentree="stdin";

   int k=1;
   objs[0]=NULL;

#ifdef STANDALONE
   if (argcVisu>1) {
#endif
      objs[0]=LoadFile(((k>=argcVisu)||(!strcmp(argvVisu[k],"-")))? NULL : argvVisu[k], false);
      if (objs[0]==NULL) {
	 if (argcVisu<2 || (!strcmp(argvVisu[1],"-"))) {
	    fprintf(stderr, "Error %s: can't read standard input with image other than Pandore format\n", progname);
	    Exit(FAILURE);
	 }

#ifdef sun
	 freopen("/dev/null", "r", stderr);
#endif
	 QImage *data = new QImage(nomentree);
#ifdef sun
	 freopen("/dev/stderr", "r", stderr);
#endif
	 if (data->width()>0) {
	    if (data->allGray()) {
	       Img2duc *ims = new Img2duc(data->height(),data->width());
	       objs[0] = ims;
	       for (int y=0; y<data->height(); y++) {
		  for (int x=0; x<data->width(); x++) {
		     QRgb pixel = data->pixel(x,y);
		     (*ims)(y,x) = qRed(pixel);
		  }
	       }
	    } else {
	       Imc2duc *ims = new Imc2duc(data->height(),data->width());
	       objs[0] = ims;
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
#ifdef STANDALONE
    }
#endif

#ifndef _WIN32
   if (objs[0]==NULL) {
      std::cerr << "Error "<<progname<<": Bad Pandore format: "<< nomentree << std::endl;
      Exit(FAILURE);
   }
#endif

#ifdef sun
   freopen("/dev/null", "r", stderr);
#endif

   QApplication app(argcQt, argvQt);
   QWidget *visual=NULL;
   if (realFINC > 0) {
      visual=OpenFile(objs[0],objd);
      if (visual) {
	 // Consider only the SON process.
	 app.exec();
	 delete visual;
      }
   } else {
       visual=Visu( );
       if (visual) {
 	 app.exec();
          delete visual;
       }
   }
   
   if (visual) {
      WriteArgs(argcVisu,argvVisu,0,realFINC,realFOUTC,&mask,objin,objs,objout,objd);
   } else {
      PrintErrorFormat(objin,realFINC);
   }
   
   return (visual==NULL);
}

#endif
