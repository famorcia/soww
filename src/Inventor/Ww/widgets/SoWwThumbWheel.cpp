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

/*!
  \class SoWwThumbWheel SoWwThumbWheel.h Inventor/Ww/widgets/SoWwThumbWheel.h
  \brief The SoWwThumbWheel class is a UI component for fancy looking thumbwheel controls.

  \ingroup components
*/


#include <Inventor/Ww/widgets/SoWwThumbWheel.h>
#include <Inventor/Ww/widgets/SoAnyThumbWheel.h>
#include <Inventor/SbBasic.h>
#include <Inventor/errors/SoDebugError.h>

#include "WheelEvents.h"
#include "sowwdefs.h"
#include "Inventor/Ww/viewers/ViewersWxIds.h"

#include <wx/wx.h>
#include <wx/mstream.h>

#include <cassert>
#include <cstdio>
#include <sstream>
#include <stdint.h>

wxBEGIN_EVENT_TABLE(SoWwThumbWheel, wxPanel)
                EVT_MOTION(SoWwThumbWheel::mouseMoveEvent)
                EVT_LEFT_DOWN(SoWwThumbWheel::mousePressEvent)
                EVT_LEFT_UP(SoWwThumbWheel::mouseReleaseEvent)
                EVT_MOUSEWHEEL(SoWwThumbWheel::mouseWheel)
                EVT_PAINT(SoWwThumbWheel::paintEvent)
wxEND_EVENT_TABLE()

static const int SHADEBORDERWIDTH = 0;

SoWwThumbWheel::SoWwThumbWheel(wxWindow * parent,
                               const char * name)
        : wxPanel(parent,
                  wxID_ANY) {
    this->constructor(SoWwThumbWheel::Vertical);
}

SoWwThumbWheel::SoWwThumbWheel(Orientation orientation,
                               wxWindow * parent,
                               const char * name)
        : wxPanel(parent,
                  wxID_ANY) {
    if(!name)
        this->SetName("SoWwThumbWheel");
    else
        this->SetName(name);
    this->constructor(orientation);
}

void
SoWwThumbWheel::constructor(Orientation orientation) {
    this->orient = orientation;
    this->state = SoWwThumbWheel::Idle;
    this->wheelValue = this->tempWheelValue = 0.0f;
    this->wheel = new SoAnyThumbWheel;
    this->wheel->setMovement(SoAnyThumbWheel::UNIFORM);
    this->wheel->setGraphicsByteOrder(SoAnyThumbWheel::ARGB);
    this->pixmaps = NULL;
    this->numPixmaps = 0;
    this->currentPixmap = -1;
    wxSize s = sizeHint();
    SetMinSize(s);
    SetMaxSize(s);
}

SoWwThumbWheel::~SoWwThumbWheel() {
    delete this->wheel;
    if (this->pixmaps) {
        for (int i = 0; i < this->numPixmaps; i++)
            delete this->pixmaps[i];
        delete [] this->pixmaps;
    }
}

void
SoWwThumbWheel::setOrientation(Orientation orientation) {
    this->orient = orientation;
    Refresh();
}

void
SoWwThumbWheel::paintEvent(wxPaintEvent& WXUNUSED(event)) {
    wxPaintDC dc(this);

    int w, dval;
    wxSize size= this->GetSize();
    if (this->orient == SoWwThumbWheel::Vertical) {
        w = size.GetX() - 12;
        dval = size.GetY() - 6;
    } else {
        w = size.GetY() - 12;
        dval = size.GetX() - 6;
    }

#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::paintEvent",
                              "dval: %d and w: %d",
                              dval, w);
#endif

    // Handle resizing to too small dimensions gracefully.
    if ((dval <= 0) || (w <= 0)) return;

    this->initWheel(dval, w);

    int pixmap = this->wheel->getBitmapForValue(this->tempWheelValue,
                                                (this->state == SoWwThumbWheel::Disabled) ?
                                                SoAnyThumbWheel::DISABLED : SoAnyThumbWheel::ENABLED);
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::paintEvent",
                              "pixmap value is: %d and bitmap pointer is %p",
                              pixmap, this->pixmaps);
