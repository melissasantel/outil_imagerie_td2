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
 * @author Régis Clouard - 2007-31-12
 */

/**
 * @file infoview.h
 *
 * This class opens a windows where some information can be displayed.
 */

#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QtGui>
#include <pandore.h>

/**
 * This class defines the user interface to display texts.
 */
class InfoView : public QMainWindow {

   Q_OBJECT

public:

   /**
    * Creates a window that contains the menu,
    * and displays the specified text.
    * @param text a text.
    * @param file the name of input file.
    */
   InfoView( const char *text, const char *file );

   /**
    * Creates a window that contains the menu,
    * and displays the collection contents.
    * @param cols the input collection.
    * @param file the name of input file.
    */
   InfoView( const pandore::Collection &cols, const char *file );

private :

   /** The menu bar. */
   QMenuBar *_menu;

   /** The window that displays the properties of the input image. */
   QTextEdit *_contents;

private slots:

/**
 * Opens a new image in the application.
 */
void openImage();

protected:
   void closeEvent(QCloseEvent *event);
    
private:

   /**
    * Creates a window that contains the menu,
    * and displays the colection contents.
    * @param file The name of input file.
    */
   void Init( const char *file );
   
   /**
    * Builds the menu with all items.
    */
   void buildMenu();

   /**
    *  Displays the contents of the window.
    */
   void displayContents();

   /**
    * Sets the title of the window with the given filename
    * without its extension and directory.
    * @param file The absolute filename.
    */
   void CreateTitle( const char *file );

public : 
   
   /**
    * Quits the application.
    */
   ~InfoView( ) {
      delete _menu;
      delete _contents;
   }
};

#endif
