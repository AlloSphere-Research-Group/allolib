#include "al/core/graphics/al_EasyVAO.hpp"

using namespace al;

void EasyVAO::updateIndices(unsigned int* data, size_t size)
{
    mNumIndices = static_cast<int>(size);
    if (!mIndexBuffer.created())
    {
        mIndexBuffer.create();
        mIndexBuffer.bufferType(GL_ELEMENT_ARRAY_BUFFER);
    }
    mIndexBuffer.bind();
    mIndexBuffer.data(sizeof(unsigned int) * mNumIndices, data);
    // mIndexBuffer.unbind();
}

void EasyVAO::update(void* data, size_t typeSize, size_t arraySize, MeshAttrib& attrib, unsigned int dataType)
{
    validate();
    bind();
    updateWithoutBinding(data, typeSize, arraySize, attrib, dataType);
    // unbind();
}

void EasyVAO::updateWithoutBinding(void* data, size_t typeSize, size_t arraySize, MeshAttrib& attrib, unsigned int dataType) {
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
    primitive(m.primitive());
    validate();
    bind();
    mNumVertices = static_cast<unsigned int>(m.vertices().size());
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
