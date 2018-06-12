#version 330 core

const vec3 sunDir_c = vec3(1, 0, 0);
const vec3 ground = vec3(0.1, 0.3, 0.6);
in mat4 viewmat;
in vec3 viewDir;

const float PI = 3.1415;

in vec3 vColor;
in vec2 vTex;

in vec4 gl_FragCoord;

out vec4 FragColor;
//out float gl_FragDepth;

vec3 sphereRawNormal(float dist, vec2 uv)
{
    // Generate normal without taking view direction into account
    vec3 BN;
    BN.x = (sin((uv.x + 0.4) * PI / 2.0) - 0.5) * 2.0;
    BN.y = (sin((uv.y + 0.4) * PI / 2.0) - 0.5) * 2.0;
    // This is the height of the sphere
    BN.z = 1.0 - sin(dist * dist * dist);
    return BN;
}

// Corrects bare normal to view space
vec3 sphereNormal(vec3 BN)
{
	return normalize(vec3(vec4(BN, 1.0) * viewmat));
}

vec3 light(vec3 N, vec3 lightDir)
{
    float dpro = dot(N, lightDir);
    
    return vec3(dpro);
}

vec3 atmo(float dist, vec3 L, vec3 N, float around)
{
	float shine = sin(dist * dist) + 0.1;
	float shine_around = dot(reflect(viewDir, N), viewDir);
	if(shine_around < 0.0)
	{
		shine_around = 0.0;
	}
	float light_val = L.x;
	vec3 shine_around_final = shine_around * shine_around * vec3(0.7, 0.2, 0.03) * 4 * (1/(1+pow(2.0*light_val-0.5, 2.0))) * around;
	return vec3(0.6, 0.9, 1.0) * shine * shine * 2 * L + shine_around_final;
}


void main()
{
	vec2 uv = vTex;
    uv.x -= 0.5;
    uv.y -= 0.5;
    
    
    float dist = sqrt(pow(uv.x, 2.0) + pow(uv.y, 2.0)) * 2.4;
   

    if(dist <= 1.0)
    {
        vec3 N = sphereNormal(sphereRawNormal(dist, uv));
        vec3 L = light(N, sunDir_c);
		L = L + atmo(dist, L, N, 1.0);
		L = ground * L;
		FragColor = vec4(L, 1.0);

		float depth = 1.0 - sin(dist * dist * dist);

		// Adds a bit of visual depth to the billboard sphere.
		gl_FragDepth = gl_FragCoord.z - (depth / 100);
    }
    else if(dist < 3.0)
	{
		vec3 N = sphereNormal(sphereRawNormal(dist, uv));
        vec3 L = light(N, sunDir_c);
		L = atmo(dist, L, N, 0.33) * 4;

		float fade = 1.0 - (dist * dist * dist * 0.83);

		FragColor = vec4(L, fade);

		gl_FragDepth = gl_FragCoord.z;
	}
	else
    {
		discard;
    }
	
}