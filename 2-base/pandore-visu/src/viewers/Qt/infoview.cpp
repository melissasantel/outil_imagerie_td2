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
 * @author Régis Clouard - 2007-01-03
 */
    
/**
 * @file infoview
 *
 * This widget displays a text or a collection contents.
 */

#include <limits>
#include "infoview.h"
#include <pandore.h>
using namespace pandore;

extern char *nomentree;

void InfoView::Init( const char *file ) {
   _contents=new QTextEdit(this);
   setCentralWidget(_contents);

   setMinimumSize(400,100);
   CreateTitle(file);
   buildMenu();
   _contents->setReadOnly(true);

   QFont f=_contents->font();
   f.setPointSize(8);
   _contents->setFont(f); 
   _contents->setWordWrapMode(QTextOption::WrapAnywhere);
   setFocus();
}

InfoView::InfoView( const char *text, const char *file ) {
   Init(file);
   if (text != NULL)
      _contents->insertPlainText(QString(text));
   setFocus();
}

InfoView::InfoView( const Collection &cols, const char *file ) {
   Init(file);
   char tmp[512];

   std::list<std::string> l = cols.List();
   _contents->setTextColor(Qt::red);
   if ((int)l.size()>1) {
      sprintf(tmp,"COLLECTION\nNumber of elements: %d\n",(int)l.size());
   } else {
      sprintf(tmp,"COLLECTION\nNumber of element: %d\n",(int)l.size());
   }
   _contents->insertPlainText(QString(tmp));

   std::list<std::string>::const_iterator i;
   for ( i = l.begin(); i != l.end(); ++i ) {
      BundledObject* bo = cols.Get(*i);
      char type_att[255];
      char att[255];
      strcpy(type_att,bo->Type().c_str());
      strcpy(att,i->c_str());
      _contents->setTextColor(Qt::blue);
      sprintf(tmp,"  * %s (%s):\n",att, type_att);
      _contents->insertPlainText(QString(tmp));
      _contents->setTextColor(Qt::black);

      if (!strcasecmp(type_att,"Double")) {
         Double &f=cols.GETVALUE(att,Double);
         sprintf(tmp,"%f\n",f);
         _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Float")) {
	 Float &f=cols.GETVALUE(att,Float);
	 sprintf(tmp,"%f\n",f);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Long")) {
	 Long &l=cols.GETVALUE(att,Long);
	 sprintf(tmp,"%d\n",l);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Ulong")) {
	 Ulong &l=cols.GETVALUE(att,Ulong);
	 sprintf(tmp,"%u\n",l);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Short")) {
	 Short &l=cols.GETVALUE(att,Short);
	 sprintf(tmp,"%hd\n",l);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Ushort")) {
	 Ushort &l=cols.GETVALUE(att,Ushort);
	 sprintf(tmp,"%hu\n",l);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Char")) {
	 Char &uc=cols.GETVALUE(att,Char);
	 sprintf(tmp,"%d\n",(int)uc);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Uchar")) {
	 Uchar &uc=cols.GETVALUE(att,Uchar);
	 sprintf(tmp,"%d\n",(int)uc);
	 _contents->insertPlainText(QString(tmp));
      } else if (!strcasecmp(type_att,"Array:Double")){
	 Double *tab=cols.GETARRAY(att,Double);
	 Long taille=cols.GETARRAYSIZE(att,Double);
	 for (int j=0;j<taille;j++) {
            sprintf(tmp,"%f\t",tab[j]);
            _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Float")){
	 Float *tab=cols.GETARRAY(att,Float);
	 Long taille=cols.GETARRAYSIZE(att,Float);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%f\t",tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Long")) {
	 Long *tab=cols.GETARRAY(att,Long);
	 Long taille=cols.GETARRAYSIZE(att,Long);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%d\t",tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Ulong")) {
	 Ulong *tab=cols.GETARRAY(att,Ulong);
	 pandore::Long taille=cols.GETARRAYSIZE(att,Ulong);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%u\t",tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Short")) {
	 Short *tab=cols.GETARRAY(att,Short);
	 Long taille=cols.GETARRAYSIZE(att,Short);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%hd\t",tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      }  else if (!strcasecmp(type_att,"Array:UShort")){
         Ushort *tab=cols.GETARRAY(att,Ushort);
         Long taille=cols.GETARRAYSIZE(att,Ushort);
         for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%hu\t",tab[j]);
	    _contents->insertPlainText(QString(tmp));
         }
         _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Char")) {
	 Char *tab=cols.GETARRAY(att,Char);
	 Long taille=cols.GETARRAYSIZE(att,Char);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%d\t",(int)tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else if (!strcasecmp(type_att,"Array:Uchar")){
	 Uchar *tab=cols.GETARRAY(att,Uchar);
	 Long taille=cols.GETARRAYSIZE(att,Uchar);
	 for (int j=0;j<taille;j++) {
	    sprintf(tmp,"%d\t",(int)tab[j]);
	    _contents->insertPlainText(QString(tmp));
	 }
	 _contents->insertPlainText(QString("\n"));
      } else {
	 _contents->insertPlainText(QString(" unprintable value(s)\n"));
      }
   }
}

void InfoView::CreateTitle( const char *filename ) {
   int i=strlen(filename);
   for (;i>=0;i--) {
      if (filename[i]=='/' || filename[i]=='\\') 
    	 break;
   }
   setWindowTitle(QString(&filename[++i]));
}

void InfoView::buildMenu( ) {
   _menu=menuBar();
   QMenu *file=new QMenu("&File");
   file->addAction(tr("&Open"),this,SLOT(openImage()));
   file->addAction(tr("&Quit"),this,SLOT(close()));
   _menu->addMenu(file);
}

void InfoView::openImage( ) {
   QWidget *OpenFile(pandore::Pobject *objs, pandore::Pobject **objd);
   QString imageFile = QFileDialog::getOpenFileName(this,
    						    "Choose an image file",
    						    ".",
    						    "Images (*.pan)");
   if (!imageFile.isEmpty() && QFile::exists(imageFile)) {
      nomentree=strdup(imageFile.toAscii().data());
      pandore::Pobject *objs=pandore::LoadFile(nomentree);
      if (objs!=NULL) {
	 (void)OpenFile(objs,NULL);
      }
   }
}

void InfoView::closeEvent( QCloseEvent *event ) {
   event->accept();
}
