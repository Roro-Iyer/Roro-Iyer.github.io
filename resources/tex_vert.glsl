#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightPos3;
uniform vec3 lightPos4;



out vec3 fragNor;
out vec3 lightDir;
out vec3 lightDir1;
out vec3 lightDir2;
out vec3 lightDir3;
out vec3 lightDir4;

out vec3 EPos;
out vec2 vTexCoord;
out vec3 lightposition;
out vec3 lightposition1;
out vec3 lightposition2;
out vec3 lightposition4;


void main() {

  /* First model transforms */
  vec3 wPos = vec3(M * vec4(vertPos.xyz, 1.0));
  gl_Position = P * V *M * vec4(vertPos.xyz, 1.0);
  
  fragNor = (M * vec4(vertNor, 0.0)).xyz;
  EPos = (M*vec4(vertPos.xyz, 1.0)).xyz;
  lightDir = lightPos - EPos;
	lightDir1 = lightPos1 - EPos;
	lightDir2 = lightPos2 - EPos;
  lightDir3 = lightPos3;
  lightDir4 = lightPos4 - EPos;
  
  /* pass through the texture coordinates to be interpolated */
  vTexCoord = vertTex;

  lightposition = lightPos;
  lightposition1 = lightPos1;
  lightposition2 = lightPos2;
  lightposition4 = lightPos4;
}
