#include "al/ui/al_Gnomon.hpp"

#include "al/graphics/al_OpenGL.hpp"

using namespace al;

Gnomon::Gnomon() {
  colors[0] = RGB(1, 0, 0);
  colors[1] = RGB(0, 1, 0);
  colors[2] = RGB(0, 0, 1);
  labels[0] = "x";
  labels[1] = "y";
  labels[2] = "z";
  addPrism(arrowMesh, 0.0f, 0.025f, .1f, 10, 0);
  gnomonMesh.primitive(Mesh::LINES);
  // x line
  gnomonMesh.vertex(0, 0, 0);
  gnomonMesh.color(colors[0]);
  gnomonMesh.vertex(1, 0, 0);
  gnomonMesh.color(colors[0]);
  // y line
  gnomonMesh.vertex(0, 0, 0);
  gnomonMesh.color(colors[1]);
  gnomonMesh.vertex(0, 1, 0);
  gnomonMesh.color(colors[1]);
  // z line
  gnomonMesh.vertex(0, 0, 0);
  gnomonMesh.color(colors[2]);
  gnomonMesh.vertex(0, 0, 1);
  gnomonMesh.color(colors[2]);
}

void Gnomon::draw(Graphics &g) {
  g.draw(gnomonMesh);
  drawArrows(g);
}

void Gnomon::drawFloating(Graphics &g, Pose pose, double scale) {
  g.pushMatrix();
  g.polygonFill();
  //  g.polygonLine();
  Vec3d gnomZ = pose.pos() - (pose.uz() * 2); // put in front of camera
  gnomZ -= (pose.uy() * .4);                  // translate to the bottom
  gnomZ -= (pose.ux() * .5);                  // translate to the left
  g.meshColor();
  g.translate(gnomZ);
  g.scale((float)scale);
  g.draw(gnomonMesh);
  drawArrows(g);
  // drawLabels(g, pose, .005, gnomZ);
  g.popMatrix();
}

void Gnomon::drawAtPos(Graphics &g, Vec3f pos, Pose cam_pose, double scale) {
  g.pushMatrix();
  g.translate(pos);
  g.scale((float)scale);
  // g.lineWidth(2);

  g.meshColor();
  g.draw(gnomonMesh);
  //  drawLabels(g, cam_pose, .002, pos);
  drawArrows(g);
  g.popMatrix();
}

void Gnomon::drawOrigin(Graphics &g, Pose cam_pose, double scale) {
  drawAtPos(g, Vec3f(0, 0, 0), cam_pose, scale);
}

void Gnomon::drawAtPose(Graphics &g, Pose pose, Pose cam_pose, double scale) {
  g.pushMatrix();
  g.translate(pose.pos());
  g.rotate(pose.quat());
  g.scale((float)scale);
  // g.lineWidth(2);
  g.draw(gnomonMesh);
  drawArrows(g);
  // drawLabels(g, cam_pose, .002);
  g.popMatrix();
}

void Gnomon::drawArrows(Graphics &g) {
  Quatf q;
  for (int i = 0; i < 3; i++) {
    //    glPushAttrib(GL_CURRENT_BIT);
    g.polygonFill();
    g.meshColor();
    g.pushMatrix();
    g.color(colors[i]);
    g.translate(gnomonMesh.vertices()[(i * 2) + 1]);
    switch (i) {
    case 0:
      q.fromEuler((float)M_PI * 0.5f, 0, 0);
      break;
    case 1:
      q.fromEuler(0, (float)-M_PI * 0.5f, 0);
      break;
    case 2:
      q.fromEuler(0, 0, 0);
      break;
    }
    g.rotate(q);
    g.draw(arrowMesh);
    g.popMatrix();
    //    glPopAttrib();
  }
}
