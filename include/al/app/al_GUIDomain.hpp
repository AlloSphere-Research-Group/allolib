#ifndef GUIDOMAIN_H
#define GUIDOMAIN_H

#include <functional>
#include <iostream>

#include "al/app/al_App.hpp"
#include "al/ui/al_ControlGUI.hpp"

#include "al_ComputationDomain.hpp"

namespace al {

class GUIPanelDomain : public SynchronousDomain {
public:
  virtual ~GUIPanelDomain() {}
  // Domain management functions
  bool init(ComputationDomain *parent = nullptr) override;
  bool tick() override;

  void configure(int initialx, int initialy, std::string name);

  // Put your gui widget code here
  std::function<void()> guiCode = []() {};

  // Put your gui configuration code here
  // This is called before creating panels
  std::function<void()> guiConfigurationCode = []() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Header] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.69f, 0.00f, 0.53f);
  };

  void fixedPosition(bool fixed);
  void setDimensions(int w, int h) {
    if (w < 0 || h < 0) {
      std::cerr << "Invalid dimensions. Ignoring. " << w << "," << h
                << std::endl;
      return;
    }
    mw = w;
    mh = h;
  }

  void setBackgroundAlpha(float alpha);

  void setFlags(int flags);

  ControlGUI gui;

private:
  int mx{-1}, my{-1}, mw{-1}, mh{-1};
  int mFlags{ImGuiWindowFlags_None};
};

/**
 * @brief GUIDomain class
 * @ingroup App
 */
class GUIDomain : public SynchronousDomain {
public:
  virtual ~GUIDomain() {}
  // Domain management functions
  bool init(ComputationDomain *parent = nullptr) override;
  bool tick() override;
  bool cleanup(ComputationDomain *parent = nullptr) override;

  static std::shared_ptr<GUIDomain>
  enableGUI(std::shared_ptr<GLFWOpenGLWindowDomain> domain);

  ControlGUI &newGUI();

  std::shared_ptr<GUIPanelDomain> newPanel();

private:
};

} // namespace al

#endif // GUIDOMAIN_H
