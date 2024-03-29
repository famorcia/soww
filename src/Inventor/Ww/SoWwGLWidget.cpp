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

#include "Inventor/Ww/SoWwGLWidget.h"
#include "Inventor/Ww/SoWwGLWidgetP.h"
#include "Inventor/Ww/widgets/SoWwGLArea.h"
#include "sowwdefs.h"
#include "Inventor/Ww/SoAny.h"

SOWW_OBJECT_ABSTRACT_SOURCE(SoWwGLWidget);

#define PRIVATE(obj) ((obj)->pimpl)
#define PUBLIC(obj) ((obj)->pub)

SoWwGLWidget::SoWwGLWidget(wxWindow* const parent ,
                           const char * const name ,
                           const SbBool embed ,
                           const int glmodes ,
                           const SbBool build):
        inherited(parent, name, embed),
        waitForExpose(true),
        drawToFrontBuffer(false) {

    PRIVATE(this) = new SoWwGLWidgetP(this);
    PRIVATE(this)->initGLModes(glmodes);

    PRIVATE(this)->glSize = SbVec2s(0, 0);
    PRIVATE(this)->glSizeUnscaled = SbVec2s(0, 0);
    PRIVATE(this)->wasresized = false;

    PRIVATE(this)->glparent = NULL;
    PRIVATE(this)->currentglwidget = NULL;
    PRIVATE(this)->previousglwidget = NULL;
    PRIVATE(this)->currentglarea = NULL;
    PRIVATE(this)->previousglarea = NULL;

    if (! build) {
        return;
    }

    this->setClassName("SoWwGLWidget");
    wxWindow* parent_widget = this->getParentWidget();
    wxWindow* widget = this->buildWidget(parent_widget);
    this->setBaseWidget(widget);
}

SoWwGLWidget::~SoWwGLWidget() {

    if (PRIVATE(this)->currentglwidget) {
        SoAny::si()->unregisterGLContext((void *)this);
    }

    delete PRIVATE(this);
}

void
SoWwGLWidget::setBorder(const SbBool enable) {
    SOWW_STUB();
}

void
SoWwGLWidget::setQuadBufferStereo(const SbBool enable) {
    SOWW_STUB();
}

SbBool
SoWwGLWidget::isQuadBufferStereo(void) const {
    return (FALSE);
}

void
SoWwGLWidget::setGLSize(const SbVec2s size){
    if (size == PRIVATE(this)->glSize) return;
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwGLWidget::setGLSize",
                           "[invoked (%d, %d)]", size[0], size[1]);
#endif // debug
    PRIVATE(this)->glSize = size;
    PRIVATE(this)->glSizeUnscaled = size;
    if (PRIVATE(this)->currentglwidget) {
        int frame = this->isBorder() ? PRIVATE(this)->borderthickness : 0;
        // PRIVATE(this)->currentglwidget->SetSize( size[0], size[1] );
    }
}

template <typename T>
T
toSbVec2(const wxSize& wx_size) {
    return (T(wx_size.GetX(), wx_size.GetY()));
}

SbVec2s
SoWwGLWidget::getGLSize(void) const{
    return (PRIVATE(this)->glSize);
}

float
SoWwGLWidget::getGLAspectRatio(void) const{
    SbVec2f v2(toSbVec2<SbVec2f>(PRIVATE(this)->currentglwidget->GetSize()));
    return ( v2[0] /v2[1] );
}

SbBool
SoWwGLWidget::isRGBMode(void){
    const bool rgb_mode = SoWwGLArea::isGLFeatureAvailable(PRIVATE(this)->gl_attributes,
                                                                WX_GL_RGBA);
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwGLWidget::isRGBMode",
                           ": %d",
                           rgb_mode);
#endif
    return (rgb_mode);
}

void
SoWwGLWidget::glLockNormal(void){
    assert(PRIVATE(this)->currentglarea != NULL);
    PRIVATE(this)->currentglarea->makeCurrent();
}

void
SoWwGLWidget::glUnlockNormal(void){
    // do nothing
}

void
SoWwGLWidget::glLockOverlay(void){
    SOWW_STUB();
}

void
SoWwGLWidget::glUnlockOverlay(void){
    SOWW_STUB();
}

void
SoWwGLWidget::glSwapBuffers(void){
    PRIVATE(this)->currentglarea->SwapBuffers();
}

void
SoWwGLWidget::glFlushBuffer(void){
    glFlush();
}

SbBool
SoWwGLWidget::glScheduleRedraw(void){
    SOWW_STUB();
    return (TRUE);
}

