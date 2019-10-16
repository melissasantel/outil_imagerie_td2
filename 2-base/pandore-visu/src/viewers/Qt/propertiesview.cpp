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
 * @author Régis Clouard - 2006-01-18
 */

/**
 * @file propertiesview.cpp
 * 
 * Defines the class which shows in a window the information
 * about the input image.
 */

#include "propertiesview.h"

PropertiesView::PropertiesView( const ImageModel *model ) {
   setAttribute(Qt::WA_QuitOnClose,false);
   QString temp=model->windowTitle();
   temp.append(" - Properties");
   setWindowTitle(temp);
   const ImageSource *ims=model->getImageSource();
   temp=QString("<table border=0>");
   temp.append("<tr><th>Type</th><td><font color=red>");
   temp.append(ims->getType());
   temp.append("</font></td></tr><tr><th>Content</th><td>");
   temp.append(ims->getContent());
   temp.append("</td></tr><tr><th>Size</th><td>");
   temp.append(ims->getSize());
   char tempString[255];
   if (ims->getPixel()!=0){
      temp.append("</td></tr><tr><th>Pixel</th><td>");
      temp.append(ims->getPixel());
      temp.append("</td></tr><tr><th>Min value</th><td>");
      if (ims->getTypeval()==ImageSource::tfloat)
	 sprintf(tempString,"%#.3g",ims->getMinval());
      else
	 sprintf(tempString,"%.0f",ims->getMinval());
      temp.append(tempString);
      temp.append("</td></tr><tr><th>Max value</th><td>");
      if (ims->getTypeval()==ImageSource::tfloat)
	 sprintf(tempString,"%#.3g",ims->getMaxval());
      else
	 sprintf(tempString,"%.0f",ims->getMaxval());
      temp.append(tempString);
      if (ims->getEtc()!=0){
	 temp.append("</td></tr><tr><th>");
	 temp.append(ims->getEtc());
      }
   } else if (ims->getLabel()!=0) {
      temp.append("</td></tr><tr><th>Label</th><td>");
      temp.append(ims->getLabel());
      temp.append("</td></tr><tr><th>Label max</th><td>");
      temp.append(ims->getLabelMax());
   } else {
      temp.append("</td></tr><tr><th>Node</th><td>");
      temp.append(ims->getNode());
      temp.append("</td></tr><tr><th>Total nodes</th><td>");
      temp.append(ims->getTotalNodes());
   }
   temp.append("</td></tr></table>");
   QTextEdit *properties=new QTextEdit(this);
   properties->setReadOnly(true);
   properties->setHtml(temp);
   setCentralWidget(properties);
}
