#pragma once

#include "al/core/gl/al_GLEW.hpp"
#include "al/core/gl/al_BufferObject.hpp"

namespace al {
    
class VAO : public GPUObject {
public:
    VAO();
    virtual ~VAO();

    virtual void onCreate() override;
    virtual void onDestroy() override;

    void bind();
    void unbind();

    void enableAttrib(unsigned int index);
    void disableAttrib(unsigned int index);
    void attribPointer(
        unsigned int index,
        BufferObject& buffer,
        int size, // components
        unsigned int type = GL_FLOAT,
        unsigned char normalized = GL_FALSE,
        int stride = 0,
        void const* offset = NULL // offset of the first component
    );
};

}