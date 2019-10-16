/* -*- c-basic-offset: 3 -*-
 *
 * PANTHEON Project
 *
 * GREYC IMAGE
 * 6 Boulevard MarÅÈchal Juin
 * F-14050 Caen Cedex France
 *
 * This file is free software. You can use it, distribute it
 * and/or modify it. However, the entire risk to the quality
 * and performance of this program is with you.
 *
 * For more information, refer to:
 * http://www.greyc.ensicaen.fr/EquipeImage/Pandore/
 */

/*
 * @author RÅÈgis Clouard - 1999-11-10
 * @author RÅÈgis Clouard - 2001-04-27 (version 3.00)
 * @author RÅÈgis Clouard - 2002-12-11 (version 4.00)
 * @author RÅÈgis Clouard - 2002-12-11 (version 4.00)
 * @author RÅÈgis Clouard - 2005-03-18 (add the dynamic menu)
 * @author RÅÈgis Clouard - 2005-10-17 (change colormap)
 */

/**
 * @file visuoutil.cpp
 * @brief Visualization of Pandore image within a Motif environment.
 */

#include <Xm/ScrollBar.h>
#include <Xm/Display.h>
#include <pandore.h>
#include <math.h>
using namespace pandore;

/* RANGE forces a to be in the range b..c (inclusive) */
#define ECRETESUP(a,c)  { if (a > c) a = c; }
#define ECRETE(a,b,c) { if (a < b) a = b;  if (a > c) a = c;}

/* Global variables */
static Display *dpy;
static GC gc;			// Graphic context
static GC default_gc;

static Visual *visual;		// Best visual
static int depth;
static Visual *default_visual;	// Default visual
static int default_depth;
static Colormap ollut;
static Colormap default_ollut;

Uchar *donneesdessin=NULL;	// Real drawing data (2d)
Uchar *tracesdessin=NULL;	// Record drawing data (same size and dimensions than the input image).

int currentslice;		// Current plan number.

XColor imlut[MAXCOLS+1];	// Current Colormap. imlut[MAXCOLS] = pen color.

struct PColor{
   Uchar red;
   Uchar green;
   Uchar blue;
};

PColor srclut[MAXCOLS+1];		// the current colormap.

XImage *imcarte=NULL;

int selrectdim;			// Size of the selection rectangle.
static int vx,vy;		// Coordinates of the selection rectangle.

Uchar *imphysic=NULL;		// Physical image (What is really displayed)

Float imseuilb, imseuilh; 	// Threshold boundaries.
int normseuil;			// for float values: the normalisation value.

float imzoom=1;			// Zoom value
Uchar sns_seuil=0;

bool fin=false;

int bytes_per_pixel=0;

static Pixel stylo;
static Pixel black_color;
static Pixel white_color;
static Pixel bgcolor;
Pixel default_bgcolor;
Pixel default_fgcolor;

static int _contrast=0;
static int _brightness=0;

/*======================================================================
 *			XIMAGE
 * Manage X11 image in a Motif window.
 * (logic+tracesdessin) -> imphysic
 *======================================================================*/

/**
 * Returns the corresponding bit shift number of mask `ul'.
 */
Ulong bitShiftNumber(unsigned long ul) {
   int i;
   for (i=31; ((ul&0x80000000) == 0) && i>=0;  i--, ul<<=1);
   return i;
}

/**
 * Use the imlut to encode the physical image (the one to be displayed)
 * Encoding depends on the screen depth value (8, 16, 24 or 32 bits).
 */
Uchar* EncodeImage() {
   register Uchar *pl,*pp;
   register Float *ps;
   register Uchar *pd;
   int val;
   Uchar *imvis=NULL;

   int stepin;
   int stepout;
   
   if (imzoom >= 1){
      stepin= 1;
      stepout=(int)imzoom;
   }else{
      stepin=(int)rint(1.0/imzoom);
      stepout=1;
   }

   // Threshold the logic image to build the physic image.
   pl= ecran.imlogic->Vector()+(currentslice*ecran.imsource->Width()*ecran.imsource->Height());
   ps= ecran.imsource->VectorX()+(currentslice*ecran.imsource->Width()*ecran.imsource->Height());
   pd= tracesdessin;
   
   // Build imphysic.
   switch(depth){
   case 8: {
      pp= imvis =(Uchar*)malloc(int(ecran.imsource->Width()*ecran.imsource->Height()*imzoom*imzoom*sizeof(Uchar)*bytes_per_pixel));
      
      for (int i=0; i<ecran.imsource->Height()/stepin; i++,ps+=(stepin*ecran.imsource->Width()),pd+=(stepin*ecran.imsource->Width()), pl+=(stepin*ecran.imsource->Width())){
	 for (int l=0; l<stepout; l++)
	    for (int j=0; j<ecran.imsource->Width()/stepin; j++){
	       int dep=j*stepin;
	       if ((*(ps+dep) < imseuilb) || (*(ps+dep) > imseuilh))
		  val=(*(pd+dep))?MAXCOLS:0; // MAXCOLS for drawn lines.
	       else
		  val=(*(pd+dep))?MAXCOLS:*(pl+dep);
	       
	       for(int k=0; k<stepout; k++)
		  *(pp++)=(Uchar)imlut[val].pixel;
	    }
      }
   }break;
   case 16:{
      int rshift = bitShiftNumber(visual->red_mask)   - 4;
      int gshift = bitShiftNumber(visual->green_mask) - 4;
      int bshift = bitShiftNumber(visual->blue_mask)  - 4;
      
      pp= imvis =(Uchar*)malloc(int(ecran.imsource->Width()*ecran.imsource->Height()*imzoom*imzoom*sizeof(Uchar)*bytes_per_pixel));
      unsigned short temp;
      for(int i=0; i<ecran.imsource->Height()/stepin; i++,ps+=(stepin*ecran.imsource->Width()),pd+=(stepin*ecran.imsource->Width()), pl+=(stepin*ecran.imsource->Width())){
	 for(int l=0; l<stepout; l++)
	    for(int j=0; j<ecran.imsource->Width()/stepin; j++){
	       int dep=j*stepin;
	       if ((*(ps+dep) < imseuilb) || (*(ps+dep) > imseuilh))
		  val=(*(pd+dep))?MAXCOLS:0;
	       else
		  val=(*(pd+dep))?MAXCOLS:*(pl+dep);
	       temp =
		  (((imlut[val].red   >> 11) & 0xff) << rshift) |
		  (((imlut[val].green >> 11) & 0xff) << gshift) |
		  (((imlut[val].blue  >> 11) & 0xff) << bshift);
	       
	       for(int k=0; k<stepout; k++){
		  if (BitmapBitOrder(dpy) == MSBFirst){
			*pp++ = (temp >> 8) & 0xff;
			*pp++ = temp & 0xff;
		  }else{
		     *pp++ = temp & 0xff;
		     *pp++ = (temp >> 8) & 0xff;
		  }
	       }
	    }
      }
   }break;
   case 24:
   case 32:{
      pp= imvis =(Uchar*)malloc(int(ecran.imsource->Width()*ecran.imsource->Height()*imzoom*imzoom*sizeof(Uchar)*bytes_per_pixel));

      unsigned long temp;

      int rshift = bitShiftNumber(visual->red_mask) - 7;
      int gshift = bitShiftNumber(visual->green_mask) - 7;
      int bshift = bitShiftNumber(visual->blue_mask) - 7;

      for (int i=0; i<ecran.imsource->Height()/stepin; i++,ps+=(stepin*ecran.imsource->Width()),pd+=(stepin*ecran.imsource->Width()), pl+=(stepin*ecran.imsource->Width())){
	 for (int l=0; l<stepout; l++)
	    for (int j=0; j<ecran.imsource->Width()/stepin; j++){
	       int dep=j*stepin;
	       if ((*(ps+dep) < imseuilb) || (*(ps+dep) > imseuilh))
		  val=(*(pd+dep))?MAXCOLS:0;
	       else
		  val=(*(pd+dep))?MAXCOLS:*(pl+dep);
	       
	       temp =
		  (((imlut[val].red   >> 8) & 0xff) << rshift) |
		  (((imlut[val].green >> 8) & 0xff) << gshift) |
		  (((imlut[val].blue  >> 8) & 0xff) << bshift);

	       for (int k=0; k<stepout; k++){
		  if (BitmapBitOrder(dpy) == MSBFirst){
		     for (int b=bytes_per_pixel-1;b>=0;b--)
			*pp++ = (unsigned char)((temp >> (b*8)) & 0xff);
		  }else{
		     for (int b=0;b<bytes_per_pixel;b++)
			*pp++ = (unsigned char)((temp >> (b*8)) & 0xff);
		  }
	       }
	    }
      }
   }break;
   }
   return imvis;
}

/**
 * Create an X11 image from imphysic.
 */
void CreeImage(Widget frame) {
   imphysic= EncodeImage();
   imcarte= XCreateImage(dpy,visual,depth,ZPixmap,0,(char*)imphysic,ecran.imsource->Width(),ecran.imsource->Height(),BitmapPad(dpy), ecran.imsource->Width()*bytes_per_pixel);
}

/**
 * Erase the "rectangle value" at cordinate (a,b).
 */
