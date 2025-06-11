#version 330 core
uniform sampler2D Texture0;
uniform int flip;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float constant;
uniform float linear;
uniform float quadratic;
uniform float constant1;
uniform float linear1;
uniform float quadratic1;
uniform float lightI;
uniform float lightI1;
uniform vec3 camPos;
uniform float MatShine;
uniform vec3 lightArray[60];

in vec2 vTexCoord;
out vec4 Outcolor;
in vec3 fragNor;
in vec3 lightDir;
in vec3 lightDir1;
in vec3 lightDir2;
in vec3 lightDir3;
in vec3 lightDir4;

in vec3 EPos;
in vec3 lightposition;
in vec3 lightposition1;
in vec3 lightposition2;
in vec3 lightposition4;


vec3 CalcDirLight(vec3 lightdirection, vec3 fragnormal, vec3 fragposition)
{	
	vec3 normal = normalize(fragnormal);
	vec3 viewDir = normalize(-fragposition);
    vec3 lightDir = normalize(-lightdirection);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), MatShine);
    // combine results
    vec3 ambient  = MatAmb;
    vec3 diffuse  = MatDif  * diff;
    vec3 specular = MatSpec * spec;
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(vec3 fragnormal, vec3 lightdirection, vec3 lightpos, vec3 fragposition){
	vec3 normal = normalize(fragnormal);
	vec3 light = normalize(lightdirection);
	float dC = dot(normal,light);
	if (flip == 0){
		normal = normalize((-1*fragNor));
		light = normalize(lightDir);
		dC = dot(normal,light);
	}
	
	float lambertian = max(dot(normal,light),0.0);
	float rC = 0.0;
	if (lambertian > 0.0){
		vec3 viewDir = normalize(-fragposition);
		vec3 reflection = reflect(-light,normal);
		vec3 halfway = normalize(light + viewDir);
		float specangle = max(dot(normal, halfway), 0.0);
		rC = pow(specangle,MatShine);
	}

	float distance  = length(lightpos - fragposition);
	float attenuation = 1.0 /(constant + linear * distance + quadratic * (distance * distance));

	vec3 ambient = MatAmb;
	vec3 diffuse = dC*MatDif*lambertian;
	vec3 specular = MatSpec*rC;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalcPointLight1(vec3 fragnormal, vec3 lightdirection, vec3 lightpos, vec3 fragposition){
	vec3 normal = normalize(fragnormal);
	vec3 light = normalize(lightdirection);
	float dC = dot(normal,light);
	if (flip == 0){
		normal = normalize((-1*fragNor));
		light = normalize(lightDir);
		dC = dot(normal,light);
	}
	
	float lambertian = max(dot(normal,light),0.0);
	float rC = 0.0;
	if (lambertian > 0.0){
		vec3 viewDir = normalize(-fragposition);
		vec3 reflection = reflect(-light,normal);
		vec3 halfway = normalize(light + viewDir);
		float specangle = max(dot(normal, halfway), 0.0);
		rC = pow(specangle,MatShine);
	}

	float distance  = length(lightpos - fragposition);
	float attenuation = 1.0 /(constant1 + linear1 * distance + quadratic1 * (distance * distance));

	vec3 ambient = MatAmb;
	vec3 diffuse = dC*MatDif*lambertian;
	vec3 specular = MatSpec*rC;

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}


void main() {
  	vec4 texColor0 = texture(Texture0, vTexCoord);
  	
	vec3 returncolor;
	vec3 curdir = vec3(0.0, 0.0, 0.0);

	returncolor = lightI*CalcPointLight(fragNor, lightDir, lightposition, EPos);
	returncolor += CalcPointLight(fragNor, lightDir1, lightposition1, EPos);
	returncolor += lightI1*CalcPointLight(fragNor, lightDir2, lightposition2, EPos);
	returncolor += 20.0*CalcPointLight(fragNor, lightDir4, lightposition4, EPos);
	returncolor += 0.1*CalcDirLight(lightDir3, fragNor, EPos);

	for (int i = 0; i < 60; i++){
		curdir = lightArray[i] - EPos.xyz;
		returncolor += 0.5*CalcPointLight1(fragNor, curdir, lightArray[i], EPos);  
	}

	Outcolor = texColor0*vec4(returncolor, 1.0);
	//Outcolor = dC*texColor0;
	
  
  	//to set the outcolor as the texture coordinate (for debugging)
	//Outcolor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);
}

