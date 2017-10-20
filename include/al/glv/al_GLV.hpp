#ifndef INCLUDE_GLV_AL_GLV_HPP
#define INCLUDE_GLV_AL_GLV_HPP

/*  GLV wrapper for allolib.
    Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_Graphics.hpp"

#include "al/glv/glv_core.h" // GLV, View
#include "al/glv/glv_layout.h"
#include "al/glv/glv_buttons.h"
#include "al/glv/glv_sliders.h"
#include "al/glv/glv_textview.h"

#include <memory>

namespace al {

void al_draw_glv(glv::GLV& glv, al::Graphics& g, unsigned x, unsigned y, unsigned w, unsigned h);
void al_draw_glv(glv::GLV& glv, al::Graphics& g, al::Window* win);

class GLVEventHandler : public WindowEventHandler {
public:
    glv::GLV* mGlvPtr;
    GLVEventHandler(glv::GLV& g);
    virtual bool mouseDown(const Mouse& m);
    virtual bool mouseDrag(const Mouse& m);
    virtual bool mouseUp(const Mouse& m);
    virtual bool resize(int dw, int dh);
};

class SliderMinMax : public glv::Slider {
public:
    double minimum, maximum;
    SliderMinMax(double min, double max, double v);
    double getCalcedValue();
    void setCalcedValue(double val) {
        setValue((val - minimum) / (maximum - minimum));
    }
};

class SliderWithLabel {
public:
    static void updateLabel(const glv::Notification& n);
	SliderMinMax slider;
    glv::Label label;
    glv::Label valueLabel;
    SliderWithLabel(std::string n);
    SliderWithLabel(std::string n, double min, double max);
    SliderWithLabel(std::string n, double min, double max, double v);
    void value(double val);
    double value();
    
};

class ButtonWithLabel {
public:
	glv::Buttons buttons;
    glv::Label label;
    ButtonWithLabel(std::string groupName, int numButtons);
    ButtonWithLabel(std::string groupName, int numCol, int numRow);
};

class NumberDialerWithLabel {
public:
	glv::NumberDialers dialer;
    glv::Label label;
    NumberDialerWithLabel(std::string name);
    NumberDialerWithLabel(std::string name, int numInt, int numFrac);
    NumberDialerWithLabel(std::string name, int numInt, int numFrac, float min, float max);
};

class GlvGui {
public:
    static int const max_entities = 64; // who makes more than 64 sliders!?
    glv::GLV mGlv;
    glv::Table mTable{">p", 5, 5};
    GLVEventHandler eventHandler{ mGlv };
    Window* mWindowPtr;
    std::unordered_map<std::string, std::unique_ptr<SliderWithLabel>> sliders;
    std::unordered_map<std::string, std::unique_ptr<ButtonWithLabel>> buttons;
    std::unordered_map<std::string, std::unique_ptr<NumberDialerWithLabel>> dialers;

    GlvGui(Window& window, bool blackLetters = false);

    void addSlider(SliderWithLabel& slider);
    void addSlider(std::string name);
    void addSlider(std::string name, double val);
    void addSlider(std::string name, double min, double max);
    void addSlider(std::string name, double min, double max, double val);
    void removeSlider(SliderWithLabel& slider);
    void removeSlider(std::string name);
    double sliderValue(std::string name);
    void setSliderValue(std::string name, double val);

    void addButton(ButtonWithLabel& buttons);
    void addButton(std::string name);
    void addButton(std::string name, bool onoff); // single button with initial value
    void addButton(std::string name, int numButtons);
    void addButton(std::string name, int numCol, int numRow);
    void removeButton(ButtonWithLabel& buttons);
    void removeButton(std::string name);
    bool buttonValue(std::string name);
    bool buttonValue(std::string name, int idx);
    bool buttonValue(std::string name, int colIdx, int rowIdx);
    void setButtonValue(std::string name, bool val);

    void addNumberDialer(NumberDialerWithLabel& dialer);
    void addNumberDialer(std::string name);
    void addNumberDialer(std::string name, int numInt, int numFrac);
    void addNumberDialer(std::string name, int numInt, int numFrac, float min, float max);
    void removeNumberDialer(NumberDialerWithLabel& dialer);
    void removeNumberDialer(std::string name);
    float numberDialerValue(std::string name);

    void draw(Graphics& g);
};

}

#endif