void effaceRectangle(Window xwin, int a, int b, int x, int y) {
   // Old rectangle.
   XPutImage(dpy,xwin,gc,imcarte,int(a*imzoom),0,int(a*imzoom),0,1,int(ecran.imsource->Height()*imzoom));
   XPutImage(dpy,xwin,gc,imcarte,0,int(b*imzoom),0,int(b*imzoom),int(ecran.imsource->Width()*imzoom),1);
   XPutImage(dpy,xwin,gc,imcarte,int(a*imzoom),int(b*imzoom),int(a*imzoom),int(b*imzoom),int(selrectdim*imzoom+1),int(selrectdim*imzoom+1));
}

/**
 * Draw the "rectangle value" at coordinate (x,y).
 * First of all, erase old rectangle coordinate a,b.
 */
void afficheRectangle(Window xwin, int a, int b, int x, int y) {
   effaceRectangle(xwin,a,b,x,y);
   // New rectangle.
   XDrawLine(dpy,xwin,gc,int(x*imzoom),0,int(x*imzoom),int(ecran.imsource->Height()*imzoom));
   XDrawLine(dpy,xwin,gc,0,int(y*imzoom),int(ecran.imsource->Width()*imzoom),int(y*imzoom));
   XDrawRectangle(dpy,xwin,gc,int(x*imzoom),int(y*imzoom),int(selrectdim*imzoom),int(selrectdim*imzoom));
}

/**
 * Draw the current X11 image.
 */
void afficheImage(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmDrawingAreaCallbackStruct *cbs=(XmDrawingAreaCallbackStruct *)call_data;
   
   if (XtIsRealized(widget)){
      XPutImage(dpy,XtWindow(widget),gc,imcarte,cbs->event->xexpose.x,cbs->event->xexpose.y,
		cbs->event->xexpose.x,cbs->event->xexpose.y,
		cbs->event->xexpose.width,cbs->event->xexpose.height);
      afficheRectangle(XtWindow(widget),0,0,vx,vy);
      XFlush(dpy);
   }
}

/**
 * Redisplay image from encoded image.
 * Call when zoom or threshol changed.
 */
void changeImage() {
   imcarte->width = int(ecran.imsource->Width()*imzoom);
   imcarte->height = int(ecran.imsource->Height()*imzoom);
   free(imcarte->data);	// Free old X11 data.
   imphysic = EncodeImage();
   imcarte->data = (Char*)imphysic; // Set new X11 data.
   imcarte->bytes_per_line = imcarte->width*bytes_per_pixel;
   XClearArea(dpy,XtWindow(tableau),0,0,0,0,True);
}

#ifndef __DRAWINGMODE

/**
 * Manage events in the drawing area for visu mode.
 * Right button -> menu.
 * Left buttonchange -> select new coordinates of the value rectangle.
 * Arrows -> move the coordinate of the value rectangle.
 */
void menuTableauVisu(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*)call_data;
   XEvent *event = cbs->event;
   Window xwin = XtWindow(widget);
   XComposeStatus compose;
   KeySym keysym;
   Char buffer[2];
   int buffersize = 2;
   int nx=vx,ny=vy;
   void changeValeurs();
   void changeProfil();
   void sortie(Widget widget, XtPointer, XtPointer);

   if (cbs->reason == XmCR_INPUT){
      switch(event->xany.type){

      case KeyPress :
	 XLookupString((XKeyEvent*)event,buffer,buffersize,&keysym,&compose);
	 switch(keysym){
	 case XK_Left :
	    if (vx>0) --vx;
	    break;
	 case XK_Up :
	    if (vy>0) --vy;
	    break;
	 case XK_Right :
	    if (vx<(ecran.imsource->Width()-selrectdim)) ++vx;
	    break;
	 case XK_Down :
	    if (vy<(ecran.imsource->Height()-selrectdim)) ++vy;
	    break;
	 case 'r':
	 case 'R':
	    menuProfil(widget, (XtPointer)0, NULL);
	    break;
	 case 'c':
	 case 'C':
	    menuProfil(widget, (XtPointer)1, NULL);
	    break;
	 case 'l':
	 case 'L':
	    menuLut(widget, NULL, NULL);
	    break;
	 case 'd':
	 case 'D':
	    menuDynamic(widget, NULL, NULL);
	    break;
	 case 'v':
	 case 'V':
	    menuValeurs(widget, NULL, NULL);
	    break;
	 case 'p':
	 case 'P':
	    menuProp(cadre,NULL,NULL);
	    break;
	 case 'h':
	 case 'H':
	    menuHistogramme(cadre,NULL,NULL);
	    break;
	 case 't':
	 case 'T':
	    menuSeuillage(cadre,NULL,NULL);
	    break;
	 case '-': {
	    int newzoom;
	    if (imzoom >= 1) newzoom=(int)imzoom-1;
	    else newzoom = -(int)(1/imzoom)-1;
	    if (newzoom == 0) newzoom =-2;
	    else if (newzoom < -4) newzoom =-4;
	    menuZoom(cadre,(XtPointer)newzoom,NULL);
	 }
	    break;
	 case '+': {
	    int newzoom;
	    if (imzoom >= 1) newzoom=(int)imzoom+1;
	    else newzoom = -(int)(1/imzoom)+1;
	    if (newzoom == -1) newzoom =1;
	    else if (newzoom > 4) newzoom =4;
	    menuZoom(cadre,(XtPointer)newzoom,NULL);
	 }
	    break;
	 case 'q':
	 case 'Q':
	    sortie(cadre,NULL,NULL);
	    break;
	 default : return;
	 }
      
	 afficheRectangle(xwin,nx,ny,vx,vy);
	 changeValeurs();
	 changeProfil();
	 break;

      case ButtonPress :
	 if (event->xbutton.button==Button1){
	    vx = int(event->xbutton.x/imzoom);
	    vy = int(event->xbutton.y/imzoom);
	    if ((vx+selrectdim<=ecran.imsource->Width()) && (vy+selrectdim<=ecran.imsource->Height())){
	       afficheRectangle(xwin,nx,ny,vx,vy);
	       changeValeurs();
	       changeProfil();
	    }else{
	       vx = nx; vy = ny;
	    }
	 }else if (event->xbutton.button==Button3){
  	    XmMenuPosition(menu, (XButtonPressedEvent*)cbs->event);
	    XtManageChild(menu);
	 }
	 break;
      default  : return;
      }
   }
}
#endif

#ifdef __DRAWINGMODE

/*======================================================================*/
/*				XDRAW					*/
/* Draw lines in the drawing area.					*/
/*======================================================================*/

/**
 * Trace une droite de couleurs entre 2 points.
 */
static void TraceDroite(Uchar* data, int x1, int y1, int x2, int y2, Ulong  ncol, Uchar coul) {
   int i,j,max,min;
   float pente;
  
   if (x1!=x2)
      pente=(float)(y1-y2)/(x1-x2);
   else
      pente=MAXFLOAT;
  
   if ((pente<-1) || (pente>1)){
      if (y2<y1){
	 min=y2; max=y1;
      }else{
	 min=y1; max=y2;
      }
      for(i=max;i>=min;i--){
	 j=(int)((i-y2)/pente +x2);
	 data[i*ncol+j]=coul;
      }
   }else{
      if (x2<x1){
	 min=x2; max=x1;
      }else{
	 min=x1; max=x2;
      }
      for(j=max;j>=min;j--){
	 i=(int)(pente*(j-x2)+y2);
	 data[i*ncol+j]=coul;
      }
   }
}

/**
 * Manage MotionNotify events.
 */
void Trace(Widget widget, XtPointer client_data, XEvent* bevent, Boolean * unused) {
   Window  xwin = XtWindow(widget);
   XPointerMovedEvent* event=(XPointerMovedEvent*)bevent;
   int nx=vx, ny=vy;
   
   vx=int(event->x/imzoom);
   vy=int(event->y/imzoom);
  
   if ((vx<=ecran.imsource->Width()) && (vy<=ecran.imsource->Height())){
      XDrawLine(dpy,xwin,gc,int(nx*imzoom),int(ny*imzoom),int(vx*imzoom),int(vy*imzoom));
      TraceDroite(tracesdessin,nx,ny,vx,vy,ecran.imsource->Width(),255);
   }else{
      vx = nx; vy=ny;
   }
}

/**
 * Manage events in the drawing area for dessin mode.
 * Right button -> menu.
 * Left buttonchange -> draw lines.
 */