#endif

    if(pixmap >= numPixmaps)
        return;

    wxRect widgetrect(0, 0,
                      this->GetSize().GetWidth(),
                      this->GetSize().GetHeight());
    wxRect wheelrect(widgetrect);

    if (this->orient == Vertical) {
        wheelrect.SetTop(   wheelrect.GetTop() + 2);
        wheelrect.SetBottom(wheelrect.GetBottom() - 4);
        wheelrect.SetLeft(  wheelrect.GetLeft() + 5);
        wheelrect.SetRight( wheelrect.GetRight() - 10);
    } else {
        wheelrect.SetTop(   wheelrect.GetTop() + 5);
        wheelrect.SetBottom(wheelrect.GetBottom() - 10);
        wheelrect.SetLeft(  wheelrect.GetLeft() + 2);
        wheelrect.SetRight( wheelrect.GetRight() - 4);
    }
    dc.DrawRectangle(wheelrect);

    wheelrect.SetTop(   wheelrect.GetTop() + 1);
    wheelrect.SetBottom(wheelrect.GetBottom() - 1);
    wheelrect.SetLeft(  wheelrect.GetLeft() + 1);
    wheelrect.SetRight( wheelrect.GetRight() - 1);
    // wheelrect is now wheel-only

    wxRect sRect;
    wxRect dRect;
    if (this->orient == Vertical) {
        sRect = wxRect(0,0,w,dval);
        dRect = wxRect(wheelrect.GetLeft(),wheelrect.GetTop(),w,dval);
    }
    else {
        sRect = wxRect(0,0,dval,w);
        dRect = wxRect(wheelrect.GetLeft(),wheelrect.GetTop(),dval,w);
    }

    assert(pixmap < numPixmaps);
    wxBitmap bitmap(*this->pixmaps[pixmap]);
    dc.DrawBitmap(bitmap, dRect.GetX(), dRect.GetY(), false);

    this->currentPixmap = pixmap;
}

/*!
  \internal
*/

void
SoWwThumbWheel::mousePressEvent(wxMouseEvent&  event) {
    if (this->state != SoWwThumbWheel::Idle)
        return;
    this->state = SoWwThumbWheel::Dragging;

    if (this->orient == SoWwThumbWheel::Vertical)
        this->mouseDownPos = event.GetY() - SHADEBORDERWIDTH - 6;
    else
        this->mouseDownPos = event.GetX() - SHADEBORDERWIDTH - 6;

    this->mouseLastPos = this->mouseDownPos;

#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::mouseMoveEvent","");
#endif

    sendEvent(SO_WW_MOUSE_WHEEL_PRESSED, "mousePressEvent");
}

/*!
  \internal
*/
void
SoWwThumbWheel::mouseMoveEvent(wxMouseEvent& event) {

    if (this->state != SoWwThumbWheel::Dragging)
        return;

    if (this->orient == SoWwThumbWheel::Vertical)
        this->mouseLastPos = event.GetY() - SHADEBORDERWIDTH - 6;
    else
        this->mouseLastPos = event.GetX() - SHADEBORDERWIDTH - 6;

    int delta = this->mouseLastPos - this->mouseDownPos;
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::mouseMoveEvent",
                           "delta: %d wheelValue: %d mouseDownPos: %d",
                           delta,
                           this->wheelValue,
                           this->mouseDownPos);
#endif

    this->tempWheelValue = this->wheel->calculateValue(this->wheelValue,
                                                       this->mouseDownPos,
                                                       delta);

    sendEvent(SO_WW_MOUSE_WHEEL_MOVED, "mouseWheel");
    Refresh();
}

/*!
  \internal
*/
void
SoWwThumbWheel::mouseReleaseEvent(wxMouseEvent& WXUNUSED(event)) {
    if (this->state != SoWwThumbWheel::Dragging)
        return;

    this->wheelValue = this->tempWheelValue;
    this->mouseLastPos = this->mouseDownPos;
    this->state = SoWwThumbWheel::Idle;
    sendEvent(SO_WW_MOUSE_WHEEL_RELEASED, "mouseReleaseEvent");
}

void
SoWwThumbWheel::mouseWheel(wxMouseEvent &event) {
    SOWW_STUB();
    return;
    int delta = /*event.GetWheelDelta() * */(float)(event.GetWheelRotation()) / 120.0;
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::mouseWheel",
                           "delta: %d wheelValue: %d mouseDownPos: %d",
                           delta,
                           this->wheelValue,
                           this->mouseDownPos);
#endif

    this->tempWheelValue = this->wheel->calculateValue(this->wheelValue,
                                                       this->mouseDownPos,
                                                       delta);
    Refresh();
    sendEvent(SO_WW_MOUSE_WHEEL_MOVED,
              "mouseWheel");
}

/*
float
SoWwThumbWheel::getNormalizedValue(int pos) const
{
  int relativepos = pos - this->mouseDownPos;
  return (float) relativepos / (float)this->getWheelLength() * 2.0f;
}
*/

/*
int
SoWwThumbWheel::getWheelLength() const
{
  return this->orient == SoWwThumbWheel::Vertical ?
    this->GetSize().GetY() : this->GetSize().GetX();
}
*/

