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

#ifndef SOWW_SOWWP_H
#define SOWW_SOWWP_H

#ifndef SOWW_INTERNAL
#error this is a private header file
#endif /* !SOWW_INTERNAL */

#include <Inventor/Ww/SoGuiP.h>
#include <wx/timer.h>
#include <wx/app.h>
#include <wx/window.h>

class SoWwP : public SoGuiP {

public:

    bool init;
    wxWindow* main_frame;
    SoWwP();
    bool is_a_sowwp_app;
    wxAppConsole*  main_app;
    static wxTimer *timerqueuetimer;
    static wxTimer *delaytimeouttimer;

    static void initTimers();
    static void stopTimers();

    static SoWwP* instance();
    bool isInitialized() const;
    void setInitialize(bool);

    static void sensorQueueChanged();
    void onIdle(wxIdleEvent& event);
    void onClose(wxCloseEvent& event);

    void buildWxApp();
    void setWxApp(wxAppConsole*);
    wxAppConsole* getWxApp() const;

    void setMainFrame(wxWindow*);
    wxWindow* getMainFrame() const;

    void finish();

    static std::string dumpWindowData(const wxWindow* window);
};

#endif //SOWW_SOWWP_H