void menuTableauDessin(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*)call_data;
   XEvent *event = cbs->event;
   Window  xwin = XtWindow(widget);
   XComposeStatus  compose;
   KeySym  keysym;
   Char buffer[2]; 
   int buffersize = 2;
   int nx=vx,ny=vy;
   void changeValeurs();
   void changeProfil();
   void sortie(Widget widget, XtPointer, XtPointer);

   if (cbs->reason == XmCR_INPUT){  
      switch(event->xany.type){

      case KeyPress :
	 XLookupString((XKeyEvent*)event,buffer,buffersize,&keysym,&compose);
	 switch(keysym){
	 case XK_Left :
	    if (vx>0) --vx;
	    break;
	 case XK_Up :
	    if (vy>0) --vy;
	    break;
	 case XK_Right :
	    if (vx<(ecran.imsource->Width()-selrectdim)) ++vx;
	    break;
	 case XK_Down :
	    if (vy<(ecran.imsource->Height()-selrectdim)) ++vy;
	    break;
	 case 'r':
	 case 'R':
	    menuProfil(widget, (XtPointer)0, NULL);
	    break;
	 case 'c':
	 case 'C':
	    menuProfil(widget, (XtPointer)1, NULL);
	    break;
	 case 'l':
	 case 'L':
	    menuLut(widget, NULL, NULL);
	    break;
	 case 'd':
	 case 'D':
	    menuDynamic(widget, NULL, NULL);
	    break;
	 case 'v':
	 case 'V':
	    menuValeurs(widget, NULL, NULL);
	    break;
	 case 'p':
	 case 'P':
	    menuProp(cadre,NULL,NULL);
	    break;
	 case 'h':
	 case 'H':
	    menuHistogramme(cadre,NULL,NULL);
	    break;
	 case 't':
	 case 'T':
	    menuSeuillage(cadre,NULL,NULL);
	    break;
	 case '-': {
	    int newzoom;
	    if (imzoom >= 1) newzoom=(int)imzoom-1;
	    else newzoom = -(int)(1/imzoom)-1;
	    if (newzoom == 0) newzoom =-2;
	    else if (newzoom < -4) newzoom =-4;
	    menuZoom(cadre,(XtPointer)newzoom,NULL);
	 }
	    break;
	 case '+': {
	    int newzoom;
	    if (imzoom >= 1) newzoom=(int)imzoom+1;
	    else newzoom = -(int)(1/imzoom)+1;
	    if (newzoom == -1) newzoom =1;
	    else if (newzoom > 4) newzoom =4;
	    menuZoom(cadre,(XtPointer)newzoom,NULL);
	 }
	    break;
	 case 'q':
	 case 'Q':
	    sortie(cadre,NULL,NULL);
	    break;
	 default : return;
	 }
	 
	 afficheRectangle(xwin,nx,ny,vx,vy);
	 changeValeurs();
	 changeProfil();
	 break;

      case ButtonPress :
	 if (event->xbutton.button==Button1){
	    vx=int(event->xbutton.x/imzoom);
	    vy=int(event->xbutton.y/imzoom);
	    effaceRectangle(xwin,nx,ny,vx,vy);
	    if ((vx<=ecran.imsource->Width()) && (vy<=ecran.imsource->Height())){
	       if (donneesdessin)
		  {
		  memcpy(donneesdessin,tracesdessin,ecran.imsource->Width()*ecran.imsource->Height()); // Memorize previous work.
		  }
	       
	       Trace(widget, NULL, event, NULL);
	       changeValeurs();
	       changeProfil();
	       
	    }else{
	       vx = nx; vy = ny;
	    }
	    
	 }else  if (event->xbutton.button==Button3){
	    XmMenuPosition(menu, (XButtonPressedEvent*)cbs->event);
	    XtManageChild(menu);
	 }
 
	 break;
 
      case ButtonRelease :
	 if (vx>ecran.imsource->Width()-selrectdim) vx= ecran.imsource->Width()-selrectdim;
	 if (vy>ecran.imsource->Height()-selrectdim) vy= ecran.imsource->Height()-selrectdim;
	 changeImage();
	 break;
	 
      default : return;
      }
   }
}

#endif

/*======================================================================*/
/*				XVALEURS				*/
/* Display pixel values selected from the selection rectangle.		*/
/* use: imsource, currentslice, imseuilb, imseuilh.			*/
/*======================================================================*/

/**
 * Display "selrectdim" values for one color.
 */
void afficheValeur(Window xwin, char* format, int band, int xoffset, int yoffset) {
#define	LGBUF 11
#define HBOX 18
#define WBOX 130
   
   Char texte[42];
   int y, x;
   
   GC gc = XCreateGC(dpy,xwin, 0,NULL);

   for(y=0; y<selrectdim; y++){
      for(x=0; x<selrectdim; x++){
	 if ((ecran.imsource->Hold(currentslice,(y+vy),(x+vx)))){
	    Double valeur = (*ecran.imsource)[band][currentslice][y+vy][x+vx];
	    if ((valeur<imseuilb) || (valeur>imseuilh))
	       valeur=0;
	    sprintf(texte,format,valeur);
	 }else
	    sprintf(texte,"-");
	 Pixel pixel_color = XGetPixel(imcarte, x+vx, y+vy);
 	 XSetForeground(dpy,gc,pixel_color);
 	 XFillRectangle(dpy,xwin,gc,(x+xoffset)*(WBOX+2),(y+yoffset)*(HBOX+2),WBOX,HBOX);
	 XSetForeground(dpy,gc,black_color); // Shadow
	 XDrawString(dpy,xwin,gc,2+(x+xoffset)*(WBOX+2),(y+yoffset+1)*(HBOX+2)-4,texte,MIN(LGBUF,strlen(texte)));
	 XSetForeground(dpy,gc,white_color); 
	 XDrawString(dpy,xwin,gc,(x+xoffset)*(WBOX+2),(y+yoffset+1)*(HBOX+2)-4-2,texte,MIN(LGBUF,strlen(texte)));
	 XSetForeground(dpy,gc,stylo);
      }
   }
#undef LGBUF
#undef HBOX
#undef WBOX
}

/**
 * Triggered by redrawing events.
 * -> Redisplay values of the selection rectangle.
 */
void afficheValeurs(Widget widget, XtPointer client_data, XtPointer call_data) {
   Window xwin;
   Char texte[42];
   ::Dimension l,h;
   
   if (!XtIsRealized(widget)) // Not displayed.
      return;
   
   xwin = XtWindow(widget);
   // Title.
   sprintf(texte,"Values from (%03d,%03d): slice %03d",vx,vy,currentslice);
   XtVaSetValues(svaleurs,XmNtitle,texte,NULL);
   // Erase old content.
   XtVaGetValues(svaleurs,XmNwidth,&l,XmNheight,&h,NULL);
   XClearArea(dpy,xwin,0,0,l-1,h-1,0);
   
   // Display new content.
   switch(ecran.typeval){
   case tchar:
   case tlong:{
      afficheValeur(xwin,"% 11.0f",0,0,0);
      if (ecran.colored){ // 2 other colors.
	 afficheValeur(xwin,"% 11.0f",1,selrectdim, 0);
	 afficheValeur(xwin,"% 11.0f",2,0,selrectdim);
      }
   }break;
   case tfloat:{
      afficheValeur(xwin,"%# 11.3g",0,0,0);
      if (ecran.colored){ // 2 other colors.
	 afficheValeur(xwin,"%# 11.3g",1,selrectdim, 0);
	 afficheValeur(xwin,"%# 11.3g",2,0,selrectdim);
      }
    }break;
  }
}

/**
 * Change values.
 */
void changeValeurs() {
   afficheValeurs(fvaleurs,NULL,NULL);
}

/**
 * Triggered by the menu.
 */
void menuValeurs(Widget widget, XtPointer client_data, XtPointer call_data) {
   XtManageChild(XtParent(fvaleurs));
}

/**
 * Create values window.
 */
void CreeCadreValeurs(Widget parent)
{
   create_svaleurs (parent);
   vx=vy=0;
}

/*======================================================================*/
/*				XHISTOGRAM				*/
/* source -> vecthist.							*/
/* <- imseuilb, imseuilh, normseuil.					*/
/*======================================================================*/

#define OX 48
#define OY 120
#define HISTDIM MAXCOLS	// Number of displayed colors.

#define MAXNSTEP MAXUSHORT	// max size for histogram array.

static int hbeg, hend;
static int histodraw[HISTDIM];

struct shistogram {
   int vector[MAXNSTEP+1];
   int size;
} histogram;

/**
 * Actived by mouse drag event.
 * Display the histogram value at designed coordinate.
 */
void displayHistogramValue(Widget widget, XtPointer client_data, XEvent* bevent, Boolean * unused) {
   int hx,hy;
   Window  xwin = XtWindow(widget);
   XPointerMovedEvent* event=(XPointerMovedEvent*)bevent;
   char legend[50];

   hx = event->x - OX;
   hy = event->y;

   if (hx <0) hx = 0;
   if (hx > 255) hx = 255;

   float rhx1=((hx*(1+((hend-hbeg)/HISTDIM)))+hbeg)*ecran.stepval+ecran.minval;
   float rhx2=(((hx+1)*(1+((hend-hbeg)/HISTDIM)))+hbeg)*ecran.stepval+ecran.minval;
//    float rhx2=1-(((hx+1)*(1+((hend-hbeg)/HISTDIM)))+hbeg)*ecran.stepval+ecran.minval;

   if (ecran.typeval == tchar || ecran.typeval == tlong)
      if (rhx1 == rhx2)
	 sprintf(legend,"[%d] = %d ",(int)rhx1,histodraw[hx]);
      else
	 sprintf(legend,"[%d,%d] = %d ",(int)rhx1,(int)rhx2,histodraw[hx]);
   else
      sprintf(legend,"[%.2f,%.2f] = %d ",rhx1,rhx2,histodraw[hx]);
   XClearArea(dpy,xwin,100,0,30*8,16,0);
   XDrawString(dpy,xwin,default_gc,100,14,legend,strlen(legend));
}

