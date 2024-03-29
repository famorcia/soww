/*
 *
 *  Copyright (C) 2000 Silicon Graphics, Inc.  All Rights Reserved. 
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  Further, this software is distributed without any warranty that it is
 *  free of the rightful claim of any third person regarding infringement
 *  or the like.  Any license provided herein, whether implied or
 *  otherwise, applies only to this software file.  Patent licenses, if
 *  any, provided herein do not apply to combinations of this program with
 *  other software, or any other product whatsoever.
 * 
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 *  Mountain View, CA  94043, or:
 * 
 *  http://www.sgi.com 
 * 
 *  For further information regarding this notice, see: 
 * 
 *  http://oss.sgi.com/projects/GenInfo/NoticeExplan/
 *
 */

/*------------------------------------------------------------
 *  This is an example from the Inventor Mentor,
 *  chapter 16, example 1.
 * 
 *  This example shows how to use the overlay planes with the
 *  viewer components. By default color 0 is used for the
 *  overlay planes background color (clear color), so we use
 *  color 1 for the object. This example also shows how to
 *  load the overlay color map with the wanted color.
 *----------------------------------------------------------*/


#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/nodes/SoNode.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/Ww/SoWw.h>
#include <Inventor/Ww/viewers/SoWwExaminerViewer.h>
#ifdef sun
#include <Inventor/nodes/SoColorIndex.h>
#include <Inventor/actions/SoSearchAction.h>
#endif

#ifdef sun

#ifdef __cplusplus
eWwern "C" {
#endif
eWwern int glXIsOverlayTGS(void);
#ifdef __cplusplus
}
#endif

# pragma weak           glXIsOverlayTGS
#endif


static const char *overlayScene = "\
#Inventor V2.0 ascii\n\
\
Separator { \
   OrthographicCamera { \
      position 0 0 5 \
      nearDistance 1.0 \
      farDistance 10.0 \
      height 10 \
   } \
   LightModel { model BASE_COLOR } \
   ColorIndex { index 1 } \
   Coordinate3 { point [ -1 -1 0, -1 1 0, 1 1 0, 1 -1 0] } \
   FaceSet {} \
} ";

int
main(int , char **argv)
{
   // Initialize Inventor and Ww
   wxWindow* myWindow = SoWw::init(argv[0]);

   // read the scene graph in
   SoInput in;
   SoNode *scene;
   in.setBuffer((void *)overlayScene, (size_t) strlen(overlayScene));
   if (! SoDB::read(&in, scene) || scene == NULL) {
      printf("Couldn't read scene\n");
      exit(1);
   }

   // Allocate the viewer, set the overlay scene and
   // load the overlay color map with the wanted color.
   SbColor color(.5, 1, .5);
   SoWwExaminerViewer *myViewer = new SoWwExaminerViewer(myWindow);

#ifdef sun
   if (!glXIsOverlayTGS)
   {
     long value;
     int num;
                   // Get overlay colormap size
     int colorMapSize = myViewer->getOverlayColorMapSize();

                   // Code which gets a handle to the SoColorIndex node
     SoSearchAction searchAction;
     searchAction.setType(SoColorIndex::getClassTypeId());
     searchAction.setInterest(SoSearchAction::FIRST);
     scene->ref();
     searchAction.apply(scene);
     SoPath *path = searchAction.getPath();
     if (path)
     {
       int zi;
       for (zi=0;zi<path->getLength();zi++)
       {
         SoNode *child;
         SoType type1;
 
         child = path->getNode(zi);
         type1 = child->getTypeId();
         if (type1 == SoColorIndex::getClassTypeId())
         {
           num = ((SoColorIndex *)child)->index.getNum();
           value = ((SoColorIndex *)child)->index[0];

                   // Modify the index to the high end of the colormap
           ((SoColorIndex *)child)->index.set1Value(0,
           colorMapSize-1-value);
         }
       }
     }
                   // Set the overlay colormap with the color we want
     myViewer->setOverlayColorMap(colorMapSize-1-value, 1, &color);
   }
   else
     myViewer->setOverlayColorMap(1, 1, &color);
#else
   myViewer->setOverlayColorMap(1, 1, &color);
#endif

   myViewer->setSceneGraph(new SoCone);
   myViewer->setOverlaySceneGraph(scene);
   myViewer->setTitle("Overlay Plane");

   // Show the viewer and loop forever
   myViewer->show();
   //WwRealizeWidget(myWindow);
   SoWw::mainLoop();
   SoWw::done();
   return 0;
}
