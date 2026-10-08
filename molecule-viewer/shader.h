#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp> 
#include <glm/glm/gtc/type_ptr.hpp>

class Shader{
public:
  Shader(const std::string& pathVertex, const std::string& pathFragment);
  ~Shader();
  
  void SetVertexPath(const std::string& pathVertex);
  void SetFragmentPath(const std::string& pathFragment);
  void Refresh();
  
  void Use() const;
  void SetBool(const std::string& name, const bool& value) const;
  void SetInt(const std::string& name, const int& value) const;
  void SetFloat(const std::string& name, const float& value) const;
  
  void SetVec2(const std::string& name, const glm::vec2& value) const;
  void SetVec3(const std::string& name, const glm::vec3& value) const;
  void SetVec4(const std::string& name, const glm::vec4& value) const;
  
  void SetMat2(const std::string& name, const glm::mat2& mat) const;
  void SetMat3(const std::string& name, const glm::mat3& mat) const;
  void SetMat4(const std::string& name, const glm::mat4& mat) const;

  void setUBO(const char* blockName, const unsigned int& binding) const;

  GLuint getID() {
      return mId;
  }

private:
  GLuint mId;
  std::string mPathVertex;
  std::string mPathFragment;
  
  void Init();
  std::string GetShaderCode(const std::string& path) const;
  std::string ShaderEnumToString(const GLenum& shaderType) const;
  void CheckCompileErrors(const GLuint& shaderId, const GLenum& shaderType) const;
  void CheckLinkingErrors() const;
};

Shader::Shader(const std::string& pathVertex, const std::string& pathFragment)
: mPathVertex(pathVertex),
  mPathFragment(pathFragment)
{
  Init();
}

Shader::~Shader() {
  glDeleteProgram(mId);
}

void Shader::SetVertexPath(const std::string& pathVertex) {
  mPathVertex = pathVertex;
}
void Shader::SetFragmentPath(const std::string& pathFragment) {
  mPathFragment = pathFragment;
}
void Shader::Refresh() {
  glDeleteProgram(mId);
  Init();
}

void Shader::Use() const {
  glUseProgram(mId);
}

void Shader::SetBool(const std::string& name, const bool& value) const {
  glUniform1i(glGetUniformLocation(mId, name.c_str()), (int)value);
}
void Shader::SetInt(const std::string& name, const int& value) const {
  glUniform1i(glGetUniformLocation(mId, name.c_str()), value);
}
void Shader::SetFloat(const std::string& name, const float& value) const {
  glUniform1f(glGetUniformLocation(mId, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const {
  glUniform2fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
void Shader::SetVec3(const std::string& name, const glm::vec3& value) const {
  glUniform3fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}
void Shader::SetVec4(const std::string& name, const glm::vec4& value) const {
  glUniform4fv(glGetUniformLocation(mId, name.c_str()), 1, &value[0]);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& mat) const {
  glUniformMatrix2fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::SetMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void Shader::SetMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(mId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
};

void Shader::setUBO(const char* blockName, const unsigned int &binding) const { //EDIT
    unsigned int index = glGetUniformBlockIndex(mId, blockName);    //would be useful when using opengl 4.2+
    glUniformBlockBinding(mId, index, binding);
}

void Shader::Init() {
  mId = glCreateProgram();
  
  std::string codeVertex = GetShaderCode(mPathVertex);
  const char* cCodeVertex = codeVertex.c_str();
  GLuint shaderIdVertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(shaderIdVertex, 1, &cCodeVertex, NULL);
  glCompileShader(shaderIdVertex);
  CheckCompileErrors(shaderIdVertex, GL_VERTEX_SHADER);
  glAttachShader(mId, shaderIdVertex);
  glDeleteShader(shaderIdVertex);
  
  std::string codeFragment = GetShaderCode(mPathFragment);
  const char* cCodeFragment = codeFragment.c_str();
  GLuint shaderIdFragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(shaderIdFragment, 1, &cCodeFragment, NULL);
  glCompileShader(shaderIdFragment);
  CheckCompileErrors(shaderIdFragment, GL_FRAGMENT_SHADER);
  glAttachShader(mId, shaderIdFragment);
  glDeleteShader(shaderIdFragment);

  glLinkProgram(mId);
  CheckLinkingErrors();
}

std::string Shader::GetShaderCode(const std::string& path) const {
  std::string code = "";
  try{
    std::ifstream sourceFile(path);
    if(!sourceFile.is_open()) throw std::string(path + " not found");
    std::string line;
    while(std::getline(sourceFile, line)) code+= line + "\n";
    sourceFile.close();
  }
  catch(std::string fail) {
    std::cout << "[Error] " << fail << '\n';
  }
  return code;
}

std::string Shader::ShaderEnumToString(const GLenum& shaderType) const {
  switch(shaderType) {
    case GL_VERTEX_SHADER: return "Vertex Sahder";
    case GL_FRAGMENT_SHADER: return "Fragment Shader";
  }
  return "Undefined Shader";
}
void Shader::CheckCompileErrors(const GLuint& shaderId, const GLenum& shaderType) const {
  GLint success;
  GLchar infoLog[1024];
  glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
  if(not success) {
    glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);
    std::cout << "[Error] " << ShaderEnumToString(shaderType) << " compilation failed: " << "\n" << infoLog << std::endl;
  }
}
void Shader::CheckLinkingErrors() const {
  GLint success;
  GLchar infoLog[1024];
  glGetProgramiv(mId, GL_LINK_STATUS, &success);
  if(not success) {
    glGetProgramInfoLog(mId, 1024, NULL, infoLog);
    std::cout << "[Error] Shader programm linking failed: " << "\n" << infoLog << std::endl;
  }
}

#endif // SHADER_H
