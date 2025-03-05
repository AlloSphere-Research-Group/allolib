
#include "al/ui/al_FileSelector.hpp"

#include "al/io/al_Imgui.hpp"

using namespace al;

FileSelector::FileSelector(std::string globalRoot,
                           std::function<bool(std::string)> filterfunction) {
  mFilterFunc = filterfunction;
  if (globalRoot.size() > 0) {
    mGlobalRoot = File::conformPathToOS(globalRoot);
  } else {
  }
}

void FileSelector::start(std::string currentDir) {
  if (currentDir.size() == 0) {
    currentDir = File::currentPath();
  }
  while (currentDir.size() > 2 &&
         (currentDir.back() == '/' || currentDir.back() == '\\')) {
    currentDir.resize(currentDir.size() - 1);
  }
  mCurrentDir = currentDir;
  mSelectedItem = "";
  auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this,
                             std::placeholders::_1);
  items = filterInDir(mGlobalRoot + mCurrentDir, boundFunc);
  mActive = true;
}

bool FileSelector::drawFileSelector() {
  if (!mActive) {
    return false;
  }

  std::string rootButtonText = mGlobalRoot;

  ImGui::PushID((void *)this);
  if (rootButtonText.size() != 0) { // Global root set.
    ImGui::Text("Global root:%s", rootButtonText.c_str());
  }
  ImGui::Text("Current Dir: %s", mCurrentDir.c_str());
  if (ImGui::Button("..")) {
    if (mCurrentDir.find('/') == std::string::npos &&
        mCurrentDir.find('\\') == std::string::npos) {
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
    auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this,
                               std::placeholders::_1);
    items = filterInDir(rootButtonText + mCurrentDir, boundFunc);
  }
  //    ImGui::SameLine();
  //    if (ImGui::Button("Set to current")) {
  //      mCurrentDir = rootButtonText +
  //      File::conformPathToOS(mDataRootPath.getCurrent()) +
  //      File::conformPathToOS(mAvailableDatasets.getCurrent()); items =
  //      filterInDir(rootButtonText + mCurrentDir, [&](FilePath const&fp) {
  //          return File::isDirectory(rootButtonText + mCurrentDir + "/" +
  //          fp.file());});
  //    }
  bool enterDirectory = false;
  bool applyItem = false;
#ifdef AL_WINDOWS
  // show drive names for windows.
  if (items.count() == 0 && rootButtonText + mCurrentDir == "") {
    items.add(FilePath("C:"));
    items.add(FilePath("D:"));
    items.add(FilePath("E:"));
    items.add(FilePath("F:"));
    items.add(FilePath("Z:"));
  }
#endif
  for (auto item : items) {
    std::string itemText = item.file().c_str();
    if (File::isDirectory(item.filepath())) {
      itemText = "[DIR] " + itemText;
    }

    if (ImGui::Selectable(itemText.c_str(), item.file() == mSelectedItem)) {
      mSelectedItem = item.file();
    }
    if (ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0)) {
      std::string newPath;
      if (mCurrentDir.size() == 0) {
        newPath = mSelectedItem;
      } else {
        newPath = mCurrentDir + "/" + mSelectedItem;
      }
      if (File::isDirectory(newPath)) {
        mCurrentDir = newPath;
        enterDirectory = true;
        mSelectedItem = "";
#ifdef AL_WINDOWS
      } else if (newPath.size() == 2 && newPath[1] == ':') {
        // For Windows drives
        mCurrentDir = newPath;
        enterDirectory = true;
        mSelectedItem = "";
#endif
      } else {
        applyItem = true;
      }
    }
  }
  if (enterDirectory) {
    auto boundFunc = std::bind(&FileSelector::filteringFunctionWrapper, this,
                               std::placeholders::_1);
    items = filterInDir(rootButtonText + mCurrentDir, boundFunc);
  }
  if (ImGui::Button("Select##Directory") || applyItem) {
    mActive = false;
    ImGui::PopID();
    if (mSelectedItem == "") {
      return false;
    }
    return true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel##Directory")) {
    mActive = false;
  }
  ImGui::PopID();
  return false;
}

FileList FileSelector::getSelection() {
  FileList list;
  if (mSelectedItem.size() == 0) {
    list.add(FilePath(mCurrentDir));
  } else {
    list.add(FilePath(mSelectedItem, mCurrentDir));
  }
  return list;
}