/**
 * React to the mouse event from the histogram's window.
 * When mouse is down display the histogram value at designed coordinates.
 */
void Pointe(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmDrawingAreaCallbackStruct *cbs = (XmDrawingAreaCallbackStruct*)call_data;
   //   XEvent *event = cbs->event;
   
   if (cbs->event->xbutton.type == ButtonPress){
      displayHistogramValue(widget,client_data, cbs->event,NULL);
   }
   if (cbs->event->xbutton.type == ButtonRelease){
      XClearArea(dpy,XtWindow(widget),100,0,30*8,16,0);
   }
}

/**
 * Calculate histogram bars.
 * Normalize between [0..MAXUSHORT] with stepval step.
 */
void CalculeHistogramme() {
   histogram.size = (int)((ecran.maxval-ecran.minval)/ecran.stepval)+1;
   for(int * pv=histogram.vector; pv<histogram.vector+histogram.size; *(pv++)=0);
   Float *ps= &ecran.imsource->X[currentslice][0][0];
   for(int i=0; i<ecran.imsource->Height()*ecran.imsource->Width(); i++){
      histogram.vector[(int)((*(ps++) - ecran.minval)/ecran.stepval)]++;
   }
}

/**
 * Display current histogram (from [seuilb..seuilh]).
 */
void afficheHistogramme(Widget widget, XtPointer client_data, XtPointer call_data) {
   int hmax, imax, xmax;
   int step;
   int j,x,val;
   Window xwin;
   ::Dimension l,h;
   char legend[12];

   if (!XtIsRealized(widget))
      return;
   if (histogram.size==0)
      CalculeHistogramme();
   xwin=XtWindow(widget);
   // Clear window.
   XtVaGetValues(fhisto,XmNwidth,&l,XmNheight,&h,NULL);
   XClearArea(dpy,xwin,1,1,l-1,h-1,0);

   // Select dynamic limits.
   if ((int)((ecran.maxval-ecran.minval)/ecran.stepval) <= HISTDIM){ // Best case!
      hbeg = 0;  hend=(int)((ecran.maxval-ecran.minval)/ecran.stepval); step=1;
   }else if ((int)((imseuilh-ecran.minval)/ecran.stepval) <= HISTDIM){ 
      hbeg = 0;  hend=(int)((imseuilh-ecran.minval)/ecran.stepval); step=1;
   }else if ((int)((ecran.maxval-imseuilb)/ecran.stepval) <= HISTDIM){
      hbeg = (int)((imseuilb-ecran.minval)/ecran.stepval); 
      hend=(int)((ecran.maxval-ecran.minval)/ecran.stepval);step=1;
   }else if ((int)((imseuilh-imseuilb)/ecran.stepval) <= HISTDIM){
      hbeg = (int)((imseuilb-ecran.minval)/ecran.stepval); 
      hend=(int)((imseuilh-ecran.minval)/ecran.stepval);step=1;
   }else{ // worse case.
      hbeg=(int)((imseuilb-ecran.minval)/ecran.stepval);
      hend=(int)((imseuilh-ecran.minval)/ecran.stepval);
      step=1+(int)((hend-hbeg)/HISTDIM);
   }

   // Build the histogram to be displayed.
   x=0;hmax=0; xmax=0; imax=hbeg;
   for (int i=hbeg; i<=hend; i+=step){
      if ((i<(imseuilb-ecran.minval)/ecran.stepval)||(i>(imseuilh-ecran.minval)/ecran.stepval)){
	 val=0;
      }else{
	 val=0;
	 for (j=0;(j<step) && ((i+j)<=hend);j++){
	    if (histogram.vector[i+j]>val){
	       val = histogram.vector[i+j];
	       if (val > hmax){
		  hmax = val; // Use hmax for vertical normalization.
		  imax = i+j;
		  xmax = x;
	       }
	    }
	 }
      }
      histodraw[x++]=val;
   }
   if (x>HISTDIM)
      x--;
   // Display vertical graduation
   XDrawLine(dpy,xwin,default_gc,OX,OY,OX,OY-105);
   XDrawLine(dpy,xwin,default_gc,OX-2,OY,OX,OY);
   XDrawString(dpy,xwin,default_gc,16,OY+8,"0",1); // zero
   sprintf(legend,"%d",hmax); // max value
   XDrawLine(dpy,xwin,default_gc,OX-2,OY-100,OX,OY-100);
   XDrawString(dpy,xwin,default_gc,2,OY-92,legend,strlen(legend));

   // Display horizontal graduation.
   XDrawLine(dpy,xwin,default_gc,OX,OY,OX+260,OY);
   XDrawLine(dpy,xwin,default_gc,OX,OY+2,OX,OY);
   XDrawLine(dpy,xwin,default_gc,OX+(x/2),OY+2,OX+(x/2),OY);
   XDrawLine(dpy,xwin,default_gc,OX+x,OY+2,OX+x,OY);
   switch(ecran.typeval){
   case tchar:
   case tlong:
      sprintf(legend,"%d",(int)(hbeg*ecran.stepval+ecran.minval));
      XDrawString(dpy,xwin,default_gc,OX-12,OY+16,legend,strlen(legend));
      if (hend-hbeg>10){
	 sprintf(legend,"%d",(int)((((hend-hbeg)/2)*ecran.stepval)+ecran.minval));
	 XDrawString(dpy,xwin,default_gc,OX+(x/2)-12,OY+16,legend,strlen(legend));
      }
      sprintf(legend,"%d",(int)((imax*ecran.stepval)+ecran.minval));
      XDrawString(dpy,xwin,default_gc,OX+xmax-12,OY+32,legend,strlen(legend));
      sprintf(legend,"%5d",(int)((hend*ecran.stepval)+ecran.minval));
      XDrawString(dpy,xwin,default_gc,OX+x-28,OY+16,legend,strlen(legend));
      break;
   case tfloat:
      sprintf(legend,"%.2f",hbeg*ecran.stepval+ecran.minval);
      XDrawString(dpy,xwin,default_gc,OX-12,OY+16,legend,strlen(legend));
      if (hend-hbeg>10){
	 sprintf(legend,"%7.2f",((hend-hbeg)/2)*ecran.stepval+ecran.minval);
	 XDrawString(dpy,xwin,default_gc,OX+(x/2)-12,OY+16,legend,strlen(legend));
      }
      sprintf(legend,"%7.2f",imax*ecran.stepval+ecran.minval);
      XDrawString(dpy,xwin,default_gc,OX+xmax-12,OY+32,legend,strlen(legend));
      sprintf(legend,"%7.2f",hend*ecran.stepval+ecran.minval);
      XDrawString(dpy,xwin,default_gc,OX+x-32,OY+16,legend,strlen(legend));
      break;
   }

   // Draw the histogram.
   for (j=255; j>=x;histodraw[j--]=0);

   for (x--;x>=0; x--){
      val=(int)((float)(histodraw[x]*100.0)/(float)hmax);
      XDrawLine(dpy,xwin,default_gc,OX+x,OY,OX+x,OY-val);
   }
   XFlush(dpy);
#undef OX
#undef OY
#undef HISTDIM
}

/**
 * Redisplay the histogram...
 * when changing slice or thresholding limits.
 */
void changeHistogramme() {
   afficheHistogramme(fhisto,NULL,NULL);
}

/**
 * Triggered by `histogram' item of the menu.
 */
void menuHistogramme(Widget widget, XtPointer client_data, XtPointer call_data) {
   changeHistogramme();  
   XtManageChild(XtParent(fhisto));
}

/**
 * Initialization: Creation of the window and
 * calculation of the histogram for the current slice.
 */
void CreeCadreHistogramme(Widget parent) {
   create_shisto(parent,default_depth, default_visual,default_ollut);
   histogram.size=0;
   XtAddCallback(fhisto, XmNinputCallback, Pointe, (XtPointer) 0 );
   XtAddEventHandler(fhisto,Button1MotionMask,false,displayHistogramValue,NULL);
}

/*======================================================================*/
/*				XPROFIL					*/
/* Use source data [0..65535].						*/
/*======================================================================*/

#define ROW 0
#define COL 1
static int direction=0;	/* row =0, column =1 */

/**
 * Draw profil.
 */
void afficheProfil(Widget widget, XtPointer client_data, XtPointer call_data) {
   int x,y;
   Float niveau;
   ::Dimension l,h;
   Window xwin;
   Char texte[32];
  
   if (!XtIsRealized(widget))
      return;

   xwin = XtWindow(widget);
   XtVaGetValues(fprofil,XmNwidth,&l,XmNheight,&h,NULL);
   XClearArea(dpy,xwin,0,0,l,h,0);

   switch(direction){
   case ROW : // Row.
      sprintf(texte,"Profile for row %d",vy);
      XtVaSetValues(sprofil,XmNtitle,texte,NULL);
      for(x=0; x<ecran.imsource->Width(); x++){
	 if (((niveau=ecran.imsource->X[currentslice][vy][x])<imseuilb) || (niveau>imseuilh))
	    niveau=0;
	 else
	    niveau = ((niveau-imseuilb)*h) / (imseuilh-imseuilb);
	 XDrawLine(dpy,xwin,default_gc,5+x,h,5+x,h-(int)niveau);
      }
      break;
   case COL : // Column.
      sprintf(texte,"Profile for column %d",vx);
      XtVaSetValues(sprofil,XmNtitle,texte,NULL);
      for(y=0; y<ecran.imsource->Height(); y++){
	 if (((niveau=ecran.imsource->X[currentslice][y][vx])<=imseuilb) || (niveau>=imseuilh))
	    niveau=0;
	 else
	    niveau = ((niveau-imseuilb)*h) / (imseuilh-imseuilb);
	 XDrawLine(dpy,xwin,default_gc,5+y,h,5+y,h-(int)niveau);
      }
      break;
   }
   XFlush(dpy);
}

