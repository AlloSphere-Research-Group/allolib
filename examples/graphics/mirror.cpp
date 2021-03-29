/*
Allocore Example: Mirror

Description:
This example demonstrates how to use a fbo to render a reflection to a texture.

Author:
Tim Wood, Nov. 2015
Kon Hyong Kim, Mar. 2020
*/

#include <iostream>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"

using namespace al;
using namespace std;

struct MyApp : public App {
  EasyFBO fbo;
  Pose mirrorPose;
  Pose fboPose;

  Light light;        // Necessary to light objects in the scene
  Material material;  // Necessary for specular highlights

  Mesh surface, sphere;  // Geometry to render
  double phase = 0.5;    // Animation phase

  void onCreate() {
    fbo.init(1024, 1024);

    // position and orientation of our mirror
    mirrorPose.pos(0, 1.5, 0.5);
    mirrorPose.faceToward(Vec3f(0, 0, 0.5));

    // position nav
    nav().pos(0, -2.2, 0.5);
    nav().faceToward(Vec3f(0, 0, 0.5));

    // Create a circular wave pattern
    addSurface(surface, 64, 64);
    for (int i = 0; i < surface.vertices().size(); ++i) {
      Mesh::Vertex& p = surface.vertices()[i];
      p.z = cos(p.mag() * 4 * M_PI) * 0.1;
    }

    // For all meshes we would like to light, we must generate normals.
    // This function is valid for TRIANGLES and TRIANGLE_STRIP primitives.
    surface.generateNormals();

    // Create a sphere to see the location of the light source.
    addSphere(sphere, 0.05);

    // Set up light
    // light.globalAmbient(RGB(0.1));  // Ambient reflection for all lights
    light.ambient(RGB(0));          // Ambient reflection for this light
    light.diffuse(RGB(1, 1, 0.5));  // Light scattered directly from light
    // light.attenuation(1,1,0);   // Inverse distance attenuation
    // light.attenuation(1,0,1);   // Inverse-squared distance attenuation

    // Set up material (i.e., specularity)
    material.specular(light.diffuse() * 0.2);  // Specular highlight, "shine"
    material.shininess(50);  // Concentration of specular component [0,128]
  }

  void onAnimate(double dt) {
    // Set light position
    phase += 1. / 1800;
    if (phase > 1) phase -= 1;
    float x = cos(7 * phase * 2 * M_PI);
    float y = sin(11 * phase * 2 * M_PI);
    float z = cos(phase * 2 * M_PI) * 0.5 + 0.6;

    light.pos(x, y, z);
  }

  // move scene draw into seperate method since we will be rendering it twice
  // once from the view of the reflection, second from our nav
  void drawScene(Graphics& g) {
    // Render sphere at light position
    g.lighting(false);
    g.pushMatrix();
    g.translate(Vec3f(light.pos()));
    g.color(light.diffuse());
    g.draw(sphere);
    g.popMatrix();

    // Draw surface
    g.lighting(true);
    g.light(light);
    g.material(material);
    g.draw(surface);
  }

  void onDraw(Graphics& g) {
    // pose representing the virtual camera we will use to render the fbo
    Pose mirrorCam;

    // reflect position of nav relative to mirror across mirror plane normal,
    // then add mirror position offset

    mirrorCam.pos() = (nav().pos() - mirrorPose.pos()).reflect(mirrorPose.uf());
    mirrorCam.pos() += mirrorPose.pos();

    // face toward a point along reflected direction from nav to mirror across
    // mirror plane normal, keep mirror up vector

    mirrorCam.faceToward(
        mirrorCam.pos() +
            (mirrorPose.pos() - nav().pos()).reflect(mirrorPose.uf()),
        mirrorPose.uu());

    // calculate the approximate FOV for the mirror based on the center point.
    // This is not 100% optically accurate, with distortions near side of the
    // mirror
    float distFromMirrorPlane =
        (nav().pos() - mirrorPose.pos()).dot(mirrorPose.uf());
    float distFromMirrorCenter = (nav().pos() - mirrorPose.pos()).mag();
    float mirrorFOV = 180.f * M_1_PI * 2.f * atan2(1.f, distFromMirrorPlane) *
                      distFromMirrorPlane / distFromMirrorCenter;

    gl::depthTesting(true);

    // render scene to frame buffer
    g.pushFramebuffer(fbo);
    g.pushViewport(0, 0, 1024, 1024);
    g.clear(0.2, 0.2, 0.2, 0.2);
    g.pushProjMatrix(Matrix4f::perspective(mirrorFOV, 1, 0.001, 100));
    g.pushViewMatrix(Matrix4f::lookAt(mirrorCam.ux(), mirrorCam.uy(),
                                      mirrorCam.uz(), mirrorCam.pos()));

    // only draw scene from one side of the mirror
    if (distFromMirrorPlane > 0) {
      drawScene(g);  // draw scene to mirror fbo

      g.pushMatrix();
      g.translate(nav().pos());  // draw nav as sphere in mirror fbo
      g.lighting(false);
      g.color(light.diffuse());
      g.draw(sphere);
      g.popMatrix();
    }

    g.popProjMatrix();
    g.popViewMatrix();
    g.popViewport();
    g.popFramebuffer();

    // draw our mirror
    g.clear(0, 0, 0);
    g.lighting(false);
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
