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
 * @author Régis Clouard - 2005-10-14 (fix bug for 8 bits display)
 */

/**
 * @file pvisu.cpp
 * @brief Display Pandore objects (images, Graph, region map...)
 */

#include <pandore.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h> 
#include <X11/keysym.h>
#include <X11/IntrinsicP.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/ArrowBG.h>
#include <Xm/MainW.h>

using namespace pandore;

#define MAXCOLS 256	// Number of virtual used colors 256

static int coloffset=0; // Bit shift for the real available colors (2: 64; 1:128; 0: 256)

// Available data type.
typedef enum {tchar, tlong, tfloat} TypeData;

// Properties of the image to be displayed.
class ImageAffichee {
public:
   // Some comments
   char *type;
   char *content;
   char *pixel;
   char size[120];
   char etc[120];

   float minval, maxval;	// Limits of the source values
   float stepval;		// Precision of the fractional source values.
   bool dynamisable;		// Is the dynamic of the color changeable?
   bool colored;		// Color image or Gray levels image.
   Imc3dsf* imsource;		// Logical image -Img3dxx- (What is to be displayed)
   Img3duc* imlogic;
   Uchar rmap[MAXCOLS];		// Original Colormap
   Uchar gmap[MAXCOLS];
   Uchar bmap[MAXCOLS];
   TypeData typeval;
   
   ImageAffichee() :stepval(1) {}
};

ImageAffichee ecran;

/**
 * Returns the name of the colorspace.
 */