/**
 * Change profil.
 */
void changeProfil() {
   afficheProfil(fprofil,NULL,NULL);
}

/**
 * Triggered by profil item of the menu.
 */
void menuProfil(Widget widget, XtPointer client_data, XtPointer call_data) {
   direction = (int)client_data;
  
   if (XtIsRealized(fprofil))
      changeProfil();
   XtManageChild(XtParent(fprofil));
}

/**
 * Initialization. Set the same size as the drawing area.
 */
void CreeCadreProfil(Widget parent) {
   ::Dimension	l,h;
  
   create_sprofil (parent,default_depth, default_visual,default_ollut);
   l = 10+((ecran.imsource->Width()>ecran.imsource->Height())? ecran.imsource->Width():ecran.imsource->Height());
   h = 130;
   XtVaSetValues(fprofil,XmNwidth,l,XmNheight,h,NULL);
}

/*======================================================================*/
/*				XTHRESHOLD				*/
/* Thresolding.								*/
/* -> (seuilb,seuilh)							*/
/*======================================================================*/

/**
 * Adjust low threshold.
 */
void changeSeuilBas(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmScaleCallbackStruct* cbs = (XmScaleCallbackStruct*)call_data;
  
   imseuilb = (cbs->value)/(float)normseuil;   

   changeImage();
   changeHistogramme();
   changeValeurs();
   changeProfil();
}

/**
 * Ajust high threshold.
 */
void changeSeuilHaut(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmScaleCallbackStruct* cbs = (XmScaleCallbackStruct*)call_data;
  
   imseuilh = (float)cbs->value/normseuil;
   changeImage();
   changeHistogramme();
   changeValeurs();
   changeProfil();
}

/**
 * Triggered by threshold item of the menu.
 */
void menuSeuillage(Widget widget, XtPointer client_data, XtPointer call_data) {
   XtManageChild(fseuil);
}

/**
 * Initialization of threshold values.
 * Calculate imseuilb, imseuilh, imstep.
 */
void CreeCadreSeuil(Widget parent) {
   int decimal=0; // for float value: number of significant decimal.

   imseuilb=ecran.minval;
   imseuilh=ecran.maxval;

   // Determine the best visual step (from MAXNSTEP).
   switch(ecran.typeval){
   case tchar:
   case tlong:
      normseuil=1;
      break;
   case tfloat:
      if (ecran.stepval>1){
	 normseuil=1;
      }else{
	 decimal = -(int)log10(ecran.stepval);
	 normseuil = (int)pow(10,decimal);
      }
      break;
   }

   // When displayed `threshold panel'.
   if ((ABS(ecran.maxval*normseuil)> INT_MAX) ||  // Limit of the Motif scale.
       (ABS(ecran.minval*normseuil)> INT_MAX) ||
       (ABS((ecran.maxval-ecran.minval)*normseuil)> INT_MAX) || 
       (ecran.colored)){ // Colored image (need 3 threshold so none)
      XtSetSensitive(mseuil,False);
      return;
   }
   
   create_sseuil (parent,default_depth, default_visual,default_ollut);
   XtVaSetValues(fseuilb,XmNminimum,(int)(ecran.minval*normseuil),
		 XmNmaximum,(int)(ecran.maxval*normseuil)+1,
		 XmNdecimalPoints,decimal,
		 XmNvalue,(int)(ecran.minval*normseuil),
		 XmNshowValue,True,NULL);
   XtVaSetValues(fseuilh,XmNminimum,(int)(ecran.minval*normseuil),
		 XmNmaximum,(int)(ecran.maxval*normseuil)+1,
		 XmNvalue,(int)(ecran.maxval*normseuil),
		 XmNdecimalPoints,decimal,
		 XmNshowValue,True,NULL);
}

/*======================================================================*/
/*				XLUT					*/
/*======================================================================*/

/**
 * Sets the colormap with the brightness and constrast. 
 * @param colin the source color.
 * @return the color modified by contrast and brightness values.
 */
XColor setRGB(PColor colin){
   float mn=colin.red,mx=colin.red; 
   int maxVal=0; 
   float h=0;
   float s;
   int v;
   XColor colout;

   if (colin.green > mx){ mx=colin.green; maxVal=1;} 
   if (colin.blue > mx){ mx=colin.blue; maxVal=2;}  
   if (colin.green < mn) mn=colin.green; 
   if (colin.blue < mn) mn=colin.blue;  

   float  delta = mx - mn; 
   v =(int)mx;  
   if( mx != 0 ) {
      s = delta / mx;  
      if (s==0.0f) 
	 h=-1;  
      else {
	 switch (maxVal) { 
	 case 0: { h = (colin.green - colin.blue ) / delta; break; }         // yel < h < mag 
	 case 1: { h = 2 + ( colin.blue - colin.red ) / delta; break; }     // cyan < h < yel 
	 case 2: { h = 4 + ( colin.red - colin.green ) / delta; break; }     // mag < h < cyan 
	 } 
      }
   } 
   else { 
      s = 0; 
      h = 0;  
   } 
   h *= 60; 
   if( h < 0 ) h += 360; 
   //apply brightness
   v+=(int)(0.5*_brightness);
   if(v>=MAXCOLS)
      v=MAXCOLS-1;
   if(v<0)
      v=0;
   int i; 
   float f, p, q, t,hTemp; 
   if ( s == 0.0 || h == -1.0) {// s==0? Totally unsaturated = grey so R,G and B all equal value 
      colin.red = colin.green = colin.blue = v;
      //apply contrast
      if(colin.red<127){
	 if(colin.red-_contrast<0) colin.red=0;
	 else if(colin.red-_contrast>127) colin.red=127;
	 else colin.red-=_contrast;
      }
      else if(colin.red>127){
	 if(colin.red+_contrast<127) colin.red=127;
	 else if(colin.red+_contrast>=MAXCOLS) colin.red=MAXCOLS-1;
	 else colin.red+=_contrast;
      }
      colout.green=colout.blue=colout.red=((unsigned int)colin.red)<<8;
      return colout;
   } 
   hTemp = h/60.0f; 
   i = (int)floor( hTemp );  // which sector 
   f = hTemp - i;            // how far through sector 
   p = v * ( 1 - s ); 
   q = v * ( 1 - s * f ); 
   t = v * ( 1 - s * ( 1 - f ) ); 
   
   switch( i ) { 
   case 0:{colin.red = (Uchar)v;colin.green = (Uchar)t;colin.blue = (Uchar)p;break;} 
   case 1:{colin.red = (Uchar)q;colin.green = (Uchar)v;colin.blue = (Uchar)p;break;} 
   case 2:{colin.red = (Uchar)p;colin.green = (Uchar)v;colin.blue = (Uchar)t;break;} 
   case 3:{colin.red = (Uchar)p;colin.green = (Uchar)q;colin.blue = (Uchar)v;break;}  
   case 4:{colin.red = (Uchar)t;colin.green = (Uchar)p;colin.blue = (Uchar)v;break;} 
   case 5:{colin.red = (Uchar)v;colin.green = (Uchar)p;colin.blue = (Uchar)q;break;} 
   }
   //apply contrast
   if(colin.red<127){
      if(colin.red-_contrast<0) colin.red=0;
      else if(colin.red-_contrast>127) colin.red=127;
      else colin.red-=_contrast;
   }
   else if(colin.red>127){
      if(colin.red+_contrast<127) colin.red=127;
      else if(colin.red+_contrast>=MAXCOLS) colin.red=MAXCOLS-1;
      else colin.red+=_contrast;
   }
   if(colin.green<127){
      if (colin.green-_contrast<0) colin.green=0;
      else if(colin.green-_contrast>127) colin.green=127;
      else colin.green-=_contrast;
   }
   else if(colin.green>127){
      if(colin.green+_contrast<127) colin.green=127;
      else if(colin.green+_contrast>=MAXCOLS) colin.green=MAXCOLS-1;
      else colin.green+=_contrast;
   }
   if(colin.blue<127){
      if(colin.blue-_contrast<0) colin.blue=0;
      else if(colin.blue-_contrast>127) colin.blue=127;
      else colin.blue-=_contrast;
   }
   else if(colin.blue>127){
      if(colin.blue+_contrast<127) colin.blue=127;
      else if(colin.blue+_contrast>=MAXCOLS) colin.blue=MAXCOLS-1;
      else colin.blue+=_contrast;
   }
   colout.red=((unsigned int)colin.red)<<8;
   colout.green=((unsigned int)colin.green)<<8;
   colout.blue=((unsigned int)colin.blue)<<8;
   return colout;
}

/**
 * Installs the current colormap with the current lut.
 * @param lut the colormap.
 * @param lutin the current lut.
 */
