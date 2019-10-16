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
 * @author Régis Clouard - 2006-01-18 (add imx images)
 */

/**
 * @file imagevisu.h
 *
 * Defines the source and logic images used
 * to display the input image.
 */

#ifndef IMAGEVISU_H
#define IMAGEVISU_H

#include <pandore.h>

using namespace pandore;

#define MAXCOLS 256	// Number of virtual used colors 256

/**
 * This class contains all data of the original image.
 * It inherits from Imc3dsf because it is the
 * largest type that includes all other image type.
 */
class ImageSource : public Imx3dsf {

public :

   /**
    * This class contains the information about the 
    * input image without the pixel values.
    */
   struct Info {
      /** The name of the Pandore type of input image. */
      const char *type;
      /** The description of the input image data. */
      const char *content;
      /** The name of the C type of input image pixels. */  
      const char *pixel;
      /** The description of the size of the input image. */
      const char *size;
      /** Some miscellaneous information. */
      const char *etc;
      /** The type of data for region. */
      const char *label;
      /** The type of node for graph. */
      const char *node;
      /** The highest label in case of region data. */
      const char *labelMax;
      /** The number of node in case of graph. */
      const char *totalNodes;
      Info(): etc(0),label(0),node(0),labelMax(0),totalNodes(0) {}
   };
   
   typedef enum {tchar, tlong, tfloat} TypeData;

private:

   /** The minimum value of the source values */ 
   float _minval;
   /** The maximum value of the source values */   
   float _maxval;
   /** The flag for redynamisable image */  
   bool _dynamisable;
   /** The flag for color image */
   bool _colored;
   /** The type of original data */
   TypeData _typeval;  
   /** The flag for region region map. */
   bool _region;
   /** The flag for graph image. */
   bool _graph;
   /** The information about the input image. */
   Info _info;

public :

   /**
    * Creates an Imc3dsf with specific information,
    * and call the builder of Imx3dsf.
    * @param nbands The number of bands in case of the multispectral image.
    * @param depth The depth of the input image.
    * @param height The height of the input image.
    * @param width The width of the input image.
    */
   ImageSource( int nbands, long depth, long height, long width ): Imx3dsf(nbands,depth,height,width), _dynamisable(true), _region(false),_graph(false), _info() { }
 
   /**
    * Sets the type name of the input image.
    * @param type The type of the image.
    */
   void setType( const char *type ) {
      _info.type=type;
   }
      
   /**
    * Sets the description of the input image data.
    * @param content The content of the image.
    */
   void setContent( const char *content ) {
      _info.content=content;
   }
      
   /**
    * Sets the size of the input image.
    * @param size The size described in a sentence.
    */
   void setSize( const char *size ) {
      _info.size=size;
   }
      
   /**
    * Sets information about pixel of the image.
    * @param pixel The information about the input pixel.
    */
   void setPixel( const char *pixel ) {
      _info.pixel=pixel;
   }

   /**
    * Sets miscellaneous information.
    * @param text The text.
    */
   void setEtc( const char *text ) {
      _info.etc=text;
   }

   /**
    * Sets information about nodes nodes of the input graph.
    * @param node The type of node.
    */
   void setNode( const char *node) {
      _info.node=node;
   }

   /**
    * Sets information about labels of the input region map.
    * @param label The type of label.
    */
   void setLabel( const char *label ) {
      _info.label=label;
   }

   /**
    * Sets information about max label of the input region map.
    * @param max The highest label.
    */
   void setLabelMax( const char *max ) {
      _info.labelMax=max;
   }

   /**
    * Sets information about the total niumber of node of the input graph.
    * @param val The number of nodes.
    */
   void setTotalNodes( const char *val ) {
      _info.totalNodes=val;
   }

   /**
    * Sets all information about the input image in the structure info
    */
   void setInfo( const Info informa ) {
      _info=informa;
   }
   
   /**
    * Sets the value of minval.
    * @param val The value.
    */
   void setMinval( float val ) {
      _minval=val;
   }

   /**
    * Sets the value of maxval.
    * @param val The value.
    */
   void setMaxval( float val ) {
      _maxval=val;
   }
   
   /**
    * Sets whether the image is dynamisable.
    * @param val The boolean.
    */
   void setDynamisable( bool val ) {
      _dynamisable=val;
   }
   
   /**
    * Sets whether the image is colored.
    * @param val The boolean.
    */
   void setColored( bool val ) {
      _colored=val;
   }

   /**
    * Sets the type of values of original image.
    * @param  val The type.
    */
   void setTypeval( TypeData val ) {
      _typeval=val;
   }
   
   /**
    * Sets whether the input image is a graph.
    * @param val The boolean.
    */
   void setGraph( bool val ) {
      _graph=val;
   }

   /**
    * Sets whether the input image is a region map.
    * @param val The boolean.
    */
   void setRegion( bool val ) {
      _region=val;
   }


   /**
    * @return minval.
    */
   float getMinval( ) const {
      return _minval;
   }
   
   /**
    * @return maxval.
    */
   float getMaxval( ) const {
      return _maxval;
   }
   
   /**
    * @return size.
    */
   const char* getSize( ) const {
      return _info.size;
   }

   /**
    * @return type.
    */
   const char* getType( ) const {
      return _info.type;
   }
  
   /**
    *  @return content.
    */
   const char* getContent( ) const {
      return _info.content;
   }

   /**
    * @return pixel.
    */
   const char* getPixel( ) const {
      return _info.pixel;
   }

   /**
    * @return miscellaneous information.
    */
   const char* getEtc( ) const {
      return _info.etc;
   }

   /**
    * @return label.
    */
   const char* getLabel( ) const {
      return _info.label;
   }
   
   /**
    * @return node.
    */
   const char* getNode( ) const {
      return _info.node;
   }

   /**
    * @return labelMax.
    */
   const char* getLabelMax( ) const {
      return _info.labelMax;
   }

   /**
    * @return total nodes.
    */
   const char* getTotalNodes( ) const {
      return _info.totalNodes;
   }

   /**
    * @return The type of values.
    */
   TypeData getTypeval( ) const {
      return _typeval;
   }

   /**
    * Checks whether the input image is a graph.
    * @return True if the image is a graph or a region.
    */
   bool isGraph( ) const {
      return _graph;
   }

   /**
    * Checks whether the input image is a region map.
    * @return True if the image is a graph or a region.
    */
   bool isRegion( ) const {
      return _region;
   }

   /**
    * Checks whether the image is colored.
    * @return True if the image is colored.
    */
   bool isColored( ) const {
      return _colored;
   }

   /**
    * Checks whether the image is dynamisable.
    * @return True if the image is dynamisable.
    */
   bool isDynamisable( ) const {
      return _dynamisable;
   }
   
   /**
    * @return All information about the input image.
    */
   Info getInfo( ) const {
      return _info;
   }
};

/**
 * This class contains data that is actually displayed.
 * Inherits from Imx3duc (with 1 or 3 bands) because all
 * screens can show it.
 */
class ImageLogic : public Imx3duc {

public :

   /**
    * Creates an image from the depth and the size.
    * @param nbands The number of bands in case of the multispectral image.
    * @param depth The depth of the image.
    * @param height The heigth of the image.
    * @param width The width of the image.
    */
   ImageLogic( int nbands, long depth, long height, long width ): Imx3duc(nbands,depth,height,width) { }

   /**
    * Creates an image.
    * @param nbands The number of bands in case of the multispectral image.
    * @param size A structure that contains depth, height, width of the image.
    */
   ImageLogic( int nbands, Dimension3d size ): Imx3duc(nbands,size) {  }
};

#endif
