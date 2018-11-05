#ifndef DRAWING_HPP
#define DRAWING_HPP

#include "al/core/graphics/al_Mesh.hpp"

namespace al
{
class Drawing {
public:
    void init() {
        verts.primitive(Mesh::LINE_STRIP);
        // Create a sine wave
        const int N = 128;
        for(int i=0; i<N; ++i){
            float f = float(i)/(N-1);
            verts.vertex(2*f-1, 0.5*sin(f*M_PI*2));
        }
    }

    void tick(float w, float h) {
        verts2.reset();
        const int N = 256;
        for (int i = 0; i < N; i += 1) {
            float t = float(i) / (N - 1);
            verts2.vertex(w * t, h * (0.5 + 0.5 * cos(t * M_2PI)));
            // The use color macro is set as a definition in flags.cmake
#ifdef USE_COLOR
            verts2.color(t, 1 - t, 0);
#endif
        }
    }

    void draw(Graphics &g) {
        g.camera(Viewpoint::IDENTITY);
        g.color(1, 1, 1);
        g.draw(verts);
        g.camera(Viewpoint::ORTHO_FOR_2D);
        g.meshColor();
        g.draw(verts2);
    }

private:
    al::Mesh verts;
    al::Mesh verts2 {al::Mesh::LINES};

};
}


#endif // DRAWING_HPP
