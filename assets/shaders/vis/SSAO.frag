#version 330 core
out float FragColor;

uniform sampler2D gDepth;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];
uniform mat4 P;
uniform mat4 IP;

uniform vec2 screen_size;

// tile noise texture over screen based on screen dimensions divided by noise size

#define noiseScale1Const 4.0
#define kernelSize 64
uniform float radius = 0.05;
uniform float bias = 0.00;

in vec2 UV;

void main() {
    vec2 noiseScale = screen_size/noiseScale1Const;


    vec4 pos = IP * vec4(gl_FragCoord.xy, texture(gDepth, UV).a, 1);

    vec3 fragPos   = pos.xyz / pos.w;
    vec3 normal    = texture(gNormal, UV).rgb;
    vec3 randomVec = texture(texNoise, UV * noiseScale).xyz;  

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);  

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 sample = TBN * samples[i]; // From tangent to view-space
        sample = fragPos + sample * radius; 
        
        vec4 offset = vec4(sample, 1.0);
        offset      = P * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0  
        
        float sampleDepth = texture(gDepth, offset.xy).x; 

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion       += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
    }  
    
    occlusion = 1.0 - (occlusion / kernelSize);
    FragColor = occlusion;  
}