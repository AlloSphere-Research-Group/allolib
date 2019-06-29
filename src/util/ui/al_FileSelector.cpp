
#include "al/util/ui/al_FileSelector.hpp"
#include "al/util/imgui/al_Imgui.hpp"

using namespace al;

FileSelector::FileSelector(std::string globalRoot, std::function<bool (std::string)> function) {
  if (function) {
    mFilterFunc = function;
  } else {
    mFilterFunc = [](std::string) {return true;};
  }
  mGlobalRoot = File::conformPathToOS(globalRoot);
}

void FileSelector::start(std::string currentDir) {
  mCurrentDir = currentDir;
  mSelectedItem = "";
  auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this, std::placeholders::_1);
  items = filterInDir(mGlobalRoot + mCurrentDir, boundFunc);
  mActive = true;
}

bool FileSelector::drawFileSelector() {
  if (!mActive) {
    return false;
  }

  std::string rootButtonText = mGlobalRoot;

  if (rootButtonText.size() != 0) { // Global root set.
    ImGui::Text("Global root:%s", rootButtonText.c_str());
  }
  ImGui::Text("Current Dir: %s", mCurrentDir.c_str());
  if (ImGui::Button("..")) {
    if (mCurrentDir.find('/') == std::string::npos && mCurrentDir.find('\\') == std::string::npos) {
      mCurrentDir = "";
    } else {
      if (mCurrentDir.find('/') != std::string::npos) {
        mCurrentDir = mCurrentDir.substr(0, mCurrentDir.rfind("/"));
      } else {
        mCurrentDir = mCurrentDir.substr(0, mCurrentDir.rfind("\\"));
      }
#ifndef AL_WINDOWS
      if (mGlobalRoot.size() == 0 && mCurrentDir == "") {
        // On *nix stop at root.
        mCurrentDir = "/";
      }
#endif
    }
    auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this, std::placeholders::_1);
    items = filterInDir(rootButtonText + mCurrentDir, boundFunc);
  }
  //    ImGui::SameLine();
  //    if (ImGui::Button("Set to current")) {
  //      mCurrentDir = rootButtonText + File::conformPathToOS(mDataRootPath.getCurrent()) + File::conformPathToOS(mAvailableDatasets.getCurrent());
  //      items = filterInDir(rootButtonText + mCurrentDir, [&](FilePath const&fp) {
  //          return File::isDirectory(rootButtonText + mCurrentDir + "/" + fp.file());});
  //    }
  bool itemClicked = false;
#ifdef AL_WINDOWS
  // show drive names for windows.
  if (items.count() == 0 &&  rootButtonText + mCurrentDir == "") {
    items.add(FilePath("C:"));
    items.add(FilePath("D:"));
    items.add(FilePath("E:"));
    items.add(FilePath("F:"));
    items.add(FilePath("Z:"));
  }
#endif
  for(auto item: items) {
    std::string itemText = item.file().c_str();
    if (ImGui::Selectable(item.file().c_str(), item.file() == mSelectedItem)) {
      mSelectedItem = item.file();
    }
    if (ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0)) {
      if (mCurrentDir.size() == 0) {
        mCurrentDir = mSelectedItem;
      } else {
        mCurrentDir = mCurrentDir + "/" + mSelectedItem;
      }
      mSelectedItem = "";
      itemClicked = true;
    }
  }
  if (itemClicked) {
    auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this, std::placeholders::_1);
    items = filterInDir(rootButtonText + mCurrentDir, boundFunc);
  }
  // FIXME push unique ids for these
  if (ImGui::Button("Select##Directory")) {
    return true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel##Directory")) {
    mActive = false;
  }
  return false;
}

FileList FileSelector::getSelection() {
  FileList list;
  list.add(FilePath(mSelectedItem, mCurrentDir));
  return list;
}

