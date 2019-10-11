#include "al/ui/al_Dialog.hpp"

using namespace al;

Dialog::Dialog(std::string title, std::string text, int buttons) {
  mTitle = title;
  mText = text;
  mOpen = false;
  mModal = false;
  mButtons = (Buttons)buttons;
}

void Dialog::start() { mOpen = true; }

void Dialog::start(std::function<void(Dialog::DialogResult)> doneCallback) {
  mDoneCallback = doneCallback;
  start();
}

bool Dialog::isOpen() { return mOpen; }

Dialog::DialogResult Dialog::getReturnValue() { return mReturnValue; }

void Dialog::draw(Graphics &g) {
  if (mOpen) {
    ImGui::OpenPopup("Modal window");
  }

  if (ImGui::BeginPopupModal("Modal window", &mOpen)) {
    ImGui::Text("%s", mText.c_str());

    if ((int)mButtons & (int)Buttons::AL_DIALOG_BUTTON_OK) {
      if (ImGui::Button("OK##__al_Dialog")) {
        mReturnValue = DialogResult::AL_DIALOG_OK;
        mOpen = false;
        ImGui::CloseCurrentPopup();
        mDoneCallback(mReturnValue);
      }
      ImGui::SameLine();
    }
    if ((int)mButtons & (int)Buttons::AL_DIALOG_BUTTON_CANCEL) {
      if (ImGui::Button("Cancel##__al_Dialog")) {
        mReturnValue = DialogResult::AL_DIALOG_CANCEL;
        mOpen = false;
        ImGui::CloseCurrentPopup();
        mDoneCallback(mReturnValue);
      }
      ImGui::SameLine();
    }
    if ((int)mButtons & (int)Buttons::AL_DIALOG_BUTTON_YES) {
      if (ImGui::Button("Yes##__al_Dialog")) {
        mReturnValue = DialogResult::AL_DIALOG_YES;
        mOpen = false;
        ImGui::CloseCurrentPopup();
        mDoneCallback(mReturnValue);
      }
      ImGui::SameLine();
    }
    if ((int)mButtons & (int)Buttons::AL_DIALOG_BUTTON_NO) {
      if (ImGui::Button("No##__al_Dialog")) {
        mReturnValue = DialogResult::AL_DIALOG_NO;
        mOpen = false;
        ImGui::CloseCurrentPopup();
        mDoneCallback(mReturnValue);
      }
    }
    ImGui::EndPopup();
  }
}
