#include "al/core/gl/al_EasyVAO.hpp"

using namespace al;

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
    mIndexBuffer.unbind();
}

void EasyVAO::update(void* data, int typeSize, int arraySize, MeshAttrib& attrib, unsigned int dataType)
{
    validate();
    bind();
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
    attrib.buffer.unbind(); 
    unbind();
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
        mIndexBuffer.unbind();
    }
    else
    {
        glDrawArrays(mGLPrimMode, 0, mNumVertices);
    }
    unbind();
}
