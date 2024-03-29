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

#include <Inventor/Ww/viewers/SoWwFullViewer.h>
#include "Inventor/Ww/viewers/SoWwFullViewerP.h"
#include "Inventor/Ww/widgets/SoWwThumbWheel.h"
#include <Inventor/Ww/widgets/SoWwPopupMenu.h>
#include "sowwdefs.h"
#include "Inventor/Ww/SoWwP.h"
#include "ButtonIndexValues.h"

// Button icons.
#include <Inventor/Ww/common/pixmaps/pick.xpm>
#include <Inventor/Ww/common/pixmaps/view.xpm>
#include <Inventor/Ww/common/pixmaps/home.xpm>
#include <Inventor/Ww/common/pixmaps/set_home.xpm>
#include <Inventor/Ww/common/pixmaps/view_all.xpm>
#include <Inventor/Ww/common/pixmaps/seek.xpm>

#include <wx/stattext.h>
#include <wx/sizer.h>

#define PUBLIC(o) (o->pub)
#define PRIVATE(o) (o->pimpl)

SOWW_OBJECT_ABSTRACT_SOURCE(SoWwFullViewer);

const int XPM_BUTTON_SIZE = 24;

SoWwFullViewer::SoWwFullViewer(wxWindow* parent,
                               const char * name,
                               SbBool embed,
                               BuildFlag buildFlag,
                               Type type,
                               SbBool build)
        : inherited(parent,
                    name,
                    embed,
                    type,
                    FALSE)
{
    PRIVATE(this) = new SoWwFullViewerP(this);

    PRIVATE(this)->viewerwidget = NULL;
    PRIVATE(this)->canvas = NULL;

    PRIVATE(this)->viewbutton = NULL;
    PRIVATE(this)->interactbutton = NULL;

    this->leftDecoration = NULL;
    this->bottomDecoration = NULL;
    this->rightDecoration = NULL;

    this->leftWheel = NULL;
    this->leftWheelLabel = NULL;
    this->leftWheelStr = NULL;
    this->leftWheelVal = 0.0f;

    this->bottomWheel = NULL;
    this->bottomWheelLabel = NULL;
    this->bottomWheelStr = NULL;
    this->bottomWheelVal = 0.0f;

    this->rightWheel = NULL;
    this->rightWheelLabel = NULL;
    this->rightWheelStr = NULL;
    this->rightWheelVal = 0.0f;

    this->setLeftWheelString("Motion X");
    this->setBottomWheelString("Motion Y");
    this->setRightWheelString("Dolly");

    PRIVATE(this)->mainlayout = NULL;
    PRIVATE(this)->appbuttonlayout = NULL;

    PRIVATE(this)->menuenabled = buildFlag & SoWwFullViewer::BUILD_POPUP;
    PRIVATE(this)->decorations = (buildFlag & SoWwFullViewer::BUILD_DECORATION) ? TRUE : FALSE;

    this->prefmenu = NULL;
    PRIVATE(this)->menutitle = "Viewer Menu";

    PRIVATE(this)->viewerbuttons = new SbPList;
    PRIVATE(this)->appbuttonlist = new SbPList;
    PRIVATE(this)->appbuttonform = NULL;

    if (! build) return;

    this->setClassName("SoWwFullViewer");
    wxWindow * viewer = this->buildWidget(this->getParentWidget());
    this->setBaseWidget(viewer);
}

SoWwFullViewer::~SoWwFullViewer() {
    delete PRIVATE(this)->viewerbuttons;
    delete PRIVATE(this)->appbuttonlist;
    delete [] this->rightWheelStr;
    delete [] this->leftWheelStr;
    delete [] this->bottomWheelStr;
    delete PRIVATE(this);
}

wxWindow*
SoWwFullViewer::buildWidget(wxWindow* parent) {
    // This will build the main view widgets, along with the decorations
    // widgets and popup menu if they are enabled.
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwFullViewer::buildWidget", "[invoked]");
    parent->SetName("MainWindow");
    SoDebugError::postInfo("SoWwFullViewer::buildWidget", "Step-1");
    SoWwP::dumpWindowData(parent);
#endif

    PRIVATE(this)->viewerwidget = parent;
    PRIVATE(this)->viewerwidget->SetName("viewerwidget");

    this->registerWidget(PRIVATE(this)->viewerwidget);

#if SOWW_DEBUG && 0
    PRIVATE(this)->viewerwidget->SetBackgroundColour(wxColour(125, 150, 190));
#endif

    PRIVATE(this)->canvas = inherited::buildWidget(PRIVATE(this)->viewerwidget);
    PRIVATE(this)->canvas->SetMinSize(wxSize(100,100));

#if SOWW_DEBUG && 0
    PRIVATE(this)->canvas->SetBackgroundColour(wxColour(250, 0, 255));
#endif

    this->buildDecoration( PRIVATE(this)->viewerwidget );
    PRIVATE(this)->showDecorationWidgets( PRIVATE(this)->decorations );

    if (PRIVATE(this)->menuenabled)
        this->buildPopupMenu();

#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwFullViewer::buildWidget", "Step-2");
    SoWwP::dumpWindowData(parent);
#endif

    PRIVATE(this)->bindEvents(PRIVATE(this)->viewerwidget);

    return PRIVATE(this)->viewerwidget;
}


