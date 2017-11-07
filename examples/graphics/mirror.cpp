/*
Allocore Example: Mirror

Description:
This example demonstrates how to use a fbo to render a reflection to a texture.

Author:
Tim Wood, Nov. 2015
*/

#include "al/core.hpp"
#include <iostream>

using namespace al;
using namespace std;

class MyApp : public App {
public:

  EasyFBO fbo;
  Pose mirrorPose;
  Pose fboPose;
  Vec3f lightPos;
  Mesh surface, sphere; // Geometry to render
  double phase;     // Animation phase

  void onCreate() {
    fbo.init(1024,1024);

    // position and orientation of our mirror
    mirrorPose.pos(0,1.5,0.5);
    mirrorPose.faceToward(Vec3f(0,0,0.5));

    // position nav
    nav().pos(0,-2.2,0.5);
    nav().faceToward(Vec3f(0,0,0.5));

    phase = 0.5;

    // Create a circular wave pattern
    addSurface(surface, 64,64);
    for(int i=0; i<surface.vertices().size(); ++i){
      Mesh::Vertex& p = surface.vertices()[i];
      p.z = cos(p.mag()*4*M_PI)*0.1;
    }

    // For all meshes we would like to light, we must generate normals.
    // This function is valid for TRIANGLES and TRIANGLE_STRIP primitives.
    surface.generateNormals();

    // Create a sphere to see the location of the light source.
    addSphere(sphere, 0.05);
  }

  void onAnimate(double dt){
    // Set light position
    phase += 1./1800; if(phase > 1) phase -= 1;
    float x = cos(7*phase*2*M_PI);
    float y = sin(11*phase*2*M_PI);
    float z = cos(phase*2*M_PI)*0.5 + 0.6;

    // light.pos(x,y,z);
    lightPos.set(x, y, z);
  }

  // move scene draw into seperate method since we will be rendering it twice
  // once from the view of the reflection, second from our nav
  void drawScene(Graphics& g){

    // Render sphere at light position
    g.pushMatrix();
      g.loadIdentity();
      g.translate(lightPos);
      g.color(1, 1, 1);
      g.draw(sphere);
    g.popMatrix();

    // Draw surface
    g.color(1, 0, 0);
    g.draw(surface);

  }


  void onDraw(Graphics& g){
    g.clear(0, 0, 0);

    // pose representing the virtual camera we will use to render the fbo 
    Pose mirrorCam;

    // reflect position of nav relative to mirror across mirror plane normal, then add mirror position offset
    mirrorCam.pos() = (nav().pos() - mirrorPose.pos()).reflect(mirrorPose.uf());
    mirrorCam.pos() += mirrorPose.pos();
    
    // face toward a point along reflected direction from nav to mirror across mirror plane normal, keep mirror up vector
    mirrorCam.faceToward(mirrorCam.pos() + (mirrorPose.pos()-nav().pos()).reflect(mirrorPose.uf()), mirrorPose.uu());

    // render scene to frame buffer
    g.pushFramebuffer(fbo);
    g.pushViewport(0, 0, 1024, 1024);
    g.clear(0.2, 0.2, 0.2, 0.2);
    g.pushProjMatrix(Matrix4f::perspective(45, 1, 0.001, 100));
    g.pushViewMatrix(Matrix4f::lookAt(mirrorCam.ux(), mirrorCam.uy(), mirrorCam.uz(), mirrorCam.pos()));

    drawScene(g); // draw scene to mirror fbo
    g.pushMatrix();
    g.translate(nav().pos()); // draw nav as sphere in mirror fbo
    g.draw(sphere);
    g.popMatrix();

    g.popProjMatrix();
    g.popViewMatrix();
    g.popViewport();
    g.popFramebuffer();

    // draw our mirror
    g.pushMatrix();
    g.translate(mirrorPose.pos());
    g.rotate(mirrorPose.quat());
    g.quad(fbo.tex(), -1, -1, 2, 2);
    g.popMatrix();

    // draw the scene 
    drawScene(g);
  }

};

int main() {
  MyApp app;
  app.start();
}