void installeLut(Colormap lut, XColor lutin[]) {
   XColor c;
   int i;

   c.flags=DoRed | DoGreen | DoBlue;
   for(i=MAXCOLS-1; i>=0; i--){
      c=lutin[i];
      if (!XAllocColor(dpy,lut,&c))
	 break; // No more color available.
      imlut[i]=c;
   }

   // Try use nearest colors within the default colormap.
   if (i>0) {
      int k;
      XColor colortab[MAXCOLS];

      for (int c=0; c<MAXCOLS; c++) colortab[c].pixel=c;
      XQueryColors(dpy,lut, colortab, MAXCOLS);
      for (;i>=0;i--) {
	int d, mdist, best;
	int rx, gx, bx;
	int r,g,b;
	
	mdist = MAXLONG;
	best = -1;
	r = lutin[i].red>>8;
	g = lutin[i].green>>8;
	b = lutin[i].blue>>8;
	
	for (k=0; k<MAXCOLS; k++) {
	  rx = r - (colortab[k].red  >>8);
	  gx = g - (colortab[k].green>>8);
	  bx = b - (colortab[k].blue >>8);
	  
	  d = SQR(rx) + SQR(gx) + SQR(bx);
	  if (d<mdist) {
	     mdist=d;
	     best=k;
	  }
	}
	
	if (best>=0 && XAllocColor(dpy, lut, &colortab[best])) {
	   imlut[i]= colortab[best];
	}
      }
   }

   // Stylo
   XParseColor(dpy,lut,"red",&c);
   XAllocColor(dpy,lut,&c);
   stylo=c.pixel;
   imlut[MAXCOLS]=c;
}

/**
 * Change color palette.
 */
void changePalette(Widget widget, XtPointer client_data, XtPointer call_data) {
   Window xwin;
   int i;
   
   for (i=0; i<MAXCOLS; i++)
      imlut[i]=setRGB(srclut[i]);
   installeLut(ollut, imlut);

   if (!flut || (!XtIsRealized(widget))) // Not displayed.
      return;
   
   xwin = XtWindow(fpalette);
   XClearArea(dpy,xwin,0,0,255,20,0);

   switch(depth){
   case 8:
      for (i=0; i<MAXCOLS; i++){
	 XSetForeground(dpy,gc,imlut[i].pixel);
	 XDrawLine(dpy,xwin,gc,i+1,1,i+1,19);
      }
      break;
   case 16:{
      int rshift = bitShiftNumber(visual->red_mask) - 4;
      int gshift = bitShiftNumber(visual->green_mask) - 4;
      int bshift = bitShiftNumber(visual->blue_mask) - 4;
      for (i=0; i<MAXCOLS; i++){
	 unsigned long temp=
	    (((imlut[i].red   >> 11) & 0xff) << rshift) |
	    (((imlut[i].green >> 11) & 0xff) << gshift) |
	    (((imlut[i].blue  >> 11) & 0xff) << bshift);
	 XSetForeground(dpy,gc,temp);
	 XDrawLine(dpy,xwin,gc,i+1,1,i+1,19);
      }
   }break;
   case 24: 
   case 32: {
      int rshift = bitShiftNumber(visual->red_mask) - 7;
      int gshift = bitShiftNumber(visual->green_mask) - 7;
      int bshift = bitShiftNumber(visual->blue_mask) - 7;
      
      for (i=0; i<MAXCOLS; i++){
	 unsigned long temp=
	    (((imlut[i].red   >> 8) & 0xff) << rshift) |
	    (((imlut[i].green >> 8) & 0xff) << gshift) |
	    (((imlut[i].blue  >> 8) & 0xff) << bshift);
	 XSetForeground(dpy,gc,temp);
	 XDrawLine(dpy,xwin,gc,i+1,1,i+1,19);
      }
   }break;

   }
}

/**
 * Constrast is simulated by a linear redistribution of colors.
 * Brightness is simulated by adding (or substracting) a constant value.
 */
void changeContrastBrightness(int c, int b) {
   changePalette(fpalette,NULL,NULL);
   changeImage();
   afficheValeurs(fvaleurs,NULL,NULL);
}

/**
 * Increase or decrease contrast.
 * Value is in %
 */
void changeContrast(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmScaleCallbackStruct* cbs = (XmScaleCallbackStruct*)call_data;
   _contrast=(cbs->value * MAXCOLS)/(2*100);
   
   changeContrastBrightness(_contrast, _brightness);
}

/**
 * Increase or decrease brightness
 * Value is in % of MAXCOLS
 */
void changeBrightness(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmScaleCallbackStruct* cbs = (XmScaleCallbackStruct*)call_data;
   _brightness= cbs->value * MAXCOLS/100;
   
   changeContrastBrightness(_contrast, _brightness);
}

/**
 * Changes current colormap.
 * Also changes the pen color.
 * Remember: There's only MAXCOLS>>coloffset colors available.
 */
void changeLut(Widget widget, XtPointer client_data, XtPointer call_data) {
   void changeImage();

   switch((int)client_data){
   case 0:
      for (int i=0;i<MAXCOLS;i++) {
	 srclut[i].red=ecran.rmap[i];
	 srclut[i].green=ecran.gmap[i];
	 srclut[i].blue=ecran.bmap[i];
      }
      break;
   case 1:
      for (int i=0;i<MAXCOLS;i++) {
	 srclut[MAXCOLS-1-i].red=ecran.rmap[i];
	 srclut[MAXCOLS-1-i].green=ecran.gmap[i];
	 srclut[MAXCOLS-1-i].blue=ecran.bmap[i];
      }
      break;
   case 2: // Color: I don't know where I found that algorithm!
      for(int i=0; i<MAXCOLS;i++){
	 srclut[i].red   =  ((i>>coloffset<<coloffset) & 0x92);
	 srclut[i].green = (((i>>coloffset<<coloffset) & 0x49)*2);
	 srclut[i].blue  = (((i>>coloffset<<coloffset) & 0x24)*4);
      }
      break;
   case 3: // Region
      srclut[0].red=srclut[0].green=srclut[0].blue=0;
      for(int i=1; i<MAXCOLS; i++){
	 XColor c;
	 switch((i % 15)){
	 case 0: XParseColor(dpy,ollut,"yellow",&c);break;
	 case 1: XParseColor(dpy,ollut,"green",&c);break;
	 case 2: XParseColor(dpy,ollut,"blue",&c);break;
	 case 3: XParseColor(dpy,ollut,"yellow",&c);break;
	 case 4: XParseColor(dpy,ollut,"aquamarine",&c);break;
	 case 5: XParseColor(dpy,ollut,"magenta",&c);break;
	 case 6: XParseColor(dpy,ollut,"chartreuse",&c);break;
	 case 7: XParseColor(dpy,ollut,"gray",&c);break;
	 case 8: XParseColor(dpy,ollut,"MistyRose",&c);break;
	 case 9: XParseColor(dpy,ollut,"snow",&c);break;
	 case 10: XParseColor(dpy,ollut,"Coral",&c);break;
	 case 11: XParseColor(dpy,ollut,"Cyan",&c);break;
	 case 12: XParseColor(dpy,ollut,"DarkOrchid",&c);break;
	 case 13: XParseColor(dpy,ollut,"Gold",&c);break;
	 case 14: XParseColor(dpy,ollut,"navy",&c);break;
	 }
	 srclut[i].red=c.red>>8;
	 srclut[i].green=c.green>>8;
	 srclut[i].blue=c.blue>>8;
      }
      break;
   case 4: // Random
      if (depth==8){
	 for (int i=0; i<MAXCOLS;i++){
	    XColor c;
	    c.pixel = rand() % MAXCOLS;
	    XQueryColor(dpy,ollut,&c);
	    srclut[i].red= c.red>>8;
	    srclut[i].green= c.green>>8;
	    srclut[i].blue= c.blue>>8;
 	 }
      }else
	 for (int i=0; i<MAXCOLS>>coloffset;i++){
	    srclut[i].red=(rand() % MAXCOLS);
	    srclut[i].green=(rand() % MAXCOLS);
	    srclut[i].blue=(rand() % MAXCOLS);
	 }
      break;
   case 5: // Rainbow   
      for (int i=0;i<MAXCOLS;i+=1){
	 int j = MAXCOLS-1-i;
	 double d=(d=cos((double)((j-MAXCOLS*0.16)*(M_PI/MAXCOLS)))) < 0.0 ? 0.0 : d;
	 srclut[i].blue = (Uchar)(d*MAXCOLS);
	 d = (d=cos((double)((j-MAXCOLS*0.52)*(M_PI/MAXCOLS)))) < 0.0 ? 0.0 :d;
	 srclut[i].green = (Uchar)(d*MAXCOLS);
	 d = (d=cos((double)((j-MAXCOLS*0.82)*(M_PI/MAXCOLS)))) < 0.0 ? 0.0 :d;
	 double e = (e=cos((double)((j * (M_PI/MAXCOLS)))) < 0.0)? 0.0 : e;
	 srclut[i].red = (Uchar)(d*MAXCOLS + e *(MAXCOLS/2));
      }
      break;
   case 6: // Noise
      for (int i=0; i<MAXCOLS;i++){
	 switch(i%3){
	 case 0:
	    srclut[i].red = 255;
	    srclut[i].green = 0;
	    srclut[i].blue = 0;
	    break;
	 case 1:
	    srclut[i].red = 0;
	    srclut[i].green = 255;
	    srclut[i].blue = 0;
	    break;
	 case 2:
	    srclut[i].red = 0;
	    srclut[i].green = 0;
	    srclut[i].blue = 255;
	    break;
	 }
      }
      break;
   }
   XmScaleSetValue(scontrast,_contrast=0);
   XmScaleSetValue(sbrightness, _brightness=0);

   changePalette(fpalette, NULL, NULL);
   changeImage();
   afficheValeurs(fvaleurs,NULL,NULL);
}