void
SoWwFullViewer::setDecoration(const SbBool enable){
#if SOWW_DEBUG && 0
    if ((enable  && this->isDecoration()) ||
        (!enable && !this->isDecoration())) {
        SoDebugError::postWarning("SoWwFullViewer::setDecoration",
                                  "decorations already turned %s",
                                  enable ? "on" : "off");
        return;
    }
#endif

    PRIVATE(this)->decorations = enable;
    if (PRIVATE(this)->viewerwidget)
        PRIVATE(this)->showDecorationWidgets(enable);
}

SbBool
SoWwFullViewer::isDecoration(void) const{
    return (PRIVATE(this)->decorations);
}

void SoWwFullViewer::setPopupMenuEnabled(const SbBool enable){
#if SOWW_DEBUG
    if ((enable && this->isPopupMenuEnabled()) ||
       (!enable && !this->isPopupMenuEnabled())) {
    SoDebugError::postWarning("SoWwFullViewer::setPopupMenuEnabled",
                              "popup menu already turned %s",
                              enable ? "on" : "off");
    return;
  }
#endif
    PRIVATE(this)->menuenabled = enable;
}

SbBool
SoWwFullViewer::isPopupMenuEnabled(void) const{
    return (PRIVATE(this)->menuenabled);
}

wxWindow*
SoWwFullViewer::getAppPushButtonParent(void) const {
    SOWW_STUB();
    return (0);
}

void
SoWwFullViewer::addAppPushButton(wxWindow* newButton)  {
    SOWW_STUB();
}

void
SoWwFullViewer::insertAppPushButton(wxWindow* newButton, int index) {
    SOWW_STUB();
}

void
SoWwFullViewer::removeAppPushButton(wxWindow* oldButton) {
    SOWW_STUB();
}

int
SoWwFullViewer::findAppPushButton(wxWindow* oldButton) const {
    SOWW_STUB();
    return (0);
}

int
SoWwFullViewer::lengthAppPushButton(void) const {
    SOWW_STUB();
    return (0);
}

wxWindow*
SoWwFullViewer::getRenderAreaWidget(void) const {
    return (PRIVATE(this)->canvas);
}

void
SoWwFullViewer::setViewing(SbBool enable) {
    if ((enable && this->isViewing()) ||
        (!enable && !this->isViewing())) {
#if SOWW_DEBUG && 0
        SoDebugError::postWarning("SoWwFullViewer::setViewing",
                              "view mode already %s", enable ? "on" : "off");
#endif
        return;
    }

    inherited::setViewing(enable);

    // Must check that buttons have been built, in case this viewer
    // component was made without decorations.
    if (PRIVATE(this)->viewerbuttons->getLength() > 0) {
        ((wxToggleButton*)(PRIVATE(this))->getViewerbutton(EXAMINE_BUTTON))->SetValue(enable);
        ((wxToggleButton*)(PRIVATE(this))->getViewerbutton(INTERACT_BUTTON))->SetValue(enable ? FALSE : TRUE);
        ((wxButton*)PRIVATE(this)->getViewerbutton(SEEK_BUTTON))->Enable(enable);
    }
}


void
SoWwFullViewer::buildDecoration(wxWindow* parent) {
    this->leftDecoration = this->buildLeftTrim(parent);
#if SOWW_DEBUG && 0
    this->leftDecoration->SetBackgroundColour(wxColour(255, 0, 0));
#endif
    this->bottomDecoration = this->buildBottomTrim(parent);
#if SOWW_DEBUG && 0
    this->bottomDecoration->SetBackgroundColour(wxColour(0, 255, 0));
#endif
    this->rightDecoration = this->buildRightTrim(parent);
#if SOWW_DEBUG && 0
    this->rightDecoration->SetBackgroundColour(wxColour(0, 0, 255));
#endif

#if SOWW_DEBUG && 0
    SoWwP::dumpWindowData(this->leftDecoration);
    SoWwP::dumpWindowData(this->rightDecoration);
    SoWwP::dumpWindowData(this->bottomDecoration);
#endif

    PRIVATE(this)->initThumbWheelEventMap();
}