char *ColorName(PColorSpace c) {
   switch(c){
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

// Include the source of the window management.
#include "visumotif.cpp"
// Include the definition of the windows.
#include "visuoutil.cpp"

/**
 * Normalizes values from 0 to outbands of the input data vector
 * into the output image.
 * @param s	the source data.
 * @param imd	the destination data.
 * @param outbands the number of levels for the output image.
 */
Errc Recadrage(Float* s, Img3duc& imd, int outbands) {
   outbands--; // [0..outbands-1]
   // Find min,max.
   Float smin,smax;
   Float* ps=s;
   smin= smax=*(ps++);
   for (;ps<s+imd.VectorSize();ps++) {  
      if (smax < *ps) smax = *ps;
      if (smin > *ps) smin = *ps;
   }
   
   // Calculate slope.
   Float slope;
   if (smax-smin==0) 
      slope=1.0F;
   else
      slope=(float)(outbands)/(smax-smin);
   
   // Linear transformation.
   ps=s;
   Uchar* pd=imd.Vector();
   for (;ps<s+imd.VectorSize();ps++,pd++) {
      *pd = (Uchar)((slope*(float)(*ps-smin)));
   }
   return SUCCESS;
}

/**
 * Histogram equalization is used to reduce
 * the number of colors from maxval-minval to nbande.
 * @param s	the source data.
 * @param imd	the destination data.
 * @param outbands the number of levels for the output image.
 */
Errc Egalisation(Float* s, Img3duc& imd, int outbands) {
   std::map<Float,float> histo;
   std::map<Float,float> histoc;

   // Compute histogram and min-max values.
   Float* ps=s;
   Uchar *pl=imd.Vector();
   for (;pl<imd.Vector()+imd.VectorSize();ps++,pl++)
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
   float total=imd.VectorSize()-h0;
   for(;h != histo.end();h++) {
      sum=histoc[h->first]=sum+h->second;
      histoc[h->first]=histoc[h->first] / total;
   }
   ps=s;
   pl=imd.Vector();
   for (;pl<imd.Vector()+imd.VectorSize();ps++,pl++)
      *pl=(Uchar)(histoc[*ps]*(float)outbands);

   histo.clear();
   histoc.clear();

   return SUCCESS;
}

/**
 * Draws a line in imd, from coordinate pti to ptf with color coul.
 */
void TraceDroite(Imc3dsf& imd, Point3d pti, Point3d ptf, Float coul) {
   Long xmin,ymin,zmin;
   Long xmax,ymax,zmax;
   Long i,j,k;
   Long d;
   float penteyx,pentezx;
   float penteyz;
  
   if (pti.x==ptf.x){
      penteyx=pentezx=MAXFLOAT4;
      xmin=xmax=ptf.x;
   }else{
      d=ptf.x-pti.x;
      penteyx=(float)(ptf.y-pti.y)/d;
      pentezx=(float)(ptf.z-pti.z)/d;
      if (pti.x<ptf.x) { xmin=pti.x; xmax=ptf.x; }
      else { xmin=ptf.x; xmax=pti.x; }
   }
   if (pti.y==ptf.y){
      penteyx=penteyz=0.0;
      ymin=ymax=ptf.x;
   }else{
      if (pti.y<ptf.y) { ymin=pti.y; ymax=ptf.y; }
      else { ymin=ptf.y; ymax=pti.y; }
   }
   if (pti.z==ptf.z){
      penteyz=MAXFLOAT4;
      pentezx=0.0;
      zmin=zmax=ptf.z;
   }else{
      penteyz=(float)(ptf.y-pti.y)/(ptf.z-pti.z);
      if (pti.z<ptf.z) { zmin=pti.z; zmax=ptf.z; }
      else { zmin=ptf.z; zmax=pti.z; }
   }
  
   if ((pentezx<-1)||(pentezx>1)) {
      if ((penteyx<-1)||(penteyx>1)) {
	 if (((pentezx<-1)&&(penteyx<pentezx)) || ((pentezx>1)&&(penteyx>pentezx)))
	    for (j=ymin;j<=ymax;j++){
	       d=j-ptf.y;
	       k=(int)(d/penteyx+ptf.x);
	       i=(int)(d/penteyz+ptf.z);
	       imd.X[i][j][k]=coul;
	    }
	 else
	    for (i=zmin;i<=zmax;i++){
	       d=i-ptf.z;
	       j=(int)(d*penteyz+ptf.y);
	       k=(int)(d/pentezx+ptf.x);
	       imd.X[i][j][k]=coul;
	    }
      } else
	 for (k=xmin;k<=xmax;k++){
	    d=k-ptf.x;
	    j=(int)(d*penteyx+ptf.y);
	    i=(int)(d*pentezx+ptf.z);
	    imd.X[i][j][k]=coul;
	 }
   } else
      if ((penteyx<-1)||(penteyx>1))
	 for (j=ymin;j<=ymax;j++){
	    d=j-ptf.y;
	    k=(int)(d/penteyx+ptf.x);
	    i=(int)(d/penteyz+ptf.z);
	    imd.X[i][j][k]=coul;
	 }
      else
	 for (k=xmin;k<=xmax;k++){
	    d=k-ptf.x;
	    j=(int)(d*penteyx+ptf.y);
	    i=(int)(d*pentezx+ptf.z);
	    imd.X[i][j][k]=coul;
	 }
}

/**
 * Scans picture until it finds more
 * than 'maxcols' different colors.
 */
Errc Suffisant(Imc3duc &ims, Img3duc &imd, int maxcols) {
   unsigned long colors[MAXCOLS],col;
   int i, j, k, c, nc, low, high, mid;
   
   // Put the first color in the table by hand.
   nc = 0;  mid = 0;  
   
   for (k=0; k<ims.Depth(); k++){
      for (i=0; i<ims.Height(); i++){
	 for (j=0; j<ims.Width(); j++){
	    col  = (Ulong)(ims.X[k][i][j]) <<16;
	    col += (Ulong)(ims.Y[k][i][j]) <<8;
	    col += (Ulong)ims.Z[k][i][j];
	    
	    low = 0;
	    high = nc-1;
	    while (low <= high){  // Binary search the 'colors' array
	       mid = (low+high)/2; 
	       if (col < colors[mid])
		  high = mid - 1;
	       else 
		  if (col > colors[mid]) 
		     low = mid + 1; 
		  else
		     break;
	    }
	    if (high < low){	// Not found -> Add it.
	       if (nc>=maxcols-1){
		  return FAILURE; // Too much colors.
	       }
	       for (c=nc; c>=low; c--)
		  colors[c+1]=colors[c];
	       colors[low] = col;
	       nc++;
	    }
	 } 
      }
   }
   // Yes, enough!
   // Map pixel values in ims into colormap.
   for (k=0; k<ims.Depth(); k++){
      for (i=0; i<ims.Height(); i++){
	 for (j=0; j<ims.Width(); j++){
	    col  = (Ulong)(ims.X[k][i][j]) << 16;
	    col += (Ulong)(ims.Y[k][i][j]) << 8;
	    col += (Ulong)ims.Z[k][i][j];
	    
	    low = 0;  high = nc-1;
	    while (low <= high) {	 // Find color.
	       mid = (low+high)/2;
	       if      (col < colors[mid]) high = mid - 1;
	       else if (col > colors[mid]) low  = mid + 1;
	       else break;
	    }
	    
	    if (high < low) {
	       exit(1); // Big problem !!!!
	    }
	    imd[k][i][j]= (Uchar)mid;
	 }
      }
   }
   // Set the related colormap.
   for (c=0; c<nc; c++){
      ecran.rmap[c] = (colors[c]>>16);
      ecran.gmap[c] = (colors[c]>>8) & 0xff;
      ecran.bmap[c] =  colors[c]     & 0xff;
   }
   
   return SUCCESS;
}

/**
 * Converts 24 bits image (imi) to 8 bits image (imd).
 * Reduces the number of colors  by seeking peer `colors'.
 */
Errc Reduit24en8(Imc3duc &imi, Img3duc &imd) {
#define RMASK      0xe0
#define GMASK      0xe0
#define BMASK      0xc0 
   int  r1,g1,b1;
   int  *line1,*line2,*l1,*l2;
   int  k,i,j,val;

   // Initialize the colormap.
   for (i=0; i<MAXCOLS; i++){
      ecran.rmap[i] = (((((i>>coloffset)<<coloffset)    )& RMASK)*255 + RMASK/2) / RMASK;
      ecran.gmap[i] = (((((i>>coloffset)<<coloffset)<<3) & GMASK)*255 + GMASK/2) / GMASK;
      ecran.bmap[i] = (((((i>>coloffset)<<coloffset)<<6) & BMASK)*255 + BMASK/2) / BMASK;
   }
   
   // Use (RRRGGGBB) for a pixel encoding.
   if ((!(line1=(int *)malloc(imi.Width()*3 * sizeof(int)))) ||
       (!(line2=(int *)malloc(imi.Width()*3 * sizeof(int)))))
      return(FAILURE);

   for(k=0;k<imi.Depth(); k++){
      // Get first line of image.
      for (j=0; j<imi.Width();j++){
	 line2[3*j] = (int)imi.X[k][0][j];
	 line2[(3*j)+1] = (int)imi.Y[k][0][j];
	 line2[(3*j)+2] = (int)imi.Z[k][0][j];
      }
      //
      for (i=0; i<imi.Height(); i++){
	 l1 = line1; line1 = line2; line2 = l1;
	 if (i<imi.Height()-1){  
	    for (j=0; j<imi.Width(); j++){ // Get next line.
	       line2[3*j]     = (int)imi.X[k][i+1][j];
	       line2[(3*j)+1] = (int)imi.Y[k][i+1][j];
	       line2[(3*j)+2] = (int)imi.Z[k][i+1][j];
	    }
	 }
	 for (j=0, l1=line1, l2=line2; j<imi.Width();j++){
	    r1 = *l1++;g1 = *l1++; b1 = *l1++;
	    ECRETE(r1,0,MAXCOLS-1); ECRETE(g1,0,MAXCOLS-1); ECRETE(b1,0,MAXCOLS-1);
	    // Choose actual pixel value.
	    val = (((r1&RMASK)|((g1&GMASK)>>3)|((b1&BMASK)>>6)));
	    imd[k][i][j] = (Uchar)val;
	    // Compute color.
	    r1 -= ecran.rmap[val];
	    g1 -= ecran.gmap[val];
	    b1 -= ecran.bmap[val];
	    // Add fractions of errors to adjacent pixels
	    if (j<imi.Width()-1) {  /* adjust RIGHT pixel */
	       l1[0] += (r1*7) / 16;
	       l1[1] += (g1*7) / 16;
	       l1[2] += (b1*7) / 16;
	    }
	    
	    if (i<imi.Height()-1) {	/* do BOTTOM pixel */
	       l2[0] += (r1*5) / 16;
	       l2[1] += (g1*5) / 16;
	       l2[2] += (b1*5) / 16;
	       
	       if (j>0) {  /* do BOTTOM LEFT pixel */
		  l2[-3] += (r1*3) / 16;
		  l2[-2] += (g1*3) / 16;
		  l2[-1] += (b1*3) / 16;
	       }
	       
	       if (j<imi.Width()-1) {  /* do BOTTOM RIGHT pixel */
		  l2[3] += (r1)/16;
		  l2[4] += (g1)/16;
		  l2[5] += (b1)/16;
	       }
	       l2 += 3;
	    }
	 }
      }
   }

   free(line1);
   free(line2);
   return SUCCESS;
}

/**
 * Converts 24 bits image (ims) to 8 bits image (iml).
 * If not enough color, compresses them.
 * @param ims : image in
 * @param iml : image out
 */
void Convertie24en8(Imc3duc &ims, Img3duc &imd) {
   if (!Suffisant(ims,imd, MAXCOLS>>coloffset)) // No color compression needed
      Reduit24en8(ims,imd); // Color compression needed.
}

/**
 * Convert range value [0-225] to [0-MAXCOLS[
 * Original map : gray levels.
 * Build two images :
 * 1- source image with true values [minval .. maxval] (type: Img3dsf)
 * 2- logic image with drawable values [0..MAXCOLS] (type : Img3duc)
 */
Errc Visu(Img2duc& ims, Img2duc& imd) {
   // Image properties.
   ecran.type    = "Type: Img2duc";
   ecran.content = "Content: Image, Gray level, 2D";
   ecran.pixel   = "Pixel: integer 8-bits";
   sprintf(ecran.size,"Size: %ld rows x %ld cols",ims.Height(), ims.Width());
   ecran.etc[0]  = '\0';

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT1;
   ecran.typeval=tchar;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   ecran.imlogic=new Img3duc(1,ims.Height(),ims.Width());
   Uchar *l=ecran.imlogic->Vector();
   Float *d=ecran.imsource->VectorX();
   Uchar *s=ims.Vector();
   for(;s<ims.Vector()+ims.Width()*ims.Height();){
      *(d++)=(float)*s;
      *(l++)=*(s++);
   }

   ims.Delete();
   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   imd=0;
   tracesdessin=imd.Vector();
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Normalization: Convert range value [0-225] to [0-MAXCOLS[
 * Original colormap : gray levels.
 */
Errc Visu(Img3duc& ims, Img3duc& imd) {
   // Image properties.
   ecran.type    = "Type: Img3duc";
   ecran.content = "Content: Image, Gray level, 3D";
   ecran.pixel   = "Pixel: integer 8-bits";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",ims.Depth(),ims.Height(), ims.Width());
   ecran.etc[0]  = '\0';

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT1;
   ecran.typeval=tchar;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Depth(),ims.Height(),ims.Width());
   ecran.imlogic=new Img3duc(ims.Depth(),ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX(); 
   Uchar *s=ims.Vector();
   Uchar *l=ecran.imlogic->Vector();
   for(;s<ims.Vector()+ims.Depth()*ims.Width()*ims.Height();){
      *(l++)=*s;
      *(d++)=(Float)*(s++);
   }
   
   ims.Delete();

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imsource;
   delete ecran.imlogic;

   return SUCCESS;
}

/**
 * Displays char 2d color images.
 */
Errc Visu(Imc2duc& ims, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Imc2duc";
   ecran.content = "Content: Image, Color, 2D";
   ecran.pixel = "Pixel integer 8-bits";
   sprintf(ecran.size,"Size : %ld rows x %ld ncols",ims.Height(), ims.Width());
   sprintf(ecran.etc, "Space: %s",ColorName(ims.ColorSpace()));
   
   // Normalizes the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT1;
   ecran.typeval = tchar;
   ecran.colored=true;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   Imc3duc *imi=new Imc3duc(1,ims.Height(), ims.Width());
   Uchar *ix=imi->VectorX(); Uchar *iy=imi->VectorY(); Uchar *iz=imi->VectorZ();
   Uchar *sx=ims.VectorX(); Uchar *sy=ims.VectorY(); Uchar *sz=ims.VectorZ();
   Float *dx=ecran.imsource->VectorX(); Float *dy=ecran.imsource->VectorY(); Float *dz=ecran.imsource->VectorZ();
   for(;sx<ims.VectorX()+ims.Width()*ims.Height();){
      *(dx++)=*sx; *(dy++)=*sy; *(dz++)=*sz;
      *(ix++)=*(sx++); *(iy++)=*(sy++); *(iz++)=*(sz++);
   }
  
   // Reduces the number of colors (24 -> 8 bits).
   ims.Delete();
   ecran.imlogic=new Img3duc(imi->Size());
   Convertie24en8(*imi,*ecran.imlogic);

   delete imi;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display char 3d color image.
 * Normalization: Convert range value [0-225] to [0-MAXCOLS[
 * Original colormap : gray levels.
 */
Errc Visu(Imc3duc& ims, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Imc3duc";
   ecran.content = "Content: Image, Color, 3D";
   ecran.pixel = "Pixel: integer 8-bits";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",ims.Depth(),ims.Height(), ims.Width());
   sprintf(ecran.etc, "Space: %s",ColorName(ims.ColorSpace()));
   
   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT1; 
   ecran.typeval = tchar;
   ecran.colored=true;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Depth(),ims.Height(),ims.Width());
   Uchar *sx=ims.VectorX();  Uchar *sy=ims.VectorY();  Uchar *sz=ims.VectorZ();
   Float *dx=ecran.imsource->VectorX();  Float *dy=ecran.imsource->VectorY();  Float *dz=ecran.imsource->VectorZ();
   for(;sx<ims.VectorX()+ims.Depth()*ims.Width()*ims.Height();){
      *(dx++)=*(sx++); *(dy++)=*(sy++); *(dz++)=*(sz++);
   }

   // Build the first normalized image.
   Imc3duc *imi=new Imc3duc(ims.Depth(),ims.Height(), ims.Width());
   sx=ims.VectorX();  sy=ims.VectorY();  sz=ims.VectorZ();
   Uchar *ix=imi->VectorX();  Uchar *iy=imi->VectorY(); Uchar *iz=imi->VectorZ();
   for(;sx<ims.VectorX()+ims.Depth()*ims.Width()*ims.Height();){
      *(ix++)=*(sx++); *(iy++)=*(sy++); *(iz++)=*(sz++);
   }
  
   // Reduce the number of colors (24 -> 8 bits).
   ims.Delete();
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   Convertie24en8(*imi,*ecran.imlogic);
   delete imi;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imsource;
   delete ecran.imlogic;

   return SUCCESS;
}

/**
 * Set nbcol random color to a MAXCOLS  cols colormap.
 */
void randomColor() {
   // Random colormap [0->maxols]
   for (int i=0; i<MAXCOLS>>coloffset; i++){
      ecran.rmap[i] = (Uchar)rand() & 0x00ff;
      ecran.gmap[i] = (Uchar)rand() & 0x00ff;
      ecran.bmap[i] = (Uchar)rand() & 0x00ff;
      if ((ecran.rmap[i] | ecran.gmap[i] | ecran.bmap[i]) == 0){ ecran.rmap[i]=(Uchar)rand() & 0x00ff;}
   }
   for (int j=1; j<1<<coloffset; j++)
      for (int i=0; i<MAXCOLS>>coloffset; i++){
	 ecran.rmap[i+(j*MAXCOLS>>coloffset)]=ecran.rmap[i];
	 ecran.gmap[i+(j*MAXCOLS>>coloffset)]=ecran.gmap[i];
	 ecran.bmap[i+(j*MAXCOLS>>coloffset)]=ecran.bmap[i];
      }
   ecran.rmap[0]=ecran.gmap[0]=ecran.bmap[0]=0;
}

/**
 * Display 2d region map.
 * Ecran.Imlogical values between [minval .. maxval]
 */
Errc Visu(Reg2d& rgs, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Reg2d";
   ecran.content = "Description: Region map, 2D";
   ecran.pixel="Label: integer 32-bits";
   sprintf(ecran.size,"Size: %ld rows x %ld ncols",rgs.Height(), rgs.Width());
   sprintf(ecran.etc,"Label max: %ld",rgs.Labels());

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=rgs.Labels();
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=false;

   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;

   ecran.imsource=new Imc3dsf(1,rgs.Height(),rgs.Width());
   Float *d=ecran.imsource->VectorX();
   Ulong *s=rgs.Vector();
   for(;s<rgs.Vector()+rgs.Width()*rgs.Height(); *(d++)=(Float)*(s++));

   // Normalize the logic image.
   ecran.imlogic=new Img3duc(1,rgs.Height(),rgs.Width());
   Uchar *l=ecran.imlogic->Vector();
   // (values 0->MAXCOLS) ... 0 is special.
   s=rgs.Vector();
   for(;s<rgs.Vector()+rgs.Width()*rgs.Height();l++,s++)
	 *l=(*s)? (*s-1)%(MAXCOLS-1)+1 : 0;

   rgs.Delete();
   randomColor();
   
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}


/**
 * Display 3d region map.
 */
Errc Visu(Reg3d& rgs, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Reg3d";
   ecran.content = "Content: Region map, 3D";
   ecran.pixel = "Label: integer 32-bits";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",rgs.Depth(),rgs.Height(), rgs.Width());
   sprintf(ecran.etc,"Label max: %ld",rgs.Labels());

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=rgs.Labels();
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=false;

   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;

   ecran.imsource=new Imc3dsf(rgs.Depth(),rgs.Height(),rgs.Width());

   Float *d=ecran.imsource->VectorX();
   Ulong *s=rgs.Vector();
   for(;s<rgs.Vector()+rgs.Depth()*rgs.Width()*rgs.Height(); *(d++)=(Float)*(s++));

   // Normalize the logic image.
   ecran.imlogic=new Img3duc(rgs.Depth(),rgs.Height(),rgs.Width());
   Uchar *l=ecran.imlogic->Vector();
   s=rgs.Vector();
   // (values 0->MAXCOLS) ... 0 is special.
   for(;s<rgs.Vector()+rgs.Depth()*rgs.Width()*rgs.Height();l++,s++)
      *l=(*s)? (*s-1)%(MAXCOLS-1)+1 : 0;

   rgs.Delete();
   randomColor();
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display 2d graph.
 */
Errc Visu(Graph2d& grs, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Graph2d";
   ecran.content = "Content: Graph, 2D";
   ecran.pixel = "Node: float, Edge: float";
   sprintf(ecran.size,"Size: %ld rows x %ld ncols",grs.Height(),grs.Width());
   sprintf(ecran.etc,"Total nodes: %ld",grs.Size());
   
   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=grs.Size();
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=false;

   ecran.imsource=new Imc3dsf(1,grs.Height(),grs.Width());
   Float *d=ecran.imsource->VectorX();
   for(;d<ecran.imsource->VectorX()+grs.Width()*grs.Height(); *(d++)=0);

   // First of all : the links with color=1.
   for (int n=0; n<grs.Size(); n++){
      if (grs[n]){
	 for (GEdge *ls=grs[n]->Neighbours();ls;ls=ls->Next()){
	    TraceDroite(*ecran.imsource,Point3d(0,grs[n]->seed.y,grs[n]->seed.x),
			Point3d(0,grs[ls->Node()]->seed.y,grs[ls->Node()]->seed.x),1.0);
	 }
      }
   }
   // Then the nodes.
   for (int n=0; n<grs.Size(); n++)
      if (grs[n])
	 (*ecran.imsource).X[0][grs[n]->seed.y][grs[n]->seed.x]=n; //grs[n]->value;

   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...

   ecran.imlogic=new Img3duc(1,grs.Height(),grs.Width());
   d=ecran.imsource->VectorX();
   Uchar *l=ecran.imlogic->Vector();
   // (values 0->MAXCOLS) ... 0 is special.
   
   for(;l<ecran.imlogic->Vector()+grs.Width()*grs.Height();l++,d++)
      *l=(*d)? ((int)*d-1)%(MAXCOLS-1)+1 : 0;

   grs.Delete();   
   randomColor();

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}


/**
 * Display 2d graph.
 */
Errc Visu(Graph3d& grs, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Graph3d";
   ecran.content = "Content: Graph, 3D";
   ecran.pixel = "Node: float, Edge: float";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",grs.Depth(),grs.Height(),grs.Width());
   sprintf(ecran.etc,"Total nodes: %ld",grs.Size());

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=grs.Size();
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=false;

   ecran.imsource=new Imc3dsf(grs.Depth(),grs.Height(),grs.Width());
   Float *d=ecran.imsource->VectorX();
   for(;d<ecran.imsource->VectorX()+grs.Depth()*grs.Width()*grs.Height(); *(d++)=0);

   // First of all : the links.
   for (int n=0; n<grs.Size(); n++){
      if (grs[n]){
	 for (GEdge *ls=grs[n]->Neighbours();ls;ls=ls->Next()){
	    TraceDroite(*ecran.imsource,grs[n]->seed,grs[ls->Node()]->seed, 1.0 /* ls->weight */);
	 }
      }
   }
   // Then the nodes.
   for (int n=0; n<grs.Size(); n++)
      if (grs[n])
	 (*ecran.imsource).X[grs[n]->seed.z][grs[n]->seed.y][grs[n]->seed.x]=n; // grs[n]->value;

   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...

   ecran.imlogic=new Img3duc(grs.Depth(),grs.Height(),grs.Width());
   d=ecran.imsource->VectorX();
   Uchar *l=ecran.imlogic->Vector();
   for(;l<ecran.imlogic->Vector()+grs.Depth()*grs.Width()*grs.Height();l++,d++)
      *l=(*d)? ((int)*d-1)%(MAXCOLS-1)+1 : 0;

   grs.Delete();   
   randomColor();
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display long 2d gray image.
 * Build two images :
 * 1- source image with true values [ecran.minval .. ecran.maxval] (type: Img3dsf)
 * 2- ecran.imlogic image with drawable values [0..MAXCOLS] (tyep : Img3duc)
 */
Errc Visu(Img2dsl& ims, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Img2dsl";
   ecran.content = "Content: Image, Gray level, 2D";
   ecran.pixel = "Pixel: integer 32-bits";
   sprintf(ecran.size,"Size: %ld rows x %ld ncols", ims.Height(), ims.Width());
   ecran.etc[0] = '\0';

   // Normalize the source data.
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX();
   Long *s=ims.Vector();
   for(;s<ims.Vector()+ims.Width()*ims.Height(); s++, d++){
      if ((*(d)=*(s)) > ecran.maxval)
	 ecran.maxval = *d;
      if (*d < ecran.minval)
	 ecran.minval = *d;
   }
   
   // Normalize the imlogic image.
   ecran.imlogic=new Img3duc(1,ims.Height(),ims.Width());
   if (ecran.minval >= 0 && ecran.maxval <= MAXUINT1){ // case UINT1 -> /2.
      ecran.minval=0;
      Uchar *l=ecran.imlogic->Vector();
      s=ims.Vector();
      for(;s<ims.Vector()+ims.Width()*ims.Height();*(l++)=*(s++));
   }else  if (ecran.maxval-ecran.minval < MAXCOLS){ // case 
      ecran.minval = ecran.maxval-MAXCOLS;
      Uchar *l=ecran.imlogic->Vector();
      s=ims.Vector();
      for(;s<ims.Vector()+ims.Width()*ims.Height();*(l++)=*(s++));
   }else{ // general case.
      ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
      if (ecran.stepval == 0) ecran.stepval=1;
      // Normalize (by default constrast stretching).
      Recadrage(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);
   }

   ims.Delete();

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;
   
   return SUCCESS;
}

/**
 * Display long 3d gray image.
 * Build two images :
 * 1- source image with true values [ecran.minval .. ecran.maxval] (type: Img3dsf)
 * 2- ecran.imlogic image with drawable values [0..MAXCOLS] (tyep : Img3duc)
 */
Errc Visu(Img3dsl& ims, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Img3dsl";
   ecran.content = "Content: Image, Gray level, 3D";
   ecran.pixel = "Pixel: integer 32-bits";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",ims.Depth(),ims.Height(), ims.Width());
   ecran.etc[0] = '\0';
   // Normalize the source data.
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Depth(),ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX();
   Long *s=ims.Vector();
   for(;s<ims.Vector()+ims.Depth()*ims.Width()*ims.Height(); s++, d++){
      if ((*(d)=(Float)*(s)) > ecran.maxval)
	 ecran.maxval = *d;
      if (*d < ecran.minval)
	 ecran.minval = *d;
   }

   // Normalize the logic image.
   ecran.imlogic=new Img3duc(ims.Depth(),ims.Height(),ims.Width());
   if (ecran.minval >= 0 && ecran.maxval <= MAXUINT1){ // case UINT1 -> /2.
      ecran.minval=0;
      Uchar *l=ecran.imlogic->Vector();
      s=ims.Vector();
      for(;s<ims.Vector()+ims.Depth()*ims.Width()*ims.Height();*(l++)=*(s++));
   }else  if (ecran.maxval-ecran.minval < MAXCOLS){ // case UINT1 -> *1
      ecran.minval = ecran.maxval-MAXCOLS;
      Uchar *l=ecran.imlogic->Vector();
      s=ims.Vector();
      for(;s<ims.Vector()+ims.Depth()*ims.Width()*ims.Height();*(l++)=*(s++));
   }else{ // Case MAXUINT4
      ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
      if (ecran.stepval == 0) ecran.stepval=1;
      // Normalize (by default constrast stretching).
      Recadrage(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);
   }

   ims.Delete();  
   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display float 2D gray image.
 * @param ims the input image (for both visu and dessin).
 * @param imd the output image (in case of operatror dessin).
 */
Errc Visu(Img2dsf& ims, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Img2dsf";
   ecran.content = "Content: Image, Gray level, 2D";
   ecran.pixel = "Pixel : float";
   sprintf(ecran.size,"Size: %ld rows x %ld ncols",ims.Height(), ims.Width());
   ecran.etc[0] = '\0';
   
   // Normalize the source data.
   ecran.typeval=tfloat;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   ecran.minval=ecran.maxval=ims[0][0];
   
   // Find ecran.maxval and ecran.minval.
   Float *d=ecran.imsource->VectorX();
   Float *s=ims.Vector();
   for(;s<ims.Vector()+ims.Width()*ims.Height(); d++){
      if ((*(d)=*(s++)) > ecran.maxval)
	 ecran.maxval = *d;
      if (*d < ecran.minval)
	 ecran.minval = *d;
   }

   if (isnan(ecran.minval) || isnan(ecran.maxval)) {
      std::cout << "Error: Invalid image data: NaN pixel value detected" << std::endl;
      exit(1);
   }
   ims.Delete();
   
   // Determine maximum precision between 2 consecutives display floatting point values.
   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP);
   if (ecran.stepval == 0) ecran.stepval=1;

   // Normalize the logic image (by default contrast stretching). 
//   Egalisation(d,*ecran.imlogic,MAXCOLS);
   Recadrage(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   tracesdessin=imd.Vector();
   imd=0;

#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}


/**
 * Display double 3d gray image.
 */
Errc Visu(Img3dsf& ims, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Img3dsf";
   ecran.content = "Content: Image, Gray level, 3D";
   ecran.pixel = "Pixel: float";
   sprintf(ecran.size,"Size: %ld planes x %ld rows x %ld ncols",ims.Depth(),ims.Height(), ims.Width());
   ecran.etc[0] = '\0';

   // Normalize the source data.
   ecran.typeval=tfloat;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Depth(),ims.Height(),ims.Width());
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   ecran.minval=ecran.maxval=ims[0][0][0];

   // Find ecran.maxval and ecran.minval.
   Float *d=ecran.imsource->VectorX();
   Float *s=ims.Vector();
   for(;s<ims.Vector()+ims.Depth()*ims.Width()*ims.Height(); d++){
      if ((*(d)=(Float)*(s++)) > ecran.maxval)
	 ecran.maxval = *d;
      if (*d < ecran.minval)
	 ecran.minval = *d;
   }

   if (isnan(ecran.minval) || isnan(ecran.maxval)) {
      std::cout << "Error: Invalid image data: NaN pixel value detected" << std::endl;
      exit(1);
   }
   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;

   ims.Delete();

   // Normalize the logic image (by default constrast stretching).
   Recadrage(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;
  
   tracesdessin=imd.Vector();
   imd=0;

#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display long 2d color image.
 */
Errc Visu(Imc2dsl& ims, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Imc2dsl";
   ecran.content = "Content: Image, Color, 2D";
   ecran.pixel = "Pixel : integer 32-bits";
   sprintf(ecran.size,"Size : %ld rows x %ld ncols",ims.Height(), ims.Width());
   sprintf(ecran.etc, "Space: %s",ColorName(ims.ColorSpace()));

   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT4;
   ecran.typeval = tlong;
   ecran.colored=true;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   Long *sx=ims.VectorX();  Long *sy=ims.VectorY();  Long *sz=ims.VectorZ();
   Float *dx=ecran.imsource->VectorX();  Float *dy=ecran.imsource->VectorY();  Float *dz=ecran.imsource->VectorZ();
   for(;sx<ims.VectorX()+ims.Width()*ims.Height();){
      *(dx++)=(Float)*(sx++); *(dy++)=(Float)*(sy++); *(dz++)=(Float)*(sz++);
   }

   // Build the first normalized image.
   Imc3duc *imi=new Imc3duc(1,ims.Height(), ims.Width());
   sx=ims.VectorX();  sy=ims.VectorY();  sz=ims.VectorZ();
   Uchar *ix=imi->VectorX();  Uchar *iy=imi->VectorY(); Uchar *iz=imi->VectorZ();
   for(;sx<ims.VectorX()+ims.Width()*ims.Height();){
      *(ix++)=*(sx++); *(iy++)=*(sy++); *(iz++)=*(sz++);
   }

   ims.Delete();  
   // Reduce the number of colors (24 -> 8 bits).
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   Convertie24en8(*imi,*ecran.imlogic);
   delete imi;
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

/**
 * Display float 2d color image.
 * Transfert de l'image par un vecteur
 * de nlig*Width() couleurs en 256 niveaux de gris.
 * Il faut donc ramener le nombre de couleurs a 255.
 * La methode consiste a normaliser l'image pour ramener les valeurs
 * entre 0 et 256.
 */
Errc Visu(Imc2dsf& ims, Img2duc& imd) {
   // Image properties.
   ecran.type = "Type: Imc2dsf";
   ecran.content = "Content: Image, Color, 2D";
   ecran.pixel = "Pixel : float";
   sprintf(ecran.size,"Size : %ld rows x %ld ncols",ims.Height(), ims.Width());
   sprintf(ecran.etc, "Space: %s",ColorName(ims.ColorSpace()));

   // Normalize the source data.
   ecran.typeval = tfloat;
   ecran.colored=true;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(1,ims.Height(),ims.Width());
   Float *sx=ims.VectorX();  Float *sy=ims.VectorY();  Float *sz=ims.VectorZ();
   Float *dx=ecran.imsource->VectorX();  Float *dy=ecran.imsource->VectorY();  Float *dz=ecran.imsource->VectorZ();
   ecran.minval=ecran.maxval=ims.X[0][0];

   // Seeking ecran.maxval and ecran.minval.
   for(;sx<ims.VectorX()+ims.Width()*ims.Height(); dx++){
      if ((*(dx)=(Float)*(sx++)) > ecran.maxval)
	 ecran.maxval = *dx;
      if (*dx < ecran.minval)
	 ecran.minval = *dx;
   }
   for(;sy<ims.VectorY()+ims.Width()*ims.Height(); dy++){
      if ((*(dy)=(Float)*(sy++)) > ecran.maxval)
	 ecran.maxval = *dy;
      if (*dy < ecran.minval)
	 ecran.minval = *dy;
   }
   for(;sz<ims.VectorZ()+ims.Width()*ims.Height(); dz++){
      if ((*(dz)=(Float)*(sz++)) > ecran.maxval)
	 ecran.maxval = *dz;
      if (*dz < ecran.minval)
	 ecran.minval = *dz;
   }

   if (isnan(ecran.minval) || isnan(ecran.maxval)) {
      std::cout << "Error: Invalid image data: NaN pixel value detected" << std::endl;
      exit(1);
   }
   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;
   
   for(;sx<ims.VectorX()+ims.Width()*ims.Height();){
      *(dx++)=(Float)*(sx++); *(dy++)=(Float)*(sy++); *(dz++)=(Float)*(sz++);
   }
   
   ims.Delete();
   // Build the first normalized image.
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   Imc3duc *imi=new Imc3duc(ecran.imsource->Size());

   dx=ecran.imsource->VectorX();
//    Egalisation(dx,*ecran.imlogic,MAXUINT1);
   Recadrage(dx,*ecran.imlogic,MAXUINT1);
   Uchar *l=ecran.imlogic->Vector();
   Uchar *ix=imi->VectorX();
   for(;ix<imi->VectorX()+imi->Width()*imi->Height();){
      *(ix++)=*(l++);
   }
   dy=ecran.imsource->VectorY();
//    Egalisation(dy,*ecran.imlogic,MAXUINT1);
   Recadrage(dy,*ecran.imlogic,MAXUINT1);
   l=ecran.imlogic->Vector();
   Uchar *iy=imi->VectorY();
   for(;iy<imi->VectorY()+imi->Width()*imi->Height();){
      *(iy++)=*(l++);
   }
   dz=ecran.imsource->VectorZ();
//    Egalisation(dz,*ecran.imlogic,MAXUINT1);
   Recadrage(dz,*ecran.imlogic,MAXUINT1);
   l=ecran.imlogic->Vector();
   Uchar *iz=imi->VectorZ();
   for(;iy<imi->VectorZ()+imi->Width()*imi->Height();){
      *(iz++)=*(l++);
   }
  
   // Reduce the number of colors (24 -> 8 bits).
   Convertie24en8(*imi,*ecran.imlogic);
   delete imi;
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

// les images multispectrales -> visualisees comme des images 3d.
/**
 * Normalization: Convert range value [0-225] to [0-MAXCOLS[
 * Original colormap : gray levels.
 */
Errc Visu(Imx2duc& ims, Img3duc& imd) {
   // Image properties.
   ecran.type    = "Type: Imx2duc";
   ecran.content = "Content: Image, Multi-Spectral, 2D";
   ecran.pixel   = "Pixel: integer 8-bits";
   sprintf(ecran.size,"Bands: %ld bands, Size:  %ld rows x %ld ncols",ims.Bands(),ims.Height(), ims.Width());
   ecran.etc[0]  = '\0';
   
   // Normalize the source data.
   ecran.minval=0.0;
   ecran.maxval=MAXUINT1;
   ecran.typeval=tchar;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Bands(),ims.Height(),ims.Width());
   ecran.imlogic=new Img3duc(ims.Bands(),ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX(); 
   Uchar *l=ecran.imlogic->Vector();
   for (int b=0; b<ims.Bands();b++){
      Uchar *s=ims.Vector(b);
      for(;s<ims.Vector(b)+ims.Width()*ims.Height();){
	 *(l++)=*s;
	 *(d++)=(Float)*(s++);
      }
   }

   ims.Delete();   
   // Normalize the logic image.
   
   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;

   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imsource;
   delete ecran.imlogic;

   return SUCCESS;
}

/**
 * Display long 3d multispectral image.
 * Build two images :
 * 1- source image with true values [ecran.minval .. ecran.maxval] (type: Img3dsf)
 * 2- ecran.imlogic image with drawable values [0..MAXCOLS] (type : Img3duc)
 */
Errc Visu(Imx2dsl& ims, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Imx2dsl";
   ecran.content = "Content: Image, Multi-Spectral, 3D";
   ecran.pixel = "Pixel: integer 32-bits";
   sprintf(ecran.size,"Bands: %ld bands, Size: %ld rows x %ld ncols",ims.Bands(),ims.Height(), ims.Width());
   ecran.etc[0] = '\0';
   // Normalize the source data.
   ecran.typeval=tlong;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Bands(),ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX();
   for (int b=0; b<ims.Bands(); b++){
      Long *s=ims.Vector(b);
      for(;s<ims.Vector(b)+ims.Width()*ims.Height(); s++, d++){
	 if ((*(d)=(Float)*(s)) > ecran.maxval)
	    ecran.maxval = *d;
	 if (*d < ecran.minval)
	    ecran.minval = *d;
      }
   }
   
   // Normalize the logic image.
   if (ecran.minval >= 0 && ecran.maxval <= MAXUINT1){ // case UINT1 -> /2.
      ecran.minval=0;
      ecran.imlogic=new Img3duc(ims.Bands(),ims.Height(),ims.Width());
      Uchar *l=ecran.imlogic->Vector();
      for (int b=0; b< ims.Bands(); b++){
	 Long *s=ims.Vector(b);
	 for(;s<ims.Vector(b)+ims.Width()*ims.Height();*(l++)=*(s++));
      }
   }else  if (ecran.maxval-ecran.minval < MAXCOLS){ // case UINT1 -> *1
      ecran.minval = ecran.maxval-MAXCOLS;
      ecran.imlogic=new Img3duc(ims.Bands(),ims.Height(),ims.Width());
      Uchar *l=ecran.imlogic->Vector();
      for (int b=0; b<ims.Bands(); b++){
	 Long *s=ims.Vector(b);
	 for(;s<ims.Vector(b)+ims.Width()*ims.Height();*(l++)=*(s++));
      }
   }else{ // Case MAXUINT4
      ecran.imlogic=new Img3duc(ims.Bands(),ims.Height(),ims.Width());
      d=ecran.imsource->VectorX();
      ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
      if (ecran.stepval == 0) ecran.stepval=1;
//       Egalisation(d,*ecran.imlogic,MAXCOLS);
      Recadrage(d,*ecran.imlogic,MAXCOLS);
   }

   ims.Delete();
   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}


/**
 * Display double 3d gray image.
 */
Errc Visu(Imx2dsf& ims, Img3duc& imd) {
   // Image properties.
   ecran.type = "Type: Imx2dsf";
   ecran.content = "Content: Image, Multi-Spectral, 2D";
   ecran.pixel = "Pixel: float";
   sprintf(ecran.size,"Bands: bands %ld, Size: %ld rows x %ld ncols",ims.Bands(),ims.Height(), ims.Width());
   ecran.etc[0] = '\0';
   // Normalize the source data.
   ecran.typeval=tfloat;
   ecran.colored=false;
   ecran.dynamisable=true;

   ecran.imsource=new Imc3dsf(ims.Bands(),ims.Height(),ims.Width());
   Float *d=ecran.imsource->VectorX();
   
   for (int b=0; b<ims.Bands(); b++){
      Float *s=ims.Vector(b);
      ecran.minval=ecran.maxval=ims[0][0][0];
      // Seeking ecran.maxval and ecran.minval.
      for(;s<ims.Vector(b)+ims.Width()*ims.Height(); d++){
	 if ((*(d)=(Float)*(s++)) > ecran.maxval)
	    ecran.maxval = *d;
	 if (*d < ecran.minval)
	    ecran.minval = *d;
      }
   }
   if (isnan(ecran.minval) || isnan(ecran.maxval)) {
      std::cout << "Error: Invalid image data: NaN pixel value detected" << std::endl;
      exit(1);
   }
   ecran.stepval = fabs((ecran.maxval-ecran.minval)/MAXNSTEP); // Donne la precision...
   if (ecran.stepval == 0) ecran.stepval=1;

   ims.Delete();
   // Normalize the logic image.
   ecran.imlogic=new Img3duc(ecran.imsource->Size());
   d=ecran.imsource->VectorX();
   Egalisation(d,*ecran.imlogic,MAXCOLS);
   Recadrage(d,*ecran.imlogic,MAXCOLS);

   // Build the levels colormap (>>;<< means 2 consecutive pixels with the same color).
   for (int i=0; i<MAXCOLS; i++)
      ecran.rmap[i]=ecran.gmap[i]=ecran.bmap[i]=(i>>coloffset)<<coloffset;
  
   tracesdessin=imd.Vector();
   imd=0;
#ifdef __DRAWINGMODE
   AfficheX11(1);
#else
   AfficheX11(0);
#endif
   delete ecran.imlogic;
   delete ecran.imsource;

   return SUCCESS;
}

#ifdef MAIN

/*
 * Modify only the following constants, and the function calls.
 */
#define	USAGE	"usage: %s [-m mask] [im_in|-]"
#define	PARC	0
#define	FINC	1
#ifdef __DRAWINGMODE
#define	FOUTC	1
#else
#define	FOUTC	0
#endif
#define	MASK	0

int main(int argc,char* argv[]) {
  Errc result;		     // The result code of the execution.
  Pobject *mask;	     // The region map..
  Pobject *objin[FINC+1];    // The input objects.
  Pobject *objs[FINC+1];     // The source objects masked by the carte.
  Pobject *objout[FOUTC+1];  // The output object.
  Pobject *objd[FOUTC+1];    // The result object of the execution.
  char* parv[PARC+1];	     // Parametres d'entree.
  Char *nomentree;
  Long pid;
  int depth;
  
  ReadArgs(argc,argv,PARC,FINC,FOUTC,&mask,objin,objs,objout,objd,parv,USAGE,MASK);

  // Consider only the SON process.
  if ((pid=fork()) != 0) Exit(pid);
  else if (pid == -1) Exit(FAILURE);
  
  // Get the image name.
  int rank=(mask)? 2 : 1;
  if (argc>rank) nomentree=argv[rank];
  else nomentree="stdin";
  depth = InitXsession(nomentree,NULL,argc,argv);

  // 24 bits -> trueColor.
  if (depth >= 24) coloffset = 0; else coloffset = 1;
  
  switch(objs[0]->Type()){
  case Po_Img2duc :{
     Img2duc* const ims=(Img2duc*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];     
     result = Visu(*ims,*imd);
  }break;
  case Po_Img2dsl :{
     Img2dsl* const ims=(Img2dsl*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Img2dsf :{
     Img2dsf* const ims=(Img2dsf*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Reg2d :{
     Reg2d* const rgs=(Reg2d*)objs[0];
     objd[0]=new Img2duc(rgs->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*rgs,*imd);
  }break;
  case Po_Reg3d :{
     Reg3d* const rgs=(Reg3d*)objs[0];
     objd[0]=new Img3duc(rgs->Size());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*rgs,*imd);
  }break;
  case Po_Graph2d :{
     Graph2d* const grs=(Graph2d*)objs[0];
     objd[0]=new Img2duc(grs->Height(),grs->Width());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*grs,*imd);
  }break;
  case Po_Graph3d :{
     Graph3d* const grs=(Graph3d*)objs[0];
     objd[0]=new Img3duc(grs->Depth(),grs->Height(),grs->Width());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*grs,*imd);
  }break;
  case Po_Imc2duc :{
     Imc2duc* const ims=(Imc2duc*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imc2dsl :{
     Imc2dsl* const ims=(Imc2dsl*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imc2dsf :{
     Imc2dsf* const ims=(Imc2dsf*)objs[0];
     objd[0]=new Img2duc(ims->Size());
     Img2duc* const imd=(Img2duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Img3duc :{
     Img3duc* const ims=(Img3duc*)objs[0];
     objd[0]=new Img3duc(ims->Size());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Img3dsl :{
     Img3dsl* const ims=(Img3dsl*)objs[0];
     objd[0]=new Img3duc(ims->Size());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Img3dsf :{
     Img3dsf* const ims=(Img3dsf*)objs[0];
     objd[0]=new Img3duc(ims->Size());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imc3duc :{
     Imc3duc* const ims=(Imc3duc*)objs[0];
     objd[0]=new Img3duc(ims->Size());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imx2duc :{
     Imx2duc* const ims=(Imx2duc*)objs[0];
     objd[0]=new Img3duc(ims->Bands(),ims->Height(),ims->Width());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imx2dsl :{
     Imx2dsl* const ims=(Imx2dsl*)objs[0];
     objd[0]=new Img3duc(ims->Bands(),ims->Height(),ims->Width());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  case Po_Imx2dsf :{
     Imx2dsf* const ims=(Imx2dsf*)objs[0];
     objd[0]=new Img3duc(ims->Bands(),ims->Height(),ims->Width());
     Img3duc* const imd=(Img3duc*)objd[0];
     result = Visu(*ims,*imd);
  }break;
  default :
     PrintErrorFormat(objin,FINC);
     result = FAILURE;
  }

  WriteArgs(argc,argv,PARC,FINC,FOUTC,&mask,objin,objs,objout,objd);
  exit (result!= SUCCESS);
}

#endif
