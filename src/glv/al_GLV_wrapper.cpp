#include "al/glv/al_GLV.hpp"

al::GLVEventHandler::GLVEventHandler(glv::GLV& g) : mGlvPtr{ &g } {}

bool al::GLVEventHandler::mouseDown(const Mouse& m) {
    float relx = float(m.x());
    float rely = float(m.y());
    auto btn = glv::Mouse::Left;
    switch (m.button()) {
        case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
        case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    mGlvPtr->setMouseDown(relx, rely, btn, 0);
    mGlvPtr->setMousePos(int(m.x()), int(m.y()), relx, rely);
    bool consumed = mGlvPtr->propagateEvent();
    return !consumed;
}

bool al::GLVEventHandler::mouseDrag(const Mouse& m) {
    float x = float(m.x());
    float y = float(m.y());
    float relx = x;
    float rely = y;
    mGlvPtr->setMouseMotion(relx, rely, glv::Event::MouseDrag);
    mGlvPtr->setMousePos(int(x), int(y), relx, rely);
    bool consumed = mGlvPtr->propagateEvent();
    return !consumed;
}

bool al::GLVEventHandler::mouseUp(const Mouse& m) {
    float relx = float(m.x());
    float rely = float(m.y());
    auto btn = glv::Mouse::Left;
    switch (m.button()) {
        case al::Mouse::MIDDLE: btn = glv::Mouse::Middle; break;
        case al::Mouse::RIGHT: btn = glv::Mouse::Right; break;
    }
    mGlvPtr->setMouseUp(relx, rely, btn, 0);
    mGlvPtr->setMousePos(int(m.x()), int(m.y()), relx, rely);
    bool consumed = mGlvPtr->propagateEvent();
    return !consumed;
}

bool al::GLVEventHandler::resize(int dw, int dh) {
    mGlvPtr->extent(float(dw), float(dh));
    mGlvPtr->broadcastEvent(glv::Event::WindowResize);
    return true;
}






al::SliderMinMax::SliderMinMax(double min, double max, double v)
    : glv::Slider(), minimum(min), maximum(max)
{
    setValue((v - minimum) / (maximum - minimum));
}

double al::SliderMinMax::getCalcedValue() {
    return getValue() * (maximum - minimum) + minimum;
}






void al::SliderWithLabel::updateLabel(const glv::Notification& n) {
    glv::Label* l = n.receiver<glv::Label>();
    SliderMinMax* s = n.sender<SliderMinMax>();
    l->setValue(glv::toString(s->getCalcedValue()));
}

al::SliderWithLabel::SliderWithLabel(std::string n): SliderWithLabel{ n, 0, 1, 0.5 } {}
al::SliderWithLabel::SliderWithLabel(std::string n, double min, double max)
    : SliderWithLabel{n, min, max, 0.5 * (max + min)} {}

al::SliderWithLabel::SliderWithLabel(std::string n, double min, double max, double v)
    : slider{ min, max, v }, label{ n },
    valueLabel{ glv::toString(v), glv::Place::CL, 5, 0 }
{
    slider.attach(SliderWithLabel::updateLabel, glv::Update::Value, &valueLabel);
    slider << valueLabel;
    slider.disable(glv::FocusToTop);
}

void al::SliderWithLabel::value(double val) {
    slider.setValue(val);
}

double al::SliderWithLabel::value() {
    return  slider.getCalcedValue();
}






al::ButtonWithLabel::ButtonWithLabel(std::string groupName, int numButtons)
    : ButtonWithLabel{groupName, std::min(numButtons, 5), (4 + numButtons) / 5} {}
al::ButtonWithLabel::ButtonWithLabel(std::string groupName, int numCol, int numRow)
    : buttons {glv::Rect{float(20 * numCol), float(20 * numRow)}, numCol, numRow},
    label {groupName}
{
    buttons.disable(glv::FocusToTop);
}






al::NumberDialerWithLabel::NumberDialerWithLabel(std::string name)
    : NumberDialerWithLabel(name, 4, 3) {}
al::NumberDialerWithLabel::NumberDialerWithLabel(std::string name, int numInt, int numFrac)
    : NumberDialerWithLabel(name, numInt, numFrac, float(-std::pow(10, numInt)), float(std::pow(10, numInt))) {}
al::NumberDialerWithLabel::NumberDialerWithLabel(std::string name, int numInt, int numFrac, float min, float max)
    : dialer{numInt,numFrac,max,min},
    label {name}
{
    dialer.paddingX(0);
    dialer.paddingY(5);
    dialer.fontSize(10);
    dialer.disable(glv::FocusToTop);
}






al::GlvGui::GlvGui(Window& window, bool blackLetters) : mWindowPtr{ &window } {
    mGlv.extent(float(mWindowPtr->width()), float(mWindowPtr->height()));
    mGlv.broadcastEvent(glv::Event::WindowResize);
    if (blackLetters) {
        mGlv.colors().set(glv::StyleColor::Gray);
    }
    else {
        mGlv.colors().set(glv::StyleColor::SmokyGray);
    }
    mWindowPtr->append(eventHandler);
    mGlv << mTable;

    buttons.reserve(max_entities);
}

void al::GlvGui::addSlider(SliderWithLabel& slider) {
    mTable << slider.slider;
    mTable << slider.label;
    mTable.arrange();
}

void al::GlvGui::addSlider(std::string name, double min, double max, double val) {
    auto search = sliders.find(name);
    if (search != sliders.end()) {
        std::cout << "slider with name \"" << name << "\" already exists" << std::endl;
        return;
    }
    sliders[name] = std::make_unique<SliderWithLabel>(name, min, max, val);
    addSlider(*sliders[name]);
}
void al::GlvGui::addSlider(std::string name) { addSlider(name, 0, 1, 0.5); }
void al::GlvGui::addSlider(std::string name, double val) { addSlider(name, 0, 1, val); }
void al::GlvGui::addSlider(std::string name, double min, double max) { addSlider(name, min, max, 0.5 * (min + max)); }

void al::GlvGui::removeSlider(SliderWithLabel& slider) {
    slider.slider.remove();
    slider.label.remove();
    // reset pointer in glv class. it might have been pointing to an object getting removed
    mGlv.setFocus(nullptr);
    mTable.arrangement(">p");
    mTable.arrange();
}

void al::GlvGui::removeSlider(std::string name) {
    auto search = sliders.find(name);
    if (search == sliders.end()) {
        std::cout << "no slider with name \"" << name << "\" exists" << std::endl;
        return;
    }
    removeSlider(*(search->second));
    sliders.erase(search);
}

double al::GlvGui::sliderValue(std::string name) {
    auto search = sliders.find(name);
    if (search == sliders.end()) {
        std::cout << "no slider with name \"" << name << "\" exists" << std::endl;
        return -1;
    }
    return search->second->value();
}

void al::GlvGui::setSliderValue(std::string name, double val) {
    auto search = sliders.find(name);
    if (search == sliders.end()) {
        std::cout << "no slider with name \"" << name << "\" exists" << std::endl;
        return;
    }
    search->second->slider.setCalcedValue(val);
}

void al::GlvGui::addButton(ButtonWithLabel& buttons) {
    mTable << buttons.buttons;
    mTable << buttons.label;
    mTable.arrange();
}
void al::GlvGui::addButton(std::string name) {
    addButton(name, 1, 1);
}

void al::GlvGui::addButton(std::string name, bool onoff) {
    addButton(name, 1, 1);
    setButtonValue(name, onoff);
}

void al::GlvGui::addButton(std::string name, int numButtons) {
    addButton(name, std::min(numButtons, 5), (4 + numButtons) / 5);
}
void al::GlvGui::addButton(std::string name, int numCol, int numRow) {
    auto search = buttons.find(name);
    if (search != buttons.end()) {
        std::cout << "button with name \"" << name << "\" already exists" << std::endl;
        return;
    }
    buttons[name] = std::make_unique<ButtonWithLabel>(name, numCol, numRow);
    addButton(*buttons[name]);
}

void al::GlvGui::removeButton(ButtonWithLabel& buttons) {
    buttons.buttons.remove();
    buttons.label.remove();
    // reset pointer in glv class. it might have been pointing to an object getting removed
    mGlv.setFocus(nullptr);
    mTable.arrangement(">p");
    mTable.arrange();
}
void al::GlvGui::removeButton(std::string name) {
    auto search = buttons.find(name);
    if (search == buttons.end()) {
        std::cout << "no button with name \"" << name << "\" exists" << std::endl;
        return;
    }
    removeButton(*(search->second));
    buttons.erase(search);
}

bool al::GlvGui::buttonValue(std::string name) {
    return buttonValue(name, 0);
}
bool al::GlvGui::buttonValue(std::string name, int idx) {
    return buttonValue(name, idx % 5, idx / 5);
}
bool al::GlvGui::buttonValue(std::string name, int colIdx, int rowIdx) {
    auto search = buttons.find(name);
    if (search == buttons.end()) {
        std::cout << "no button with name \"" << name << "\" exists" << std::endl;
        return 0;
    }
    return search->second->buttons.getValue(colIdx, rowIdx);
}

void al::GlvGui::setButtonValue(std::string name, bool val) {
    auto search = buttons.find(name);
    if (search == buttons.end()) {
        std::cout << "no button with name \"" << name << "\" exists" << std::endl;
        return;
    }
    search->second->buttons.setValue(val);
}


void al::GlvGui::addNumberDialer(NumberDialerWithLabel& dialer) {
    mTable << dialer.dialer;
    mTable << dialer.label;
    mTable.arrange();
}
void al::GlvGui::addNumberDialer(std::string name) {
    addNumberDialer(name, 4, 3);
}
void al::GlvGui::addNumberDialer(std::string name, int numInt, int numFrac) {
    addNumberDialer(name, numInt, numFrac, float(-std::pow(10, numInt)), float(std::pow(10, numInt)));
}
void al::GlvGui::addNumberDialer(std::string name, int numInt, int numFrac, float min, float max) {
    auto search = dialers.find(name);
    if (search != dialers.end()) {
        std::cout << "dialer with name \"" << name << "\" already exists" << std::endl;
        return;
    }
    dialers[name] = std::make_unique<NumberDialerWithLabel>(name, numInt, numFrac, min, max);
    addNumberDialer(*dialers[name]);
}

void al::GlvGui::removeNumberDialer(NumberDialerWithLabel& dialer) {
    dialer.dialer.remove();
    dialer.label.remove();
    // reset pointer in glv class. it might have been pointing to an object getting removed
    mTable.arrangement(">p");
    mGlv.setFocus(nullptr);
    mTable.arrange();
}
void al::GlvGui::removeNumberDialer(std::string name) {
    auto search = dialers.find(name);
    if (search == dialers.end()) {
        std::cout << "no dialer with name \"" << name << "\" exists" << std::endl;
        return;
    }
    removeNumberDialer(*(search->second));
    dialers.erase(search);
}

float al::GlvGui::numberDialerValue(std::string name) {
    auto search = dialers.find(name);
    if (search == dialers.end()) {
        std::cout << "no dialer with name \"" << name << "\" exists" << std::endl;
        return 0;
    }
    return float(search->second->dialer.getValue());
}

void al::GlvGui::draw(Graphics& g) {
    al_draw_glv(mGlv, g, mWindowPtr);
}