wxWindow*
SoWwFullViewer::buildLeftTrim(wxWindow* parent){
    wxPanel* p = new wxPanel(parent);
    p->SetName("leftTrim");
    p->SetMinSize(wxSize(24,100));

#if SOWW_DEBUG && 0
    p->SetBackgroundColour(wxColour(255, 0, 255));
#endif

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SoWwThumbWheel * t = new SoWwThumbWheel(SoWwThumbWheel::Vertical, p);
    t->SetName("left thumb wheel");
    t->setRangeBoundaryHandling(SoWwThumbWheel::ACCUMULATE);
    this->leftWheelVal = t->value();
    this->leftWheel = t;
    sizer->Add(0,0,1,0);
    sizer->Add(t, 0, wxALL |wxALIGN_CENTER_HORIZONTAL, 0);
    p->SetSizer(sizer);
    p->Fit();
#if SOWW_DEBUG && 0
    SoWwP::dumpWindowData(p);
#endif
    return p;
}

wxWindow*
SoWwFullViewer::buildBottomTrim(wxWindow* parent) {
    wxWindow * w = new wxPanel(parent);
    w->SetName("bottomTrim");
    w->SetMinSize(wxSize(100,24));
    wxStaticText* label = new wxStaticText( w, wxID_ANY, this->leftWheelStr);
#if SOWW_DEBUG && 0
    label->SetBackgroundColour(wxColour(200,200,0));
#endif
    label->SetName("left wheel label");
    this->leftWheelLabel = label;

    label = new wxStaticText( w, wxID_ANY, this->bottomWheelStr ,wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
#if SOWW_DEBUG && 0
    label->SetBackgroundColour(wxColour(100,100,0));
#endif
    label->SetName("bottom wheel label");
    this->bottomWheelLabel = label;

    label = new wxStaticText( w, wxID_ANY, this->rightWheelStr, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT );
#if SOWW_DEBUG && 0
    label->SetBackgroundColour(wxColour(100,50,0));
#endif
    label->SetName("right wheel label");
    this->rightWheelLabel = label;

    SoWwThumbWheel * t = new SoWwThumbWheel(SoWwThumbWheel::Horizontal, w);
    t->SetName("bottom thumb wheel");
#if SOWW_DEBUG && 0
    t->SetBackgroundColour(wxColour(0,0,0));
#endif

    this->bottomWheel = t;
    t->setRangeBoundaryHandling(SoWwThumbWheel::ACCUMULATE);

    this->bottomWheelVal = t->value();

    wxFlexGridSizer* layout = new wxFlexGridSizer( 0, 6, 0, 5 );

    layout->AddGrowableCol( 1 );
    layout->AddGrowableCol( 4 );
    layout->Add( this->leftWheelLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    layout->AddStretchSpacer();
    layout->Add( this->bottomWheelLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    layout->Add( this->bottomWheel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);
    layout->AddStretchSpacer();
    layout->Add( this->rightWheelLabel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

    w->SetSizer( layout );
    w->Fit();
    return w;
}

wxWindow*
SoWwFullViewer::buildRightTrim(wxWindow* parent) {
    wxPanel* p = new wxPanel(parent);
    p->SetName("rightTrim");
#if SOWW_DEBUG && 0
    p->SetBackgroundColour(wxColour(255,0,0));
#endif
    p->SetMinSize(wxSize(XPM_BUTTON_SIZE+12, 100));
    p->SetMaxSize(wxSize(XPM_BUTTON_SIZE+12, -1));
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    SoWwThumbWheel * t = new SoWwThumbWheel(SoWwThumbWheel::Vertical, p);
    t->SetName("right thumb wheel");
#if SOWW_DEBUG && 0
    t->SetBackgroundColour(wxColour(100,250,110));
#endif
    t->setRangeBoundaryHandling(SoWwThumbWheel::ACCUMULATE);
    this->rightWheelVal = t->value();
    this->rightWheel = t;
    const int border_size = 0;
    sizer->Add(this->buildViewerButtons(p),  1, wxALL | wxCENTER, border_size);
    sizer->Add( 0, 0, 1, wxEXPAND, border_size);
    sizer->Add(t, 1, wxALL| wxCENTER, border_size);
    p->SetSizer(sizer);
    p->Layout();
    return p;
}

wxWindow*
SoWwFullViewer::buildAppButtons(wxWindow* parent) {
    SOWW_STUB();
    return (0);
}

wxWindow*
SoWwFullViewer::buildViewerButtons(wxWindow* parent) {
    wxPanel * w = new wxPanel(parent);
    w->SetName("viewerButtons");
#if SOWW_DEBUG && 0
    w->SetBackgroundColour(wxColour(250,100,250));
#endif
    this->createViewerButtons(w, PRIVATE(this)->viewerbuttons);

    const int numViewerButtons = PRIVATE(this)->viewerbuttons->getLength();
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    for (int i = 0; i < numViewerButtons; i++) {
        wxAnyButton * b = PRIVATE(this)->getViewerbutton(i);
        sizer->Add( b, 0, wxALL | wxCENTER, 0);
    }
    w->SetSizer(sizer);
    return (w);
}

void
SoWwFullViewer::createViewerButtons(wxWindow* parent,
                                    SbPList * buttonlist) {
    for (int i=0; i <= SEEK_BUTTON; i++) {
        wxAnyButton *p = new wxButton(parent, i);

        switch (i) {
            case INTERACT_BUTTON: {
                parent->RemoveChild(p);
                delete p;
                PRIVATE(this)->interactbutton = new wxToggleButton(parent, i, wxEmptyString);
                p = PRIVATE(this)->interactbutton;
                p->SetBitmap(wxImage(pick_xpm));
                p->SetName("INTERACT");
                PRIVATE(this)->interactbutton->SetValue(this->isViewing() ? FALSE : TRUE);
            }

                break;
            case EXAMINE_BUTTON: {
                parent->RemoveChild(p);
                delete p;
                PRIVATE(this)->viewbutton = new wxToggleButton(parent, i, wxEmptyString);
                p = PRIVATE(this)->viewbutton;
                PRIVATE(this)->viewbutton->SetValue(this->isViewing());
                p->SetName("EXAMINE");
                p->SetBitmap(wxImage(view_xpm));
            }
                break;
            case HOME_BUTTON: {
                p->SetBitmap(wxImage(home_xpm));
                p->SetName("HOME");
            }
                break;
            case SET_HOME_BUTTON: {
                p->SetBitmap(wxImage(set_home_xpm));
                p->SetName("SET_HOME");
            }
                break;
            case VIEW_ALL_BUTTON: {
                p->SetBitmap(wxImage (view_all_xpm));
                p->SetName("VIEW_ALL");
            }
                break;
            case SEEK_BUTTON: {
                p->SetBitmap(wxImage(seek_xpm));
                p->SetName("SEEK");
            }
                break;
            default:
                assert(0);
                break;
        }

        p->SetWindowStyle( wxBU_EXACTFIT | wxBU_NOTEXT);
        p->SetSize(XPM_BUTTON_SIZE, XPM_BUTTON_SIZE);
        buttonlist->append(p);
    }
}

void
SoWwFullViewer::buildPopupMenu(void) {
    this->prefmenu = PRIVATE(this)->setupStandardPopupMenu();
}

void
SoWwFullViewer::openPopupMenu(const SbVec2s position) {
    if (! this->isPopupMenuEnabled()) return;
    if (this->prefmenu == NULL)
        this->buildPopupMenu();
    int x = 2 + position[0];
    int y = 2 + this->getGLSize()[1] - position[1] - 1;

    PRIVATE(this)->prepareMenu(this->prefmenu);
    this->prefmenu->popUp(this->getGLWidget(), x, y);
}

void
SoWwFullViewer::setLeftWheelString(const char * const name) {
    delete [] this->leftWheelStr;
    this->leftWheelStr = NULL;

    if (name)
        this->leftWheelStr = strcpy(new char [strlen(name)+1], name);
    if (this->leftWheelLabel)
        this->leftWheelLabel->SetLabel(name ? name : "");
}

void
SoWwFullViewer::setBottomWheelString(const char * const name) {
    delete [] this->bottomWheelStr;
    this->bottomWheelStr = NULL;

    if (name)
        this->bottomWheelStr = strcpy(new char [strlen(name)+1], name);
    if (this->bottomWheelLabel)
        this->leftWheelLabel->SetLabel(name ? name : "");

}

void
SoWwFullViewer::setRightWheelString(const char * const name) {
    delete [] this->rightWheelStr;
    this->rightWheelStr = NULL;

    if (name)
        this->rightWheelStr = strcpy(new char [strlen(name)+1], name);

    if (this->rightWheelLabel) {
        this->rightWheelLabel->SetLabel(name ? name : "");
    }
}

void
SoWwFullViewer::sizeChanged(const SbVec2s & size) {
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwFullViewer::sizeChanged", "(%d, %d)",
                         size[0], size[1]);
#endif

    SbVec2s newsize(size);
    // wxWidgets already has only size of gl area
    // decorations size do not need to be removed
    /*
     if (PRIVATE(this)->decorations) {
        newsize[0] -= width(this->leftDecoration);
        newsize[0] -= width(this->rightDecoration);
        newsize[1] -= height(this->bottomDecoration);
    }
    newsize = SbVec2s(SoWwMax(newsize[0], (short)1),
                      SoWwMax(newsize[1], (short)1));
*/

    inherited::sizeChanged(newsize);
}

const char *
SoWwFullViewer::getRightWheelString() const {
    return (this->rightWheelStr);
}

#undef PUBLIC
#undef PRIVATE
