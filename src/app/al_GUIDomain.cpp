#include "al/app/al_GUIDomain.hpp"

using namespace al;

bool GUIPanelDomain::init(ComputationDomain *parent) {
  gui.setTitle("ControlGUI");
  gui.init(mx, my, false);
  gui.fixedPosition(false);
  auto ret = SynchronousDomain::init(this);
  return ret;
}

bool GUIPanelDomain::tick() {

  tickSubdomains(true);
  // TODO add option for fixed position

  if (gui.mFixed) {
    ImGui::SetNextWindowPos(ImVec2(mx, my), ImGuiCond_Always);
  } else {
    ImGui::SetNextWindowPos(ImVec2(mx, my), ImGuiCond_FirstUseEver);
  }
  //  ImGui::SetNextWindowSize(ImVec2(width(), height()));
  ImGui::SetNextWindowBgAlpha(gui.mGUIBackgroundAlpha);
  ImGui::Begin(gui.mName.c_str(), nullptr, ImGuiWindowFlags_None);

  guiConfigurationCode();
  guiCode();
  gui.drawWidgets();

  ImGui::End();

  tickSubdomains(true);
  return true;
}

void GUIPanelDomain::configure(int initalx, int initialy, std::string name) {
  gui.setTitle(name);
}

void GUIPanelDomain::fixedPosition(bool fixed) { gui.fixedPosition(fixed); }

void GUIPanelDomain::setBackgroundAlpha(float alpha) {
    gui.backgroundAlpha(alpha);
}

bool GUIDomain::init(ComputationDomain *parent) {
    initializeSubdomains(true);
    if (!mInitialized) {
    imguiInit();
    mInitialized = true;
  }
  initializeSubdomains(false);
  return true;
}

bool GUIDomain::tick() {
  imguiBeginFrame();

  tickSubdomains(true);

  tickSubdomains(false);
  imguiEndFrame();

  imguiDraw();
  return true;
}

bool GUIDomain::cleanup(ComputationDomain *parent) {
  cleanupSubdomains(true);
  cleanupSubdomains(false);
  if (mInitialized) {
    imguiShutdown();
    mInitialized = false;
  }
  return true;
}

std::shared_ptr<GUIDomain>
GUIDomain::enableGUI(std::shared_ptr<GLFWOpenGLWindowDomain> domain) {
  return domain->newSubDomain<GUIDomain>();
}

ControlGUI &GUIDomain::newGUI() {
  auto panel = newPanel();
  return panel->gui;
}

std::shared_ptr<GUIPanelDomain> GUIDomain::newPanel() {
  return this->newSubDomain<GUIPanelDomain>();
}
