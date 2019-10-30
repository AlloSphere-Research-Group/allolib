#ifndef AL_FILESELECTOR_HPP
#define AL_FILESELECTOR_HPP

/*	Allolib --
    Multimedia / virtual environment application class library

    Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology,
   UCSB. Copyright (C) 2012-2018. The Regents of the University of California.
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
    FileSelector GUI for ImGui

    File author(s):
    Andrés Cabrera mantaraya36@gmail.com
*/

#include <functional>
#include <string>

#include "al/io/al_File.hpp"

namespace al {

/// FileSelector
/// @ingroup UI
class FileSelector {
 public:
  FileSelector(
      std::string globalRoot = "",
      std::function<bool(std::string)> filterfunction = [](std::string) {
        return true;
      });

  void start(std::string currentDir = "");

  /**
   * @brief drawFileSelector
   * @return true if a file has been selected
   */
  bool drawFileSelector();

  FileList getSelection();

  bool isActive() { return mActive; }

  void cancel() { mActive = false; }

 protected:
  bool filteringFunctionWrapper(FilePath const& fp) {
    return mFilterFunc(mGlobalRoot + mCurrentDir + "/" + fp.file());
  }

 private:
  std::string mSelectedItem = "";
  std::string mCurrentDir = "";
  std::string mGlobalRoot = "";
  FileList items;
  bool mActive{false};
  std::function<bool(std::string)> mFilterFunc;
};

}  // namespace al

#endif