/**
 * Triggered when selected lut item of the menu.
 * -> Display related pop menu.
 */
void menuLut(Widget widget, XtPointer client_data, XtPointer call_data) {
   XtManageChild(XtParent(flut));
}

/**
 * Initializes the LUT Widget.
 */
void CreeCadreLut(Widget parent) {
   create_slut (parent,depth, visual,ollut);
}

// Dynamic

/**
 * Triggered when selected lut item of the menu.
 * -> Displays related pop menu.
 */
void menuDynamic(Widget widget, XtPointer client_data, XtPointer call_data) {
   XtManageChild(XtParent(fdynamic));
}

/**
 * Initializes the LUT Widget.
 */
void CreeCadreDynamic(Widget parent) {
   create_sdynamic (parent,depth, visual,ollut);
}

/**
 * Changes the image dynamic betwen "Contrast stretching" and "Histogram equalization".
 * Does not work with colored images!
 */
void changeDynamic(Widget widget, XtPointer client_data, XtPointer call_data) {
   Errc Recadrage(Float* s, Img3duc& imd, int outbands) ;
   Errc Egalisation(Float* s, Img3duc& imd, int outbands) ;

   switch((int)client_data){
   case 0:
      Recadrage(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);
      break;
   case 1:
      Egalisation(ecran.imsource->VectorX(),*ecran.imlogic,MAXCOLS);
      break;
   }
   changeImage();
}

/*======================================================================*/
/*				XZOOM					*/
/* -> (imzoom,imzoomy)							*/
/*======================================================================*/

/**
 * Triggered when selecting zoom item of the menu.
 * The zoom value is indicated by client_data.
 * Places the selection rectangle in the center of the window, by
 * adjusting the scrollbar.
 * Calls `changeImage' to calculate and to display the new image.
 */
void menuZoom(Widget widget, XtPointer client_data, XtPointer call_data) {
   ::Dimension winheight, winwidth;
   int offsety, offsetx;
   int v1,v2,v3,v4;

   imzoom = (int)client_data;
   if (imzoom<0) imzoom = 1/-imzoom;
   XtVaSetValues(tableau,XmNwidth,int(ecran.imsource->Width()*imzoom),XmNheight,int(ecran.imsource->Height()*imzoom),NULL);
   changeImage();

   // Determine offsets to put the select rectangle (vx,vy)
   // in the center of each scrollbars.
   offsetx=0;
   XtVaGetValues(cadre,XmNheight,&winwidth,NULL);
   if (winwidth < int(ecran.imsource->Width()*imzoom)){
      offsetx = int(((vx+(selrectdim/2))*imzoom)-(winwidth/2));
      if ((offsetx+winwidth)>= int(ecran.imsource->Width()*imzoom))
	 offsetx = int((ecran.imsource->Width()*imzoom)-winwidth-1);
      if (offsetx<0)
	 offsetx=0;
   }

   offsety=0;
   XtVaGetValues(cadre,XmNwidth,&winheight,NULL);
   if (winheight < int(ecran.imsource->Height()*imzoom)){
      offsety = int(((vy+selrectdim/2)*imzoom)-(winheight/2));
      if ((offsety+winheight)>= int(ecran.imsource->Height()*imzoom))
	 offsety = int((ecran.imsource->Height()*imzoom)-winheight-1);
      if (offsety<0)
	 offsety=0;
   }

   XmScrollBarGetValues(hscrollbar,&v1,&v2,&v3,&v4);
   XmScrollBarSetValues(hscrollbar,offsetx,v2,v3,v4,True);
   XmScrollBarGetValues(vscrollbar,&v1,&v2,&v3,&v4);
   XmScrollBarSetValues(vscrollbar,offsety,v2,v3,v4,True);
}

/*======================================================================*/
/*				XMENU					*/
/*======================================================================*/


/**
 * Triggered by the Drawing menu item.
 * -> Erases the last line.
 */
void menuAnnuler(Widget widget, XtPointer client_data, XtPointer call_data) {
   memcpy(tracesdessin,donneesdessin,ecran.imsource->Width()*ecran.imsource->Height());
   changeImage();
}
  
/**
 * Triggered by the clear menu item.
 * -> Clears all the drawing memory.
 */
void menuEffacer(Widget widget, XtPointer client_data, XtPointer call_data) {
   memset(tracesdessin,0,ecran.imsource->Width()*ecran.imsource->Height());
   changeImage();
}

/*======================================================================*/
/*				XICON					*/
/* Use the current image as icon pixmap.				*/
/*======================================================================*/

/**
 * Creates icon. Display a reduced version of the image itself.
 */
void CreeIcone(Widget parent) {
   GC gcIcon;
   Pixmap icon;
   XIconSize *size_list;
   int iwidth, iheight;
   XImage *imicone;
   Char *data;
   int i,j,ratio;
   int pimx, pimy, picx,picy;

   // Get available icon sizes from window manager
   if (!XGetIconSizes(dpy,RootWindow(dpy,DefaultScreen(dpy)),&size_list,&ratio)){
      return;       // No icon sizes setting ...
   }else{
      iwidth = size_list[0].max_width;
      iheight= size_list[0].max_height;
   }

   // Determine ratio between image and icon.
   ratio=(ecran.imsource->Height()>ecran.imsource->Width())? ecran.imsource->Height()/iheight:ecran.imsource->Width()/iwidth;
   if (ratio<1) ratio=1;

   data = (char *)calloc(iwidth*iheight, sizeof(char)*bytes_per_pixel);
   imicone= XCreateImage(dpy,default_visual,default_depth,ZPixmap,0,data,iwidth,iheight,BitmapPad(dpy),iheight*bytes_per_pixel);
   
   // Draw image from the center -> best drawing
   for (i=0; i<iheight/2 ;i++){
      for (j=0; j<iwidth/2; j++){
	 if ( ((i*ratio)<(ecran.imsource->Height()/2)) && ( (j*ratio)< (ecran.imsource->Width()/2) )){
	    long pixel;
	    pimy = (ecran.imsource->Height()/2) + (i*ratio);
	    pimx = (ecran.imsource->Width()/2) + (j*ratio);
	    picy = (iheight/2) +i;
	    picx = (iwidth/2) +j;
	    
	    pixel=XGetPixel(imcarte,pimx,pimy);
	    XPutPixel(imicone,picx,picy,pixel);
	    
	    pimy = (ecran.imsource->Height()/2) - ((i+1)*ratio);	
	    picy = (iheight/2) - i -1;
	    pixel=XGetPixel(imcarte,pimx,pimy);
	    XPutPixel(imicone,picx,picy,pixel);
	    
	    pimx = (ecran.imsource->Width()/2) - ((j+1)*ratio);	
	    picx = (iwidth/2) - j -1;
	    pixel=XGetPixel(imcarte,pimx,pimy);
	    XPutPixel(imicone,picx,picy,pixel);
	    
	    pimy = (ecran.imsource->Height()/2) + (i*ratio);	
	    picy = (iheight/2) + i;
	    pixel=XGetPixel(imcarte,pimx,pimy);
	    XPutPixel(imicone,picx,picy,pixel);
	 }
      }
   }
   icon=XCreatePixmap(dpy,RootWindow(dpy,DefaultScreen(dpy)),iwidth,iheight,default_depth);
   gcIcon = XCreateGC(dpy, icon,0,NULL);
   XPutImage(dpy,icon,gcIcon,imicone,0,0,0,0,iwidth,iheight);
   XtVaSetValues(parent, XmNiconPixmap, icon, NULL);
   
   XFree(size_list);
   free(data);
}

/*======================================================================*/
/*				PROPERTIES				*/
/* Display image properties.						*/
/*======================================================================*/

/**
 * Displays image properties in a popup.
 */
void afficheProperties(Widget widget, XtPointer client_data, XtPointer call_data) {
   if (!XtIsRealized(widget)) // Not displayed.
      return;
   
   int y=16;
   char *text;

   text= ecran.type;
   XDrawString(dpy,XtWindow(widget),default_gc,0,y,text,strlen(text));   y+=16;
   text = ecran.content;
   XDrawString(dpy,XtWindow(widget),default_gc,0,y,text,strlen(text));   y+=16;
   text = ecran.pixel;
   XDrawString(dpy,XtWindow(widget),default_gc,0,y,text,strlen(text));   y+=16;
   text = ecran.size;
   XDrawString(dpy,XtWindow(widget),default_gc,0,y,text,strlen(text));   y+=16;
   text = ecran.etc;
   XDrawString(dpy,XtWindow(widget),default_gc,0,y,text,strlen(text));   y+=16;
}

/**
 * Display image properties.
 */
