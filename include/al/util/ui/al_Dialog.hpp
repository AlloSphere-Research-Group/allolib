#ifndef AL_DIALOG_HPP
#define AL_DIALOG_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2018. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
    Dialog Window

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <atomic>

#include "imgui.h"

#include "al/core/io/al_Window.hpp"
#include "al/core/graphics/al_Graphics.hpp"
#include "al/util/imgui/al_Imgui.hpp"


namespace al {

class Dialog : public Window, public WindowEventHandler {
public:
    enum class Buttons : int { 
        AL_DIALOG_BUTTON_OK = 0x01,
        AL_DIALOG_BUTTON_CANCEL = 0x02,
        AL_DIALOG_BUTTON_YES = 0x04,
        AL_DIALOG_BUTTON_NO = 0x08
        };

    enum class DialogResult : int { 
        AL_DIALOG_OK,
        AL_DIALOG_CANCEL,
        AL_DIALOG_YES,
        AL_DIALOG_NO
        };

    Dialog(Buttons buttons = Buttons::AL_DIALOG_BUTTON_OK, bool modal = true) {
        mModal = modal;
        mButtons = buttons;
        append(windowEventHandler());
    };

    DialogResult exec() {
        bool is_verbose = true;
        // Should we call this here? Is it a problem to call again?
        // glfw::init(is_verbose);
        
        mDialogGraphics.init();

        // TODO if running in renderer, then don't create new window, but draw in the scene.
        // This probably needs more extensive modification of App and OmniRenderer.
        Window::create(is_verbose);
        initIMGUI();
        mDone = false;
        while (!mDone) {
            onDraw(mDialogGraphics);
            Window::refresh();
            // TODO there should be a way to yield to the parent window to refresh it while we
            // display, even if this dialog is modal.
        }
        std::cout << "done" << std::endl;
        shutdownIMGUI();
        Window::destroy();
        return DialogResult::AL_DIALOG_OK;
    }

protected:
    void onDraw(Graphics &g) {
        static float val = 0;
        g.clear(val);
        beginIMGUI();

        if ( (int) mButtons & (int) Buttons::AL_DIALOG_BUTTON_OK) {
            // TODO perhaps we should add a unique id to this button?
            if (ImGui::Button("Ok##Dialog")) {
                mReturnValue = DialogResult::AL_DIALOG_OK;
                mDone = true;
            }
        }

        endIMGUI();
        val = val + 0.01;
        if (val > 1) val = 0;
    }

    bool keyDown(Keyboard const &k) final {
        mDone = true;
        return false;
    } 
    // struct DialogKeyControls : WindowEventHandler {
    //     bool keyDown(const Keyboard& k) {
    //         this->onKey
    //     }
    // };
    // DialogKeyControls stdControls;

private:
    bool mModal;
    Buttons mButtons;
    DialogResult mReturnValue;
    std::atomic<bool> mDone;
    Graphics mDialogGraphics;
};

}


#endif // AL_DIALOG_HPP