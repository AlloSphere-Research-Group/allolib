#include "al/graphics/al_Shader.hpp"
#include "al/graphics/al_OpenGL.hpp"
#include "al/system/al_Printing.hpp"

#include <cstring>
#include <string>

using std::string;

namespace al {

GLenum gl_shader_type(Shader::Type v) {
  switch (v) {
    case Shader::FRAGMENT:
      return GL_FRAGMENT_SHADER;
    case Shader::VERTEX:
      return GL_VERTEX_SHADER;
    case Shader::GEOMETRY:
      return GL_GEOMETRY_SHADER;
    default:
      return 0;
  }
}

const char* ShaderBase::log() const {
  GLint lsize;
  get(GL_INFO_LOG_LENGTH, &lsize);
  if (0 == lsize) return nullptr;

  static char buf[AL_SHADER_MAX_LOG_SIZE];
  getLog(buf);
  return buf;
}

void ShaderBase::printLog() const {
  const char* s = log();
  if (s && s[0]) printf("%s\n", s);
}

void Shader::getLog(char* buf) const {
  glGetShaderInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}

void ShaderProgram::getLog(char* buf) const {
  glGetProgramInfoLog(id(), AL_SHADER_MAX_LOG_SIZE, NULL, buf);
}

Shader::Shader(const std::string& source, Shader::Type type)
    : mSource(source), mType(type) {}

Shader& Shader::compile() {
  if (!mSource.empty()) {
    create();
    const char* s = mSource.c_str();
    glShaderSource(mID, 1, &s, NULL);
    glCompileShader(mID);
  }
  return *this;
}

bool Shader::compiled() const {
  GLint v;
  glGetShaderiv(mID, GL_COMPILE_STATUS, &v);
  return (v == GL_TRUE);
}

void Shader::get(int pname, void* params) const {
  glGetShaderiv(id(), pname, (GLint*)params);
}

void Shader::onCreate() {
  mID = glCreateShader(gl_shader_type(mType));
  if (0 == id()) AL_WARN("Error creating shader object");
}

void Shader::onDestroy() { glDeleteShader(id()); }

Shader& Shader::source(const std::string& v) {
  mSource = v;
  return *this;
}

Shader& Shader::source(const std::string& src, Shader::Type type) {
  mType = type;
  return source(src);
}

static ShaderProgram::Type param_type_from_gltype(GLenum gltype) {
  switch (gltype) {
    case GL_FLOAT:
      return ShaderProgram::FLOAT;
    case GL_FLOAT_VEC2:
      return ShaderProgram::VEC2;
    case GL_FLOAT_VEC3:
      return ShaderProgram::VEC3;
    case GL_FLOAT_VEC4:
      return ShaderProgram::VEC4;

    case GL_INT:
      return ShaderProgram::INT;
    case GL_INT_VEC2:
      return ShaderProgram::INT2;
    case GL_INT_VEC3:
      return ShaderProgram::INT3;
    case GL_INT_VEC4:
      return ShaderProgram::INT4;

    case GL_BOOL:
      return ShaderProgram::BOOL;
    case GL_BOOL_VEC2:
      return ShaderProgram::BOOL2;
    case GL_BOOL_VEC3:
      return ShaderProgram::BOOL3;
    case GL_BOOL_VEC4:
      return ShaderProgram::BOOL4;

    case GL_FLOAT_MAT2:
      return ShaderProgram::MAT22;
    case GL_FLOAT_MAT3:
      return ShaderProgram::MAT33;
    case GL_FLOAT_MAT4:
      return ShaderProgram::MAT44;

    case GL_SAMPLER_1D:
      return ShaderProgram::SAMPLER_1D;
    case GL_SAMPLER_2D:
      return ShaderProgram::SAMPLER_2D;
    case GL_SAMPLER_2D_RECT:
      return ShaderProgram::SAMPLER_RECT;
    case GL_SAMPLER_3D:
      return ShaderProgram::SAMPLER_3D;
    case GL_SAMPLER_CUBE:
      return ShaderProgram::SAMPLER_CUBE;
    case GL_SAMPLER_1D_SHADOW:
      return ShaderProgram::SAMPLER_1D_SHADOW;
    case GL_SAMPLER_2D_SHADOW:
      return ShaderProgram::SAMPLER_2D_SHADOW;
    default:
      return ShaderProgram::NONE;
  }
}

ShaderProgram::ShaderProgram() {}

ShaderProgram::~ShaderProgram() { destroy(); }

ShaderProgram& ShaderProgram::attach(Shader& s) {
  glAttachShader(id(), s.id());
  return *this;
}

const ShaderProgram& ShaderProgram::detach(const Shader& s) const {
  glDetachShader(id(), s.id());
  return *this;
}
const ShaderProgram& ShaderProgram::link(bool doValidate) const {
  glLinkProgram(id());
  if (doValidate) validateProgram();
  return *this;
}

bool ShaderProgram::validateProgram(bool doPrintLog) const {
  GLint isValid;
  glValidateProgram(id());
  glGetProgramiv(id(), GL_VALIDATE_STATUS, &isValid);
  if (GL_FALSE == isValid) {
    if (doPrintLog) printLog();
  }
  return true;
}

bool ShaderProgram::compile(const std::string& vertSource,
                            const std::string& fragSource,
                            const std::string& geomSource) {
  create();  // will destroy and recreate if already created

  mVertSource = vertSource;
  mFragSource = fragSource;
  mGeomSource = geomSource;

  Shader mShaderV{vertSource, al::Shader::VERTEX};
  mShaderV.compile();
  mShaderV.printLog();
  attach(mShaderV);

  Shader mShaderF{fragSource, al::Shader::FRAGMENT};
  mShaderF.compile();
  mShaderF.printLog();
  attach(mShaderF);

  Shader mShaderG{geomSource, al::Shader::GEOMETRY};
  bool bGeom = !geomSource.empty();
  if (bGeom) {
    mShaderG.compile();
    mShaderG.printLog();
    attach(mShaderG);
  }

  link(false);
  printLog();

  // OpenGL.org says to detach shaders after linking:
  //   https://www.opengl.org/wiki/Shader_Compilation
  detach(mShaderV);
  detach(mShaderF);
  if (bGeom) detach(mShaderG);

  if (!linked()) {
    return false;
  }
  return true;
}

void ShaderProgram::onCreate() { mID = glCreateProgram(); }
void ShaderProgram::onDestroy() { glDeleteProgram(id()); }

void ShaderProgram::use(unsigned programID) { glUseProgram(programID); }

const ShaderProgram& ShaderProgram::use() {
  use(id());
  return *this;
}

void ShaderProgram::begin() { use(); }

void ShaderProgram::end() const { glUseProgram(0); }

bool ShaderProgram::linked() const {
  GLint v;
  get(GL_LINK_STATUS, &v);
  return (v == GL_TRUE);
}

#define GET_LOC(map, glGetFunc)  \
  int loc;                       \
  auto it = map.find(name);      \
  if (it != map.end()) {         \
    loc = it->second;            \
  } else {                       \
    loc = glGetFunc(id(), name); \
    map[name] = loc;             \
  }

int ShaderProgram::getUniformLocation(const char* name) const {
  GET_LOC(mUniformLocs, glGetUniformLocation);
  if (-1 == loc) {
    AL_WARN_ONCE("No such uniform named \"%s\"", name);
  }
  return loc;
}

int ShaderProgram::attribute(const char* name) const {
  GET_LOC(mAttribLocs, glGetAttribLocation);
  if (-1 == loc) {
    AL_WARN_ONCE("No such attribute named \"%s\"", name);
  }
  return loc;
}

void ShaderProgram::uniform4f(int loc, float v0, float v1, float v2,
                              float v3) const {
  glUniform4f(loc, v0, v1, v2, v3);
}

void ShaderProgram::uniformMat4f(int loc, float* data) const {
  glUniformMatrix4fv(loc, 1, GL_FALSE, data);
}

const ShaderProgram& ShaderProgram::uniform(int loc, int v) const {
  glUniform1i(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v) const {
  glUniform1f(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1) const {
  glUniform2f(loc, v0, v1);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1,
                                            float v2) const {
  glUniform3f(loc, v0, v1, v2);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform(int loc, float v0, float v1,
                                            float v2, float v3) const {
  glUniform4f(loc, v0, v1, v2, v3);
  return *this;
}

const ShaderProgram& ShaderProgram::uniform4v(int loc, const float* v,
                                              int count) const {
  glUniform4fv(loc, count, v);
  return *this;
}

const ShaderProgram& ShaderProgram::uniformMatrix3(int loc, const float* v,
                                                   bool transpose) const {
  glUniformMatrix3fv(loc, 1, transpose, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniformMatrix4(int loc, const float* v,
                                                   bool transpose) const {
  glUniformMatrix4fv(loc, 1, transpose, v);
  return *this;
}

const ShaderProgram& ShaderProgram::uniform1(const char* name, const float* v,
                                             int count) const {
  glUniform1fv(getUniformLocation(name), count, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform2(const char* name, const float* v,
                                             int count) const {
  glUniform2fv(getUniformLocation(name), count, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform3(const char* name, const float* v,
                                             int count) const {
  glUniform3fv(getUniformLocation(name), count, v);
  return *this;
}
const ShaderProgram& ShaderProgram::uniform4(const char* name, const float* v,
                                             int count) const {
  return uniform4v(getUniformLocation(name), v, count);
  // glUniform4fv(getUniformLocation(name), count, v); return *this;
}

const ShaderProgram& ShaderProgram::uniform(const char* name, int v) const {
  return uniform(getUniformLocation(name), v);
}
const ShaderProgram& ShaderProgram::uniform(const char* name, float v) const {
  return uniform(getUniformLocation(name), v);
}
const ShaderProgram& ShaderProgram::uniform(const char* name, float v0,
                                            float v1) const {
  return uniform(getUniformLocation(name), v0, v1);
}
const ShaderProgram& ShaderProgram::uniform(const char* name, float v0,
                                            float v1, float v2) const {
  return uniform(getUniformLocation(name), v0, v1, v2);
}
const ShaderProgram& ShaderProgram::uniform(const char* name, float v0,
                                            float v1, float v2,
                                            float v3) const {
  return uniform(getUniformLocation(name), v0, v1, v2, v3);
}
const ShaderProgram& ShaderProgram::uniformMatrix3(const char* name,
                                                   const float* v,
                                                   bool transpose) const {
  return uniformMatrix3(getUniformLocation(name), v, transpose);
}
const ShaderProgram& ShaderProgram::uniformMatrix4(const char* name,
                                                   const float* v,
                                                   bool transpose) const {
  return uniformMatrix4(getUniformLocation(name), v, transpose);
}

const ShaderProgram& ShaderProgram::attribute(int loc, float v) const {
  glVertexAttrib1f(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0,
                                              float v1) const {
  glVertexAttrib2f(loc, v0, v1);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0, float v1,
                                              float v2) const {
  glVertexAttrib3f(loc, v0, v1, v2);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute(int loc, float v0, float v1,
                                              float v2, float v3) const {
  glVertexAttrib4f(loc, v0, v1, v2, v3);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute(const char* name, float v) const {
  return attribute(attribute(name), v);
}
const ShaderProgram& ShaderProgram::attribute(const char* name, float v0,
                                              float v1) const {
  return attribute(attribute(name), v0, v1);
}
const ShaderProgram& ShaderProgram::attribute(const char* name, float v0,
                                              float v1, float v2) const {
  return attribute(attribute(name), v0, v1, v2);
}
const ShaderProgram& ShaderProgram::attribute(const char* name, float v0,
                                              float v1, float v2,
                                              float v3) const {
  return attribute(attribute(name), v0, v1, v2, v3);
}
const ShaderProgram& ShaderProgram::attribute1(const char* name,
                                               const float* v) const {
  glVertexAttrib1fv(attribute(name), v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute2(const char* name,
                                               const float* v) const {
  glVertexAttrib2fv(attribute(name), v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute3(const char* name,
                                               const float* v) const {
  glVertexAttrib3fv(attribute(name), v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute4(const char* name,
                                               const float* v) const {
  glVertexAttrib4fv(attribute(name), v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute1(int loc, const double* v) const {
  glVertexAttrib1dv(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute2(int loc, const double* v) const {
  glVertexAttrib2dv(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute3(int loc, const double* v) const {
  glVertexAttrib3dv(loc, v);
  return *this;
}
const ShaderProgram& ShaderProgram::attribute4(int loc, const double* v) const {
  glVertexAttrib4dv(loc, v);
  return *this;
}

void ShaderProgram::get(int pname, void* params) const {
  glGetProgramiv(id(), pname, (GLint*)params);
}

void ShaderProgram::listParams() const {
  GLuint program = id();
  GLint numActiveUniforms = 0;
  GLint numActiveAttributes = 0;

  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numActiveAttributes);

  printf("ShaderProgram::listParams()\n");

  for (int j = 0; j < numActiveUniforms; j++) {
    GLsizei length;
    GLint size;
    GLenum gltype;
    char name[256];

    glGetActiveUniform(program, j, sizeof(name), &length, &size, &gltype, name);

    // check for array names
    if (name[strlen(name) - 3] == '[' && name[strlen(name) - 1] == ']') {
      name[strlen(name) - 3] = '\0';
    }

    printf("uniform %d(%s): type %d size %d length %d\n", j, name,
           param_type_from_gltype(gltype), size, length);
  }

  for (int j = 0; j < numActiveAttributes; j++) {
    GLsizei length;
    GLint size;
    GLenum gltype;
    char name[256];  // could query for max char length

    glGetActiveAttrib(program, j, sizeof(name), &length, &size, &gltype, name);

    printf("attribute %d(%s): type %d size %d length %d\n", j, name,
           param_type_from_gltype(gltype), size, length);
  }
}

}  // namespace al
