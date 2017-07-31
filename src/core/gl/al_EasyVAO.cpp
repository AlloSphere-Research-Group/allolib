#include "al/core/gl/al_EasyVAO.hpp"

using namespace al;

std::unordered_map<unsigned int, unsigned int> EasyVAO::mPrimMap = {
  { Mesh::POINTS, GL_POINTS },
  { Mesh::LINES, GL_LINES },
  { Mesh::LINE_STRIP, GL_LINE_STRIP },
  { Mesh::LINE_LOOP, GL_LINE_LOOP },
  { Mesh::TRIANGLES, GL_TRIANGLES },
  { Mesh::TRIANGLE_STRIP, GL_TRIANGLE_STRIP },
  { Mesh::TRIANGLE_FAN, GL_TRIANGLE_FAN },
  { Mesh::LINES_ADJACENCY, GL_LINES_ADJACENCY },
  { Mesh::LINE_STRIP_ADJACENCY, GL_LINE_STRIP_ADJACENCY },
  { Mesh::TRIANGLES_ADJACENCY, GL_TRIANGLES_ADJACENCY },
  { Mesh::TRIANGLE_STRIP_ADJACENCY, GL_TRIANGLE_STRIP_ADJACENCY }
};

void EasyVAO::updateIndices(unsigned int* data, int size)
{
    mNumIndices = size;
    if (!mIndexBuffer.created())
    {
        mIndexBuffer.create();
        mIndexBuffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
    }
    mIndexBuffer.bind();
    mIndexBuffer.data(sizeof(unsigned int) * mNumIndices, data);
    // mIndexBuffer.unbind();
}

void EasyVAO::update(void* data, int typeSize, int arraySize, MeshAttrib& attrib, unsigned int dataType)
{
    validate();
    bind();
    updateWithoutBinding(data, typeSize, arraySize, attrib, dataType);
    // unbind();
}

void EasyVAO::updateWithoutBinding(void* data, int typeSize, int arraySize, MeshAttrib& attrib, unsigned int dataType) {
    // only enable attribs with content
    if (arraySize > 0) {
        enableAttrib(attrib.layoutIndex);
    }
    else {
        disableAttrib(attrib.layoutIndex);
        return;
    }

    // buffer yet created, make it and set vao to point to it
    // note: data should be in floats!
    if (!attrib.buffer.created()) {
        attrib.buffer.create();
        attribPointer(attrib.layoutIndex, attrib.buffer, attrib.dataDimension, dataType);
    }

    // upload CPU size data to buffer in GPU
    attrib.buffer.bind();
    attrib.buffer.data(typeSize * arraySize, data);
    // attrib.buffer.unbind();
}

void EasyVAO::update(Mesh& m) {
    primitive(mPrimMap[m.primitive()]);
    validate();
    bind();
    mNumVertices = m.vertices().size();
    updateWithoutBinding(m.vertices().data(), sizeof(Vec3f), m.vertices().size(), mPositionAtt);
    updateWithoutBinding(m.colors().data(), sizeof(Vec4f), m.colors().size(), mColorAtt);
    updateWithoutBinding(m.texCoord2s().data(), sizeof(Vec2f), m.texCoord2s().size(), mTexcoord2dAtt);
    updateWithoutBinding(m.normals().data(), sizeof(Vec3f), m.normals().size(), mNormalAtt);
    // unbind();
    updateIndices(m.indices().data(), m.indices().size());
}

void EasyVAO::primitive(unsigned int prim)
{
    mGLPrimMode = prim;
}

void EasyVAO::draw()
{
    bind();
    if (mNumIndices > 0)
    {
        mIndexBuffer.bind();
        glDrawElements(mGLPrimMode, mNumIndices, GL_UNSIGNED_INT, NULL);
        // mIndexBuffer.unbind();
    }
    else
    {
        glDrawArrays(mGLPrimMode, 0, mNumVertices);
    }
    // unbind();
}