SbBool
SoWwGLWidget::isBorder(void) const{
    return (PRIVATE(this)->borderthickness ? true : false);
}

void
SoWwGLWidget::setDoubleBuffer(const SbBool enable){
    SOWW_STUB();
}

SbBool
SoWwGLWidget::isDoubleBuffer(void) const{
    const bool double_buffer = SoWwGLArea::isGLFeatureAvailable(PRIVATE(this)->gl_attributes,
                                                                WX_GL_DOUBLEBUFFER);
#if SOWW_DEBUG && 0
    SoDebugError::postInfo("SoWwGLWidget::isDoubleBuffer",
                           ": %d",
                           double_buffer);
#endif
    return (double_buffer);
}

void
SoWwGLWidget::setDrawToFrontBufferEnable(const SbBool enable){
    drawToFrontBuffer = enable;
}

SbBool
SoWwGLWidget::isDrawToFrontBufferEnable(void) const{
    return (drawToFrontBuffer);
}

void
SoWwGLWidget::setAccumulationBuffer(const SbBool enable){
    SOWW_STUB();
}

SbBool
SoWwGLWidget::getAccumulationBuffer(void) const{
    SOWW_STUB();
    return (FALSE);
}

void
SoWwGLWidget::setStencilBuffer(const SbBool enable){
    SOWW_STUB();
}

SbBool
SoWwGLWidget::getStencilBuffer(void) const{
    SOWW_STUB();
    return (FALSE);
}

void
SoWwGLWidget::setAlphaChannel(const SbBool enable){
    SOWW_STUB();
}

SbBool
SoWwGLWidget::getAlphaChannel(void) const{
    SOWW_STUB();
    return (FALSE);
}

void
SoWwGLWidget::setOverlayRender(const SbBool onoff){
    SOWW_STUB();
}

SbBool
SoWwGLWidget::isOverlayRender(void) const{
    SOWW_STUB();
    return (FALSE);
}

void
SoWwGLWidget::setSampleBuffers(const int numsamples){
    SOWW_STUB();
}

int
SoWwGLWidget::getSampleBuffers(void) const{
    SOWW_STUB();
    return (0);
}

wxWindow*
SoWwGLWidget::getGLWidget(void) const{
    return (PRIVATE(this)->currentglwidget);
}

wxWindow*
SoWwGLWidget::getNormalWidget(void) const{
    SOWW_STUB();
    return (0);
}

wxWindow*
SoWwGLWidget::getOverlayWidget(void) const{
    SOWW_STUB();
    return (0);
}

SbBool
SoWwGLWidget::hasOverlayGLArea(void) const{
    SOWW_STUB();
    return (false);
}

SbBool
SoWwGLWidget::hasNormalGLArea(void) const{
    bool res = (PRIVATE(this)->currentglarea->context() != 0);
    return (res);
}

unsigned long
SoWwGLWidget::getOverlayTransparentPixel(void){
    SOWW_STUB();
    return (0);
}

void
SoWwGLWidget::processEvent(wxEvent& WXUNUSED(event)){
    // Nothing is done here for the SoWwGLWidget, as realize, resize and
    // expose events are caught by explicitly attaching signal callbacks
    // to the widget.
}

wxWindow *
SoWwGLWidget::buildWidget(wxWindow* parent) {
    assert(parent !=0 && "parent can not be null");
    PRIVATE(this)->glparent = parent;
    return (PRIVATE(this)->buildGLWidget());
}

void
SoWwGLWidget::redrawOverlay(void){
    SOWW_STUB();
}

void
SoWwGLWidget::initGraphic(void){
    this->glLockNormal();
    // Need to set this explicitly when running on top of Open Inventor,
    // as it seems to have been forgotten there.
    // This code should be invoked from SoWwRenderArea::initGraphics()
    glEnable(GL_DEPTH_TEST);
    this->glUnlockNormal();
}
void
SoWwGLWidget::initOverlayGraphic(void){
    SOWW_STUB();
}

void
SoWwGLWidget::sizeChanged(const SbVec2s & size){
    SOWW_STUB();
#if SOWW_DEBUG
    SoDebugError::postInfo("SoWwGLWidget::sizeChanged",
                           "<%d, %d>",
                           size[0], size[1]
    );
#endif
    // Do nothing
}

void
SoWwGLWidget::widgetChanged(wxWindow* w){
    SOWW_STUB();
}

#undef PRIVATE
#undef PUBLIC
