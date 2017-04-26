#ifndef INC_GLV_DRAW_H
#define INC_GLV_DRAW_H

/*	Graphics Library of Views (GLV) - GUI Building Toolkit
	See COPYRIGHT file for authors and license information */

#include <cmath>
#include <stdio.h>
#include "al/glv/glv_color.h"

#define GLV_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

namespace glv {

typedef unsigned int index_t;

/// Two-dimensional point
struct Point2{
	Point2(){}
	Point2(float x_, float y_): x(x_), y(y_){}
	void operator()(float x_, float y_){x=x_; y=y_;}
	union{
		struct{ float x,y; };
		float elems[2];
	};
};

/// Three-dimensional point
struct Point3{
	Point3(){}
	Point3(float x_, float y_, float z_): x(x_), y(y_), z(z_){}
	void operator()(float x_, float y_, float z_){x=x_; y=y_; z=z_;}
	union{
		struct{ float x,y,z; };
		float elems[3];
	};
};


/// Buffers of vertices, colors, and indices
class GraphicsData{
public:

	GraphicsData(): mColors(1){}

	/// Get color buffer
	const Buffer<Color>& colors() const { return mColors; }
	
	/// Get index buffer
	const Buffer<index_t>& indices() const { return mIndices; }
	
	/// Get 2D vertex buffer
	const Buffer<Point2>& vertices2() const { return mVertices2; }
	
	/// Get 3D vertex buffer
	const Buffer<Point3>& vertices3() const { return mVertices3; }

	/// Reset all buffers
	void reset(){
		mVertices2.reset(); mVertices3.reset();
		mColors.reset(); mIndices.reset();
	}

	/// Append color
	void addColor(float r, float g, float b, float a=1){
		addColor(Color(r,g,b,a)); }

	/// Append color
	void addColor(const Color& c){
		colors().append(c); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2){
		addColor(c1); addColor(c2); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2, const Color& c3){
		addColor(c1,c2); addColor(c3); }

	/// Append colors
	void addColor(const Color& c1, const Color& c2, const Color& c3, const Color& c4){
		addColor(c1,c2,c3); addColor(c4); }

	/// Append index
	void addIndex(index_t i){
		indices().append(i); }
  
	/// Append indices
	void addIndex(index_t i1, index_t i2){
		addIndex(i1); addIndex(i2); }
  
	/// Append indices
	void addIndex(index_t i1, index_t i2, index_t i3){
		addIndex(i1,i2); addIndex(i3); }
  
	/// Append indices
	void addIndex(index_t i1, index_t i2, index_t i3, index_t i4){
		addIndex(i1,i2,i3); addIndex(i4); }

	/// Append 2D vertex
	void addVertex(float x, float y){ addVertex2(x,y); }

	/// Append 3D vertex
	void addVertex(float x, float y, float z){ addVertex3(x,y,z); }

	/// Append 2D vertex
	void addVertex2(float x, float y){
		vertices2().append(Point2(x,y)); }

	/// Append 2D vertices
	void addVertex2(float x1, float y1, float x2, float y2){
		addVertex2(x1,y1); addVertex2(x2,y2); }

	/// Append 2D vertices
	void addVertex2(float x1, float y1, float x2, float y2, float x3, float y3){
		addVertex2(x1,y1,x2,y2); addVertex2(x3,y3); }

	/// Append 2D vertices
	void addVertex2(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4){
		addVertex2(x1,y1,x2,y2,x3,y3); addVertex2(x4,y4); }

	/// Append 2D vertex
	template <class VEC2>
	void addVertex2(const VEC2& v){ addVertex2(v[0], v[1]); }

	/// Append 3D vertex
	void addVertex3(float x, float y, float z){ vertices3().append(Point3(x,y,z)); }

	/// Append 3D vertex
	template <class VEC3>
	void addVertex3(const VEC3& v){ addVertex3(v[0], v[1], v[2]); }

	/// Get mutable color buffer
	Buffer<Color>& colors(){ return mColors; }

	/// Get mutable index buffer
	Buffer<index_t>& indices(){ return mIndices; }

	/// Get mutable 2D vertex buffer
	Buffer<Point2>& vertices2(){ return mVertices2; }

	/// Get mutable 3D vertex buffer
	Buffer<Point3>& vertices3(){ return mVertices3; }

protected:
	Buffer<Point2> mVertices2;
	Buffer<Point3> mVertices3;
	Buffer<Color> mColors;
	Buffer<index_t> mIndices;
};

} // glv::

#endif

