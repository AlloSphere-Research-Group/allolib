#ifndef INCLUDE_AL_GNOMON_HPP
#define INCLUDE_AL_GNOMON_HPP

#include "al/graphics/al_Font.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/types/al_Color.hpp"

namespace al {

/// Gnomon
/// @ingroup UI
struct Gnomon {
  Mesh gnomonMesh;
  Mesh arrowMesh;

  // static Gnomon gnomon;

  Color colors[3];
  const char *labels[3];

  Gnomon();

  void draw(Graphics &g);

  /// floating gnomon in front of camera
  void drawFloating(Graphics &g, Pose pose = Pose(), double scale = 1.0);

  /// Gnomon at any position
  void drawAtPos(Graphics &g, Vec3f pos, Pose cam_pose = Pose(),
                 double scale = 1.0);

  void drawOrigin(Graphics &g, Pose cam_pose = Pose(), double scale = 1.0);

  /// Gnomon at any pose
  void drawAtPose(Graphics &g, Pose pose, Pose cam_pose = Pose(),
                  double scale = 1.0);

  void drawArrows(Graphics &g);

  void drawLabels(Graphics &g, FontRenderer &fontrender, Pose cam_pose,
                  double scale = 1.0, Vec3f offset = Vec3f(0, 0, 0)) {
    // XYZ labels
    g.polygonFill();
    //    glEnable(GL_BLEND);
    //    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
    //    glEnable(GL_ALPHA_TEST);
    //    glAlphaFunc(GL_GREATER, 0.5);
    g.blending(true);
    g.blendTrans();
    for (int i = 0; i < 3; i++) {
      g.pushMatrix();
      Vec3f xform = gnomonMesh.vertices()[(i * 2) + 1];
      Vec3d forward = Vec3d(cam_pose.pos() - xform - offset).normalize();
      Quatd rot = Quatd::getBillboardRotation(forward, cam_pose.uu());

      g.translate(xform);
      g.rotate(rot);
      g.scale(scale);

      g.color(colors[i]);
      fontrender.write(labels[i], 0.1);
      fontrender.render(g);
      g.popMatrix();
    }
    //    glDisable(GL_ALPHA_TEST);
    //    glDisable(GL_BLEND);
  }
};

}  // namespace al
// Gnomon Gnomon::gnomon;

#endif
