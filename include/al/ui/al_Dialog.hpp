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
#include <mutex>

#include "imgui.h"

#include "al/io/al_Window.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_Imgui.hpp"


namespace al {

class Dialog : public Window, public WindowEventHandler {
public:


    enum Buttons {
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

    Dialog(std::string title, std::string text,
           int buttons = Buttons::AL_DIALOG_BUTTON_OK) {
        mTitle = title;
        mText = text;
        mModal = false; // Always non-modal (not easy to create modal windows on glfw)
        mButtons = (Buttons) buttons;

        append(windowEventHandler());
    };

    DialogResult exec() {
        static std::mutex mDialogLock; // Only allow one dialog window at a time
        std::unique_lock<std::mutex> lk (mDialogLock);
        bool is_verbose = true;
        // Should we call this here? Is it a problem to call again?
        // glfw::init(is_verbose);
        
        mDialogGraphics.init();

        // TODO Support in window render (e.g. for VR or Allosphere).
        Window::dimensions(200, 100);
        Window::title(mTitle);
        Window::create(is_verbose);
        imguiInit();
        mDone = false;
        while (!mDone) {
            onDraw(mDialogGraphics);
            Window::refresh();
            // TODO there should be a way to yield to the parent window to refresh it while we
            // display, even if this dialog is modal.
        }
        std::cout << "done" << std::endl;
        imguiShutdown();
        Window::close();
        imguiInit();
        return mReturnValue;
    }

protected:
    void onDraw(Graphics &g) {
        imguiBeginFrame();
//        ImGui::SetNextWindowBgAlpha(alpha);

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar; // if (no_titlebar)
        // window_flags |= ImGuiWindowFlags_NoScrollbar; // if (no_scrollbar)
        // window_flags |= ImGuiWindowFlags_MenuBar; // if (!no_menu)
        window_flags |= ImGuiWindowFlags_NoMove; // if (no_move)
        window_flags |= ImGuiWindowFlags_NoResize; // if (no_resize)
        window_flags |= ImGuiWindowFlags_NoCollapse; // if (no_collapse)
        window_flags |= ImGuiWindowFlags_NoNav; // if (no_nav)
//        if (!use_input) window_flags |= ImGuiWindowFlags_NoInputs;

        ImGui::SetNextWindowSize(ImVec2(Window::width(), Window::height()));
        ImGui::SetNextWindowPos(ImVec2(0,0));

        ImGui::Begin("" , nullptr, window_flags);
        ImGui::Text("%s", mText.c_str());

        if ( (int) mButtons & (int) Buttons::AL_DIALOG_BUTTON_OK) {
            if (ImGui::Button("OK##__al_Dialog")) {
                mReturnValue = DialogResult::AL_DIALOG_OK;
                mDone = true;
            }
            ImGui::SameLine();
        }
        if ( (int) mButtons & (int) Buttons::AL_DIALOG_BUTTON_CANCEL) {
            if (ImGui::Button("Cancel##__al_Dialog")) {
                mReturnValue = DialogResult::AL_DIALOG_CANCEL;
                mDone = true;
            }
            ImGui::SameLine();
        }
        if ( (int) mButtons & (int) Buttons::AL_DIALOG_BUTTON_YES) {
            if (ImGui::Button("Yes##__al_Dialog")) {
                mReturnValue = DialogResult::AL_DIALOG_YES;
                mDone = true;
            }
            ImGui::SameLine();
        }
        if ( (int) mButtons & (int) Buttons::AL_DIALOG_BUTTON_NO) {
            if (ImGui::Button("No##__al_Dialog")) {
                mReturnValue = DialogResult::AL_DIALOG_NO;
                mDone = true;
            }
            ImGui::SameLine();
        }

        imguiEndFrame();
        ImGui::End();

    }

    bool keyDown(Keyboard const &k) final {
        if (k.key() == Keyboard::ENTER) {
            mDone = true;
        }
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

    std::string mTitle;
    std::string mText;
    Buttons mButtons;
    DialogResult mReturnValue;
    std::atomic<bool> mDone;
    Graphics mDialogGraphics;
};

}


#endif // AL_DIALOG_HPP
