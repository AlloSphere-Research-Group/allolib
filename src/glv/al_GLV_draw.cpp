#include "al/glv/al_GLV.hpp"

#include "al/core/graphics/al_Mesh.hpp" // GraphicsData
#include "al/core/graphics/al_Shapes.hpp"
#include "al/core/graphics/al_DefaultShaders.hpp"


#include <cmath>
#include <iostream>
#include <memory>
#include <iostream>

using namespace al;

namespace glv {

struct GraphicsHolder {
    Graphics* mGraphics;
    std::unique_ptr<ShaderProgram> mShaderPtr;
    void set(Graphics& g) {
        mGraphics = &g;
    }
    Graphics& get() {
        return *mGraphics;
    }
    ShaderProgram& shader() {
        if (!mShaderPtr) {
            mShaderPtr = std::make_unique<ShaderProgram>();
            mShaderPtr->compile(al_color_vert_shader(), al_color_frag_shader());
        }
        return *mShaderPtr;
    }
};

GraphicsHolder& graphicsHolder() {
    static GraphicsHolder graphicsHolder;
    return graphicsHolder;
}

/// Returns closest pixel coordinate
int pix(float v) {
    return v >= 0 ? (int)(v+0.5f) : (int)(v-0.5f);
}
/// Returns center of closest pixel coordinate
float pixc(float v) {
    return pix(v) + 0.5f;
}

void rectangle(float l, float t, float r, float b) {
    static VAOMesh mesh = [&](){
        VAOMesh mesh {Mesh::TRIANGLES};
        // reserve 6 vertices
        mesh.vertices().reserve(6);
        for (int i = 0; i < 6; i += 1) {
            mesh.vertices().emplace_back();
        }
        return mesh;
    }();

    float x = l;
    float y = t;
    float w = r - l;
    float h = b - t;

    mesh.vertices()[0].set(x, y, 0);
    mesh.vertices()[1].set(x + w, y, 0);
    mesh.vertices()[2].set(x, y + h, 0);
    mesh.vertices()[3].set(x, y + h, 0);
    mesh.vertices()[4].set(x + w, y, 0);
    mesh.vertices()[5].set(x + w, y + h, 0);
    mesh.update();

    graphicsHolder().get().draw(mesh);
}

void frame(float l, float t, float r, float b) {
    static VAOMesh mesh = [&](){
        VAOMesh mesh {Mesh::LINE_STRIP};
        // reserve 5 vertices
        mesh.vertices().reserve(5);
        for (int i = 0; i < 5; i += 1) {
            mesh.vertices().emplace_back();
        }
        return mesh;
    }();

    float x = l;
    float y = t;
    float w = r - l;
    float h = b - t;
    // mesh.reset();
    // mesh.primitive(LINE_STRIP);
    mesh.vertices()[0].set(x, y, 0);
    mesh.vertices()[1].set(x + w, y, 0);
    mesh.vertices()[2].set(x + w, y + h, 0);
    mesh.vertices()[3].set(x, y + h, 0);
    mesh.vertices()[4].set(x, y, 0);
    mesh.update();

    graphicsHolder().get().draw(mesh);
}

void line(float x0, float y0, float x1, float y1) {
    static VAOMesh mesh = [&](){
        VAOMesh mesh {Mesh::LINES};
        // reserve 5 vertices
        mesh.vertices().reserve(2);
        for (int i = 0; i < 2; i += 1) {
            mesh.vertices().emplace_back();
        }
        return mesh;
    }();

    mesh.vertices()[0].set(x0, y0, 0);
    mesh.vertices()[1].set(x1, y1, 0);
    mesh.update();

    graphicsHolder().get().draw(mesh);
}

void lines(Mesh& gd) {
    static EasyVAO vao;
    // vao.primitive(GL_LINES);
    vao.primitive(Mesh::LINES);
    vao.updatePosition(gd.vertices().data(), gd.vertices().size());
    graphicsHolder().get().draw(vao);
}

void grid (
    float l, float t, float w, float h,
    float divx, float divy,
    bool incEnds=true
) {
    static VAOMesh mesh {Mesh::LINES};

    mesh.reset();
    float inc, r=l+w, b=t+h;

    if(divy > 0 && h>0){
        inc = (float)h/(float)divy;
        float i = incEnds ? t-0.0001f : t-0.0001f+inc;
        float e = incEnds ? b : b-inc;
        for(; i<e; i+=inc) {
            mesh.vertex(l, i);
            mesh.vertex(r, i);
        }
    }

    if(divx > 0 && w>0){
        inc = (float)w/(float)divx;
        float i = incEnds ? l-0.0001f : l-0.0001f+inc;
        float e = incEnds ? r : r-inc;
        for(; i<e; i+=inc) {
            mesh.vertex(i, t);
            mesh.vertex(i, b);
        }
    }
    mesh.update();
    graphicsHolder().get().draw(mesh);
}

decltype(auto) currentMatrix() {
    return graphicsHolder().get().modelMatrix();
}

void pushMatrix() {
    graphicsHolder().get().pushMatrix();
}
void popMatrix() {
    graphicsHolder().get().popMatrix();
}

void loadIdentity() {
    graphicsHolder().get().loadIdentity();
}

void translate(float x, float y) {
    graphicsHolder().get().translate(x, y);
}

void rotate(float angleX, float angleY, float angleZ) {
    graphicsHolder().get().rotate(angleX, angleY, angleZ);
}

void scale(float scaleX, float scaleY) {
    graphicsHolder().get().scale(scaleX, scaleY);
}

void scissorTest(bool doScissor) {
    graphicsHolder().get().scissorTest(doScissor);
}

void scissor(int x, int y, int w, int h) {
    graphicsHolder().get().scissor(x, y, w, h);
}

void color(float r, float g, float b, float a) {
    graphicsHolder().get().shader().uniform("col0", Color(r, g, b, a));
}

void color(Color const& c) {
    color(c.r, c.g, c.b, c.a);
}

void text(
    const char * s,
    float l=0, float t=0,
    unsigned fontSize=8, float lineSpacing=1.5, unsigned tabSpaces=4
) {
    Font f;
    f.size(float(fontSize));
    f.lineSpacing(lineSpacing);
    f.tabSpaces(tabSpaces);
    f.render(s, l, t, 0);
}

void drawContext (float tx, float ty, View * v, float& cx, float& cy, View *& c) {
    cx += tx; cy += ty; // update absolute coordinates of drawing context

    // clear model matrix (assumed set already)
    // it was loadIdentity() before but
    // pop then push preserves previous matrix (the one before pushing)
    popMatrix();
    pushMatrix();

    // pix: round position to nearest pixel coordinates
    translate(float(pix(cx)), float(pix(cy)));
    c = v;
}

void computeCrop(std::vector<Rect>& cr, int lvl, space_t ax, space_t ay, View * v) {
    if (v->enabled(CropChildren)) {
        cr[lvl].set(ax, ay, v->w, v->h);    // set absolute rect
        // get intersection with myself and previous level
        if (lvl>0) {
            Rect r = cr[lvl];
            // r.resizeEdgesBy(-1); // shrink area to save the borders
            r.intersection(cr[lvl-1], cr[lvl]);
        }
    }
    // if no child cropping, then inherit previous level's crop rect
    else{ cr[lvl] = cr[lvl-1]; }
}

// Views are drawn depth-first from leftmost to rightmost sibling
void GLV::drawWidgets(unsigned int ww, unsigned int wh, double dsec) {
    

    // TODO: Perhaps the View tree should be serialized into a separate list
    //      used for rendering?
    //      This will permit a user to change the graph structure during a draw 
    //      callback. Currently if this is attempted, we crash and burn.

    // Render all primitives at integer positions, ref: OpenGL Redbook
    // NOTE: This is a comprise to get almost pixel-perfection for both lines 
    // (half-integers) and polygons (integers). We'll do it "by hand" due to all
    // the exceptions and to get exact pixel-perfect accuracy.

    float cx = 0, cy = 0; // drawing context absolute position
    View * const root = this;
    View * cv = root;

    // The crop region is the intersection of all parent rects up to the top 
    // view. The intersections also need to be done in absolute coordinates.    
    std::vector<Rect> cropRects(16, Rect(float(ww), float(wh)));  // index is hierarchy level
    int lvl = 0;    // start at root = 0

    /* animation disabled */

    // Animate all the views
    // struct AnimateViews : public TraversalAction{
    //  AnimateViews(double dt_): dt(dt_){}
    //  virtual bool operator()(View * v, int depth){
    //      if(v->enabled(Animate)) v->onAnimate(dt);
    //      return true;
    //  }
    //  double dt;
    // } animateViews(dsec);
    // traverseDepth(animateViews);

    graphicsData().reset();
    doDraw(*this);

    //scissorTest(true);
    while(true){

        cv->onDataModelSync(); // update state based on attached model variables
        cv->rectifyGeometry();

        pushMatrix();
        // find the next view to draw

        // go to child node if exists and I'm drawable
        if(cv->child && cv->visible()) {
            drawContext(cv->child->l, cv->child->t, cv->child, cx, cy, cv);
            computeCrop(cropRects, ++lvl, cx, cy, cv);
        }
        else if(cv->sibling) { // go to sibling node if exists
            drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
            computeCrop(cropRects, lvl, cx, cy, cv);
        }
        else { // retrace upwards until a parent's sibling is found
            while(cv != root && cv->sibling == 0) {
                drawContext(-cv->l, -cv->t, cv->parent, cx, cy, cv);
                lvl--;
            }

            if(cv->sibling){
                drawContext(cv->sibling->l - cv->l, cv->sibling->t - cv->t, cv->sibling, cx, cy, cv);
                computeCrop(cropRects, lvl, cx, cy, cv);
            }
            else {
                popMatrix();
                break; // break the loop when the traversal returns to the root
            }
        }

        // draw current view
        if(cv->visible()){
            Rect r = cropRects[lvl-1]; // cropping region comes from parent context

            
            
            // if(cv->enabled(CropSelf)) { // crop my own draw?
            //     r.intersection(Rect(cx, cy, cv->w, cv->h), r);
            // }
            

            if(r.h<=0.f || r.w <= 0.f) { // bypass if drawing area outside of crop region
                // std::cout << "bypassing" << std::endl;
                popMatrix();
                continue;
            }

            // int sx = pix(r.l);
            // int sy = wh - (pix(r.t) + pix(r.h)) + 0.99; // scissor coord has origin at bottom left
            // int sw = pix(r.w);
            // int sh = r.h + 0.5;
            // if(sy < 0) sy=0;

            // glScissor takes size in framebuffer dimension
            //scissor(
            //  sx * windowHighresFactorX(),
            //  sy * windowHighresFactorY(),
            //  sw * windowHighresFactorX(),
            //  sh * windowHighresFactorY()
            //);

            graphicsData().reset();
            cv->doDraw(*this);
        }
        popMatrix();
    }
    //scissorTest(false);
}

void View::doDraw(GLV& glv){
    if(enabled(DrawBack)){
        color(colors().back);
        rectangle(0, 0, w, h);
    }

    bool drawNext = true;
    DrawHandlers::iterator it = mDrawHandlers.begin();
    while(it != mDrawHandlers.end()){
        DrawHandlers::iterator itnext = ++it; --it;
        glv.graphicsData().reset();
        drawNext = (*it)->onDraw(*this, glv);
        if(!drawNext) break;
        it = itnext;
    }

    if(drawNext){
        glv.graphicsData().reset();
        onDraw(glv);
    }

    if(enabled(DrawBorder)){
        // float borderWidth = 1.0;

        // double border thickness if focused
        // if(enabled(Focused) && enabled(FocusHighlight)){
        //  borderWidth *= 2;
        // }

        // lineWidth(borderWidth); // disabled in >gl3

        color(colors().border);
        frame(0.5f, 0.5f, float(pix(w))-0.5f, float(pix(h))-0.5f);
        // frame(1.5, 1.5, pix(w)-1.0, pix(h)-1.0);
        // const float ds = 0.5; // OpenGL suggests 0.375, but smears with AA enabled
        // frame(ds, ds, pix(w)-ds, pix(h)-ds);
        // frame(0,0, pix(w)-ds, pix(h)-ds); // hack to give bevelled look

    }
}

void Widget::drawGrid(){
    if(enabled(DrawGrid) && size()>1){
        color(colors().border);
        grid(0.0f, 0.0f, w, h, float(sizeX()), float(sizeY()), false);
    }
}

void Widget::drawSelectionBox(){
    if(enabled(Focused) && enabled(DrawSelectionBox) && size()>1){
        // lineWidth(1); // disabled in >gl3
        color(colors().border);
        frame(sx*dx(), sy*dy(), (sx+1)*dx(), (sy+1)*dy());
    }
}

void Widget::onDraw(GLV& g){
    // drawSelectionBox();
    drawGrid();
    g.graphicsData().reset();
}

void Sliders::onDraw(GLV& glv) {
    Widget::onDraw(glv);

    float x=paddingX(), xd=dx(), yd=dy();

    // TODO: fix padding in orientation direction

    if(vertOri()){
        for(int i=0; i<sizeX(); ++i){
        
            float y=paddingY();
        
            for(int j=0; j<sizeY(); ++j){
                int ind = index(i,j);
                auto const& cf = colors().fore;
                if(isSelected(i,j)) color(cf.r, cf.g, cf.b, cf.a);
                else                color(cf.r, cf.g, cf.b, cf.a * 0.5f);

                float v01 = float(to01(getValue(ind)));
                //float y0 = to01(0)*(yd - paddingY()*2);
                float y0 = float(to01(0)*yd);
                //rect(x + x0, y, f*xd+x, y+yd-padding());
                
                rectangle(x, y + (yd-v01*(yd-paddingY()*2)), x+xd-paddingX()*2, y + (yd-y0));

                // if zero line showing
                // if(max()>0 && min()<0){
                //  color(colors().border);
                //  float linePos = pixc(y+yd-y0);
                //  line(x, linePos, x+xd, linePos);
                // }
                y += yd;
            }
            x += xd;    
        }
    }
    else{
        for(int i=0; i<sizeX(); ++i){
        
            float y=paddingY();
        
            for(int j=0; j<sizeY(); ++j){
                int ind = index(i,j);
                auto const& cf = colors().fore;
                if(isSelected(i,j)) color(cf.r, cf.g, cf.b, cf.a);
                else                color(cf.r, cf.g, cf.b, cf.a * 0.5f);

                float v01 = float(to01(getValue(ind)));
                float x0 = float(to01(0))*xd;
                rectangle(x + x0, y, v01*(xd-paddingX()*2)+x, y+yd-paddingY()*2);

                // if zero line showing
                // if(max()>0 && min()<0){
                //  color(colors().border);
                //  float linePos = pixc(x+x0);
                //  line(linePos, y, linePos, y+yd);
                // }
                y += yd;
            }
            x += xd;
        }
    }
}

void Font::render(Mesh& gd, const char * v, float x, float y, float z) const {
    //static bool print_once = [](){ std::cout << "Font::render" << std::endl; return true; }();

    gd.reset();

    float sx = mScaleX;
    float sy = mScaleY;
    float tx = x;
    float ty = y;
    //float tz = z;
    //float sh = -0.5*sy; // TODO: shear needs to be done an a per-line basis
    //float sh = 0;
    
    //tx=ty=tz=0;

    struct RenderText : public TextIterator{
        RenderText(const Font& f_, const char *& s_, Mesh& g_, float tx_, float ty_, float sx_, float sy_)
        : TextIterator(f_,s_), g(g_), tx(tx_), ty(ty_), sx(sx_), sy(sy_){}
        bool onPrintable(char c){
            return addCharacter(g, c, pixc(tx+x*sx), pixc(ty+y*sy), sx, sy);
        }
        Mesh    & g;
        float tx,ty,sx,sy;
    } renderText(*this, v, gd, tx,ty,sx,sy);

    renderText.run();
    
    // draw::paint(draw::Lines, gd);
    lines(gd);
}

void SliderRange::onDraw(GLV& g) {
    //static bool print_once = [](){ std::cout << "SliderRange::onDraw" << std::endl; return true; }();

}

void Slider2D::onDraw(GLV& g) {
    //static bool print_once = [](){ std::cout << "Slider2D::onDraw" << std::endl; return true; }();

}

void Label::onDraw(GLV& g){
    // static bool print_once = [](){ std::cout << "Label::onDraw" << std::endl; return true; }();
    // lineWidth(stroke()); 
    color(colors().text);
    if(mVertical){
        translate(0,h);
        rotate(0,0,-90);
    }
    font().render(
        g.graphicsData(),
        data().toString().c_str(),
        paddingX(),
        paddingY()
    );
    // scale(mSize, mSize);
    // text(value().c_str());
}



void NumberDialers::fitExtent() {
    //static bool print_once = [](){ std::cout << "NumberDialers::fitExtent" << std::endl; return true; }();
    extent(
        // pix(sizeX() * (paddingX()*2 + (numDigits() * font().advance('M'))) + 1),
        float(pix(sizeX() * (paddingX() + (numDigits() * font().advance('M'))))),
        float(pix(sizeY() * (paddingY() * 2 + font().cap())))
    );
}


void NumberDialers::onDraw(GLV& g) {
    //static bool print_once = [](){ std::cout << "NumberDialers::onDraw" << std::endl; return true; }();
    // using namespace glv::draw;

    fitExtent();

    float dxCell= dx();
    float dyCell= dy();
    float dxDig = font().advance('M');

//  View::enable(DrawSelectionBox);
//  View::enable(DrawGrid);

    // draw box at position (only if focused)
    if(enabled(Focused)){

        float x = dxCell*selectedX() + paddingX()/1 - 1;
        //float y = dyCell*selectedY() + paddingY()/2;
        float y = dyCell*(float(selectedY())+0.5f);
        float ty= font().cap()/2.0f + 3.0f;

//      color(colors().fore, colors().fore.a*0.4);
        color(colors().selection);
        //rectangle(bx + dig()*dxDig, by, bx + (dig()+1)*dxDig, by + dyCell-0.5f);
        rectangle(x + dig()*dxDig, y-ty, x + (dig()+1)*dxDig, y+ty);
    }

    drawSelectionBox();
    // drawGrid(g.graphicsData());
    drawGrid();

    // lineWidth(1);

    if(mTextEntryMode){
        mTextEntry.extent(dxCell, dyCell);
        mTextEntry.pos(dxCell*selectedX(), dyCell*selectedY());
    }

    for(int i=0; i<sizeX(); ++i){
        for(int j=0; j<sizeY(); ++j){

            float cx = dxCell*i;    // left edge of cell
            float cy = dyCell*j;    // top edge of cell

            // draw number
            long long vali = valInt(i,j);
            unsigned long absVal = static_cast<unsigned long>(vali < 0 ? -vali : vali);
            int msd = mNF;  // position from right of most significant digit

            if(absVal > 0){
                msd = (int)log10((double)absVal);
                int p = numDigits() - (mShowSign ? 2:1);
                msd = msd < mNF ? mNF : (msd > p ? p : msd);
            }

            if(mNI == 0) msd-=1;

            // Determine digit string
            char str[32];
            int ic = numDigits();
            str[ic] = '\0';
            for(int i=0; i<numDigits(); ++i) str[i]=' ';

            if(mShowSign && vali < 0) str[0] = '-';

            unsigned long long power = 1;
            bool drawChar = false; // don't draw until non-zero or past decimal point

            for(int i=0; i<=msd; ++i){
                char c = '0' + static_cast<char>((absVal % (power*10))/power);
                power *= 10;
                if(c!='0' || i>=mNF) drawChar = true;
                --ic;
                if(drawChar) str[ic] = c;
            }

            // Draw the digit string
            float tx = float(int(cx + paddingX()));
            float ty = float(int(cy + paddingY()));

            if(vali || !dimZero()){
                color(colors().text);
            } else {
                color(colors().text.mix(colors().back, 0.8f));
            }
        //  printf("%s\n", str);
//          font().render(g.graphicsData(), str, pixc(tx), pixc(ty));
//          if(mNF>0) font().render(g.graphicsData(), ".", pixc(dxDig*(mNI+numSignDigits()-0.5f) + tx), pixc(ty));
            font().render(g.graphicsData(), str, tx, ty);
            if(mNF>0) font().render(g.graphicsData(), ".", dxDig*(mNI+numSignDigits()-0.5f) + tx, ty);
        }
    }
}

void DropDown::onDraw(GLV& g){
    //static bool print_once = [](){ std::cout << "DropDown::onDraw" << std::endl; return true; }();
}

ListView& ListView::fitExtent(){
    //static bool print_once = [](){ std::cout << "ListView::fitExtent" << std::endl; return true; }();
    return *this;
}


void ListView::onDraw(GLV& g){
    //static bool print_once = [](){ std::cout << "ListView::onDraw" << std::endl; return true; }();
}

void TextView::onDraw(GLV& g){
    //static bool print_once = [](){ std::cout << "TextView::onDraw" << std::endl; return true; }();
}

void Table::onDraw(GLV& g){
    if(enabled(DrawGrid)){
        color(colors().border);
        // lineWidth(1);
        for(unsigned i=0; i<mCells.size(); ++i){
            space_t cl,ct,cr,cb;
            getCellDim(i, cl,ct,cr,cb);
            cl -= mPad1/2;
            cr += mPad1/2;
            ct -= mPad2/2;
            cb += mPad2/2;
            frame(cl,ct,cr,cb);
        }
    }
}

void Scroll::onDraw(GLV& g){

}

void Divider::onDraw(GLV& g){
}



void Buttons::onDraw(GLV& g){
    Widget::onDraw(g);

    // using namespace glv::draw;

    float xd = dx();
    float yd = dy();
    float padx = paddingX();
    float pady = paddingY();
    color(colors().fore);
    
    // stroke(1);

    for (int i=0; i<sizeX(); ++i) {
        float x = xd*i + padx;
        for (int j=0; j<sizeY(); ++j) {
            float y = yd*j + pady;
            if (getValue(i,j)) {
                rectangle (x, y, x+xd-padx*2, y+yd-pady*2);
            }
            else {
                // off
            }
        }       
    }
    
}

} // namespace glv

void al::al_draw_glv(
    glv::GLV& glv, Graphics& g,
    unsigned x, unsigned y, unsigned w, unsigned h
) {
    g.depthTesting(false);
    g.blending(true);
    g.blendModeTrans();
    g.polygonMode(Graphics::FILL);
    g.cullFace(false);

    g.shader(glv::graphicsHolder().shader());

    g.pushViewport(x, y, w, h);
    g.pushCamera(Viewpoint::ORTHO_FOR_2D);

    glv::graphicsHolder().set(g);

    g.pushMatrix();
    g.loadIdentity();
    g.translate(0.0f, float(h)); // move to top-left
    g.scale(1, -1); // flip y
    glv.drawWidgets(w, h, 0); // 0 for dsec: animation disabled...

    g.popMatrix();
    g.popCamera();
}

void al_draw_glv(glv::GLV& glv, Graphics& g, Window* w) {
    // animation is disabled...
    al_draw_glv(glv, g, 0, 0, w->fbWidth(), w->fbHeight());
}