void menuProp(Widget w, XtPointer, XtPointer) {
   XtManageChild(XtParent(fprops));
}

/*======================================================================*/
/*				XSLICES					*/
/* Select the slice.							*/
/*======================================================================*/

/**
 * Selects the slice.
 */
void changeSlice(Widget widget, XtPointer client_data, XtPointer call_data) {
   XmScaleCallbackStruct* cbs = (XmScaleCallbackStruct*)call_data;
   tracesdessin-=(currentslice*ecran.imsource->Width()*ecran.imsource->Height());
   currentslice=cbs->value;

   tracesdessin+=(currentslice*ecran.imsource->Width()*ecran.imsource->Height());
   changeImage();
   CalculeHistogramme();
   changeHistogramme();
   changeValeurs();
   changeProfil();
}

/*======================================================================*/
/*				MENU					*/
/* Display and manage the popup menu.					*/
/*======================================================================*/

/**
 * Related menu : colorp==1  -> unable val, histo prof and seuil items.
 *		  drawingp=0 -> unable drawing submenu.
 */
void CreeMenu(Widget parent) {
   create_menu (parent,default_depth, default_visual,default_ollut);
   if (ecran.colored){
      XtSetSensitive(mprof,False);
      XtSetSensitive(mhisto,False);
      XtSetSensitive(mdynamic,False);
   }

   if (!ecran.dynamisable){
      XtSetSensitive(mdynamic,False);
   }

#ifndef __DRAWINGMODE
   XtSetSensitive(mdrawing,False);
   XtAddCallback( tableau, XmNinputCallback, menuTableauVisu, (XtPointer) 0 );
#else
   XtAddCallback( tableau, XmNinputCallback, menuTableauDessin, (XtPointer) 0 );
   XtAddEventHandler(tableau,Button1MotionMask,false,Trace,NULL);
#endif
}

/**
 * Triggered by the exit item of the menu.
 */
void sortie(Widget widget, XtPointer, XtPointer) {
   fin = true; // flag for end.
}


Widget cadre2 = (Widget) NULL;
XtAppContext app_context;

/**
 * Gets current background of the manager.
 */
Pixel getMotifColors() {
   cadre2=XtAppCreateShell (NULL, "XApplication", applicationShellWidgetClass, dpy, NULL,0 );
   // Create a dummy application to get WMW's background color.
   Widget bform = XmCreateForm ( cadre2, "bform", NULL, 0 );
   XtManageChild (bform);
   XColor bg;
   XtVaGetValues(cadre2,XmNbackground, &bg.pixel,NULL);
   XtDestroyWidget(bform);
   XtDestroyWidget(cadre2);
   
   // Get the true color.
   bg.flags = DoRed | DoGreen | DoBlue; 
   XQueryColor(dpy,default_ollut, &bg);
   XAllocColor(dpy,ollut,&bg);
   return bg.pixel;
   
}

/**
 * Motif: Need to set decoration colors for 24 bits depth Widgets.
 * (cadre, sclice, hscrollvbar, vscrollbar svaleurs).
 */
void reinstallMotifColors() {
   // Motif : Need to set color for 24 bits display Widget!
   XmChangeColor(cadre,bgcolor);
   XmChangeColor(baseform,bgcolor);
   XmChangeColor(slices,bgcolor);
   XmChangeColor(vscrollbar,bgcolor);
   XmChangeColor(hscrollbar,bgcolor);
   XmChangeColor(fvaleurs,bgcolor);
   XmChangeColor(lutbaseform,bgcolor);
   XmChangeColor(flut,bgcolor);
   XmChangeColor(scontrast,bgcolor);
   XmChangeColor(sbrightness,bgcolor);
}

/**
 * Creates all windows. Execute 50 loops.
 */
Errc AfficheX11(int drawingp) {
   // If image size > screen size -> use screen size
   int w=DisplayWidth(dpy,DefaultScreen(dpy));
   int h=DisplayHeight(dpy,DefaultScreen(dpy));
   w-=7;
   h-=(5+64);
   if (ecran.imsource->Width() < w &&  ecran.imsource->Height() < h) {
      w=ecran.imsource->Width();
      h=ecran.imsource->Height();
   }
   XtVaSetValues(cadre,XmNwidth,w+5,XmNheight,h+5+32,NULL);
   XtVaSetValues(tableau,XmNwidth,ecran.imsource->Width(),XmNheight,ecran.imsource->Height(),NULL);
   
   if (drawingp){
      donneesdessin = (Uchar*)calloc(ecran.imsource->Width()*ecran.imsource->Height(),sizeof(Uchar));
   }

   // Value rectangle
   if (ecran.colored)
      selrectdim=3;
   else
      selrectdim=6;

   // Set original LUT.
   for (int i=0;i<MAXCOLS;i++) {
      srclut[i].red=ecran.rmap[i];
      srclut[i].green=ecran.gmap[i];
      srclut[i].blue=ecran.bmap[i];
   }

   // Create all graphic components.
   changePalette(fpalette,NULL,NULL);
   CreeMenu(tableau);
   CreeCadreSeuil(cadre);
   CreeCadreHistogramme(cadre);
   CreeCadreValeurs(cadre); // same depth than the drawing area.
   CreeCadreLut(cadre);
   XtVaSetValues(XmGetXmDisplay(XtDisplay(flut)),
		 XmNdragInitiatorProtocolStyle, XmDRAG_NONE,
		 XmNdragReceiverProtocolStyle,  XmDRAG_NONE,
		 NULL);
   CreeCadreDynamic(cadre);
   XtVaSetValues(XmGetXmDisplay(XtDisplay(fdynamic)),
		 XmNdragInitiatorProtocolStyle, XmDRAG_NONE,
		 XmNdragReceiverProtocolStyle,  XmDRAG_NONE,
		 NULL);
   create_sprops(cadre,default_depth, default_visual,default_ollut);
   CreeCadreProfil(cadre);
   CreeImage(cadre);
   CreeIcone(cadre);

   reinstallMotifColors();

   // Set the slices slider.
   currentslice=0;
   if (ecran.imsource->Depth()==1){
      XtSetSensitive(slices,False);
      XtVaSetValues(slices,XmNminimum,1,XmNvalue,1,NULL);
   }else{
      XtVaSetValues(slices,XmNminimum,0,XmNmaximum,ecran.imsource->Depth()-1,NULL);
   }
   XtRealizeWidget (cadre);

   // Create the global Graphic Context : get all used colors.
   XColor bg;
   XtVaGetValues(cadre,XmNbackground, &bg.pixel,NULL);
   bgcolor = bg.pixel;
   default_gc = DefaultGC(dpy,DefaultScreen(dpy));
   default_fgcolor=0;
   default_bgcolor=1;
   gc = XCreateGC(dpy,XtWindow(tableau), 0,NULL);
   XParseColor(dpy,ollut,"black",&bg);
   XAllocColor(dpy,ollut,&bg);
   black_color=bg.pixel;
   XParseColor(dpy,ollut,"white",&bg);
   XAllocColor(dpy,ollut,&bg);
   white_color=bg.pixel;
   XSetForeground(dpy,gc,stylo);

   // Rewrite the XtAppMainLoop (app_context) to capture the exit event.
   XEvent event;

    do{
       XtAppNextEvent(app_context,&event);
       XtDispatchEvent(&event);
    }while(!fin);
    
    free(donneesdessin);
    return SUCCESS;
}

/**
 * Initializes a Motif session.
 * Set and Get environmental parameters.
 */
int InitXsession(char *vrainom, char *nomsortie, int argc, char* argv[]) {
   XVisualInfo visual_info;
   XPixmapFormatValues *xpixmapformatvalues,*x;

   XtSetLanguageProc ((XtAppContext)NULL,(XtLanguageProc)NULL,(XtPointer)NULL);
   XtToolkitInitialize();
   
   app_context = XtCreateApplicationContext ();

   dpy=XtOpenDisplay(app_context,NULL,"visu","XApplication",NULL,0,&argc,argv);
  
   if (!dpy){
      fprintf(stderr,"%s: can't open display, already exiting...\n", vrainom);
      return FAILURE;
   }

   // Get the default visual.
   default_visual = DefaultVisual(dpy,DefaultScreen(dpy));
   default_ollut = XDefaultColormap(dpy,DefaultScreen(dpy));
   default_depth = DefaultDepth(dpy,DefaultScreen(dpy));

   // Get the best visual.
   if (XMatchVisualInfo(dpy, DefaultScreen(dpy), 24, TrueColor, &visual_info)){
      visual = visual_info.visual;
      depth = visual_info.depth;
      ollut = XCreateColormap(dpy,DefaultRootWindow(dpy),visual,AllocNone);
    }else{
      visual = default_visual;
      ollut = default_ollut;
      depth = default_depth;
    }
   bgcolor= getMotifColors();
   create_cadre(dpy,vrainom, 0, NULL, /** added by hand */depth, visual, ollut,bgcolor);
   int count_return;
   x=xpixmapformatvalues=XListPixmapFormats(dpy,&count_return);
   for (;count_return >0; count_return--)
      if (xpixmapformatvalues->depth==depth){
	 bytes_per_pixel=xpixmapformatvalues->bits_per_pixel/8;
	 break;
      } else
	 xpixmapformatvalues++;
   XFree(x);
   return depth;
}