/*
int
SoWwThumbWheel::orientedCoord(const QPoint &p) const
{
  return (this->orient == SoWwThumbWheel::Horizontal) ?  p.x() : p.y();
}
*/

wxSize
SoWwThumbWheel::sizeHint() const {
    const int length = 122;
    int thick = 24;

    if (this->orient == SoWwThumbWheel::Horizontal)
        return wxSize(length, thick);
    else
        return wxSize(thick, length);
}

SoWwThumbWheel::Orientation
SoWwThumbWheel::orientation() const {
    return this->orient;
}

float
SoWwThumbWheel::value() const {
    return this->wheelValue;
}

void
fill(std::vector<uint8_t>& buffer,
          unsigned long n,
          int channel = 3) {
    if(channel>3) {
        buffer.push_back((n >> 24) & 0xFF);
    }
    buffer.push_back((n >> 16) & 0xFF);
    buffer.push_back((n >> 8) & 0xFF);
    buffer.push_back(n & 0xFF);
}

uint8_t*
toRGBChannel(const std::vector<unsigned int>& img) {
    std::vector<uint8_t> vout;
    for(size_t i=0;i<img.size();++i) {
        fill(vout, img[i]);
    }
    assert(vout.size() == img.size()*3);
    uint8_t* out = static_cast<uint8_t*>(malloc( vout.size() ));
    memcpy(out, &vout[0], vout.size());
    return (out);
}

void
SoWwThumbWheel::initWheel(int diameter, int width) {
    int dval, w;
    this->wheel->getSize(dval, w);
    if (dval == diameter && w == width) return;

    this->wheel->setSize(diameter, width);

    int pwidth = width;
    int pheight = diameter;
    if (this->orient == Horizontal) {
        pwidth = diameter;
        pheight = width;
    }

    if (this->pixmaps != NULL) {
        for (int i = 0; i < this->numPixmaps; i++)
            delete this->pixmaps[i];
        delete [] this->pixmaps;
    }

    this->numPixmaps = this->wheel->getNumBitmaps();
    this->pixmaps = new wxBitmap * [this->numPixmaps];

    for (int i = 0; i < this->numPixmaps; i++) {
        std::vector<unsigned int> buffer(pwidth*pheight);
        this->wheel->drawBitmap(i, &buffer[0],
                                (this->orient == Vertical) ? SoAnyThumbWheel::VERTICAL :
                                SoAnyThumbWheel::HORIZONTAL);
        uint8_t* rgb = toRGBChannel(buffer);
        wxImage img(pwidth, pheight,rgb);
        assert(img.IsOk());
        this->pixmaps[i] = new wxBitmap(img);
    }
}

void
SoWwThumbWheel::setEnabled(bool enable) {
    if (enable)
        this->state = SoWwThumbWheel::Idle;
    else
        this->state = SoWwThumbWheel::Disabled;
    Refresh();
}

bool
SoWwThumbWheel::isEnabled() const {
    return (this->state != SoWwThumbWheel::Disabled);
}

void
SoWwThumbWheel::setValue(float value) {
    this->wheelValue = this->tempWheelValue = value;
    this->mouseDownPos = this->mouseLastPos;
    Refresh();
}

void
SoWwThumbWheel::setRangeBoundaryHandling(boundaryHandling handling) {
    switch (handling) {
        case CLAMP:
            this->wheel->setBoundaryHandling(SoAnyThumbWheel::CLAMP);
            break;
        case MODULATE:
            this->wheel->setBoundaryHandling(SoAnyThumbWheel::MODULATE);
            break;
        case ACCUMULATE:
            this->wheel->setBoundaryHandling(SoAnyThumbWheel::ACCUMULATE);
            break;
        default:
            assert(0 && "impossible");
    }
}

SoWwThumbWheel::boundaryHandling
SoWwThumbWheel::getRangeBoundaryHandling() const {
    switch (this->wheel->getBoundaryHandling()) {
        case SoAnyThumbWheel::CLAMP:
            return CLAMP;
        case SoAnyThumbWheel::MODULATE:
            return MODULATE;
        case SoAnyThumbWheel::ACCUMULATE:
            return ACCUMULATE;
        default:
            assert(0 && "impossible");
    }
    return CLAMP; // never reached
}

void
SoWwThumbWheel::sendEvent(long id,
                          const std::string& event_id) {
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwThumbWheel::sendEvent",
                           "id: %d event: %s tempWheelValue: %d",
                           id,
                           event_id.c_str(),
                           this->tempWheelValue);
#endif

    wxCommandEvent a_wx_event(id, GetId());
    a_wx_event.SetEventObject(this);
    a_wx_event.SetString(event_id);
    a_wx_event.SetClientData(&this->tempWheelValue);
    ProcessWindowEvent(a_wx_event);
}
