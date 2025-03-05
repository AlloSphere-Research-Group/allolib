#ifndef AL_DIALOG_HPP
#define AL_DIALOG_HPP

/*	Allocore --
        Multimedia / virtual environment application class library

        Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
        All rights reserved.

        Redistribution and use in source and binary forms, with or without
        modification, are permitted provided that the following conditions are
   met:

                Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

                Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the
   distribution.

                Neither the name of the University of California nor the names
   of its contributors may be used to endorse or promote products derived from
                this software without specific prior written permission.

        THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
        IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

        File description:
    ImGui modal query dialog

        File author(s):
        Andrés Cabrera mantaraya36@gmail.com
*/

#include <functional>
#include <string>

#include "al/graphics/al_Graphics.hpp"
#include "al/io/al_Imgui.hpp"

namespace al {

/// Dialog
/// @ingroup UI
class Dialog {
 public:
  virtual ~Dialog() {}

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
         int buttons = Buttons::AL_DIALOG_BUTTON_OK);

  void start();
  void start(std::function<void(DialogResult)> doneCallback);

  bool isOpen();
  DialogResult getReturnValue();

  virtual void draw(Graphics &g);

 protected:
 private:
  bool mModal;

  std::function<void(DialogResult)> mDoneCallback;

  std::string mTitle;
  std::string mText;
  Buttons mButtons;
  DialogResult mReturnValue;
  bool mOpen;
};

}  // namespace al

#endif  // AL_DIALOG_HPP
