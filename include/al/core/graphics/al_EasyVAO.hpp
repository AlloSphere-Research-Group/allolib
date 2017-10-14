#ifndef INCLUDE_AL_EASYVAO_HPP
#define INCLUDE_AL_EASYVAO_HPP

/*  VAO wrapper ready to receive mesh data

        - This class does not store cpu side data
        - Any user data format could be used (custom vector4, custom color class, etc)
          as long as they are stored as contiguous memory

    Keehong Youn, 2017, younkeehong@gmail.com
*/

#include "al/core/graphics/al_VAO.hpp"
#include "al/core/graphics/al_BufferObject.hpp"
#include "al/core/graphics/al_Mesh.hpp"

// #include <iostream>
// #include <unordered_map>

namespace al
{

class EasyVAO : public VAO
{
public:
    // layout (attribute location) for (GL) <-> (glsl shader)
    enum AttribLayout: unsigned int
    {
        LAYOUT_POSITION = 0,
        LAYOUT_COLOR = 1,
        LAYOUT_TEXCOORD = 2,
        LAYOUT_NORMAL = 3,
        LAYOUT_EXTRA_1D = 4,
        LAYOUT_EXTRA_2D = 5,
        LAYOUT_EXTRA_3D = 6,
        LAYOUT_EXTRA_4D = 7,
    };

    enum AttribDimension: unsigned int
    {
        DIMENSION_POSITION = 3,
        DIMENSION_COLOR = 4,
        DIMENSION_TEXCOORD = 2,
        DIMENSION_NORMAL = 3,
        DIMENSION_EXTRA_1D = 1,
        DIMENSION_EXTRA_2D = 2,
        DIMENSION_EXTRA_3D = 3,
        DIMENSION_EXTRA_4D = 4,
    };

    struct MeshAttrib
    {
        unsigned int layoutIndex; // attrib layout?
        unsigned int dataDimension; // vec3? vec4? ...
        BufferObject buffer;
        MeshAttrib(unsigned int i, unsigned int d): layoutIndex(i), dataDimension(d) {}
    };

    template <typename T>
    void updatePosition(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        mNumVertices = static_cast<int>(arraySize);
        update(data, sizeof(T), arraySize, mPositionAtt, dataType);
    }

    template <typename T>
    void updateColor(T* data, size_t arraySize, unsigned int dataType=GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mColorAtt, dataType);
    }
    template <typename T>
    void updateTexcoord(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mTexcoord2dAtt, dataType);
    }

    template <typename T>
    void updateNormal(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mNormalAtt, dataType);
    }

    template <typename T>
    void updateExtra1D(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mExtra1dAtt, dataType);
    }

    template <typename T>
    void updateExtra2D(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mExtra2dAtt, dataType);
    }

    template <typename T>
    void updateExtra3D(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mExtra3dAtt, dataType);
    }

    template <typename T>
    void updateExtra4D(T* data, size_t arraySize, unsigned int dataType = GL_FLOAT)
    {
        update(data, sizeof(T), arraySize, mExtra4dAtt, dataType);
    }

    // indices should be unsigned int
    void updateIndices(unsigned int* data, size_t size);
    void update(void* data, size_t typeSize, size_t arraySize, MeshAttrib& attrib, unsigned int dataType = GL_FLOAT);
    void primitive(unsigned int prim);
    void draw();

    // also recceives al::Mesh
    void update(Mesh& m);

private:
    void updateWithoutBinding(void* data, size_t typeSize, size_t arraySize, MeshAttrib& attrib, unsigned int dataType = GL_FLOAT);
    
    unsigned int mGLPrimMode = GL_TRIANGLES;
	int mNumVertices = 0;
	int mNumIndices = 0;
    MeshAttrib
        mPositionAtt {LAYOUT_POSITION, DIMENSION_POSITION},
        mColorAtt {LAYOUT_COLOR, DIMENSION_COLOR},
        mTexcoord2dAtt {LAYOUT_TEXCOORD, DIMENSION_TEXCOORD},
        mNormalAtt {LAYOUT_NORMAL, DIMENSION_NORMAL},
        mExtra1dAtt {LAYOUT_EXTRA_1D, DIMENSION_EXTRA_1D},
        mExtra2dAtt {LAYOUT_EXTRA_2D, DIMENSION_EXTRA_2D},
        mExtra3dAtt {LAYOUT_EXTRA_3D, DIMENSION_EXTRA_3D},
        mExtra4dAtt {LAYOUT_EXTRA_4D, DIMENSION_EXTRA_4D};

    BufferObject mIndexBuffer;
};

}

#endif