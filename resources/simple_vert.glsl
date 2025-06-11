#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform vec3 lightPos;
uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightPos3;
uniform vec3 lightPos4;


//keep these and set them correctly
out vec3 fragNor;
out vec3 lightDir;
out vec3 lightDir1;
out vec3 lightDir2;
out vec3 lightDir3;
out vec3 lightDir4;
out vec3 EPos;
out vec3 lightposition;
out vec3 lightposition1;
out vec3 lightposition2;
out vec3 lightposition4;




void main()
{
	gl_Position = P * V * M * vertPos;
	
	//update these as needed
	fragNor = (M * vec4(vertNor, 0.0)).xyz; 
	lightDir = lightPos - (M*vertPos).xyz;
	lightDir1 = lightPos1 - (M*vertPos).xyz;
	lightDir2 = lightPos2 - (M*vertPos).xyz;
	lightDir4 = lightPos4 - (M*vertPos).xyz;
	EPos = (M*vertPos).xyz;
	lightposition = lightPos;
	lightposition1 = lightPos1;
	lightposition2 = lightPos2;
	lightDir3 = lightPos3;
	lightposition4 = lightPos4;
}
