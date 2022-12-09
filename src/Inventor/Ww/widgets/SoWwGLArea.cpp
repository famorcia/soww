/**************************************************************************\
 * BSD 3-Clause License
 *
 * Copyright (c) 2022, Fabrizio Morciano
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
\**************************************************************************/

#include "Inventor/Ww/widgets/SoWwGLArea.h"
#include "Inventor/Ww/SoWwGLWidgetP.h"

#include "wx/wx.h"
#include "wx/file.h"
#include "wx/dcclient.h"
#include "wx/wfstream.h"

#include <GL/gl.h>
#include <GL/glu.h>

#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/engines/SoElapsedTime.h>
#include <Inventor/events/SoMouseButtonEvent.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoEventCallback.h>
#include <Inventor/nodes/SoMaterial.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoRotationXYZ.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/SbViewportRegion.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/SoInteraction.h>

wxBEGIN_EVENT_TABLE(SoWwGLArea, wxGLCanvas)
                EVT_SIZE(SoWwGLArea::OnSize)
                EVT_PAINT(SoWwGLArea::OnPaint)
                EVT_ERASE_BACKGROUND(SoWwGLArea::OnEraseBackground)
wxEND_EVENT_TABLE()


SoWwGLArea::SoWwGLArea(SoWwGLWidgetP* aGLWidget,
                       wxWindow *parent,
                       wxGLAttributes& attributes,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
        : wxGLCanvas(parent,
                     attributes,
                     id,
                     pos,
                     size,
                     style | wxFULL_REPAINT_ON_RESIZE,
                     name)
{
    wwGlWidget = aGLWidget;
    glRealContext = 0;
    // Explicitly create a new rendering context instance for this canvas.
    isGLInitialized = false;
}

SoWwGLArea::~SoWwGLArea() {
    delete glRealContext;
}

void SoWwGLArea::OnPaint(wxPaintEvent& event )
{
    std::clog<<__PRETTY_FUNCTION__<<std::endl;

    // must always be here
    wxPaintDC dc(this);

    InitGL();
    wwGlWidget->gl_exposed();
}

void SoWwGLArea::OnSize(wxSizeEvent& event)
{
    std::clog<<__PRETTY_FUNCTION__<<std::endl;

    // on size Coin need to know the new view port
    wwGlWidget->gl_reshape(event.GetSize().x,
                           event.GetSize().y);
    wwGlWidget->gl_changed();
}

void SoWwGLArea::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    std::clog<<__PRETTY_FUNCTION__<<std::endl;
    // Do nothing, to avoid flashing on MSW
}

void SoWwGLArea::InitGL()
{
    std::clog<<__PRETTY_FUNCTION__<<std::endl;
    if(!isGLInitialized) {
        glRealContext = new wxGLContext(this);
        SetCurrent(*glRealContext);
        isGLInitialized = true;
        wwGlWidget->gl_init();
    } else {
        SetCurrent(*glRealContext);
    }
}

void SoWwGLArea::makeCurrent() {
    if(glRealContext)
        SetCurrent(*glRealContext);
}

const wxGLContext *SoWwGLArea::context() {
    return glRealContext;
}
