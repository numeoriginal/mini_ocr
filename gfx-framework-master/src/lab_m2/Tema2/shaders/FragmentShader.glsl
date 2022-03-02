#version 410

// Input
layout(location = 0) in vec2 texture_coord;
precision mediump float;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture

float red_v[9], green_v[9], blue_v[9];
vec3 channels;


float insertion_sort(float a[9])
{
   int i, j;
   for (i = 0; i < 8; ++i){
	   for(j = 0; j < 8 - i; ++j){
		if (a[j] > a[j+1]){
			float aux = a[j];
			a[j] = a[j + 1];
			a[j + 1] = aux;
		}
	   }
   }
   return a[4];
}

vec4 median(){
	vec2 t_size = 1.0f / screenSize;
	vec4 color = texture(textureImage, textureCoord);
	if (textureCoord.x == 0 || textureCoord.y == 0 || textureCoord.y == screenSize.y - 1 || textureCoord.x == screenSize.x - 1) {
		return vec4(color.r, color.g, color.b,0);
	} else {
		
		red_v[0] = texture(textureImage, textureCoord + vec2(0, 0) * t_size).r;
		green_v[0] = texture(textureImage, textureCoord + vec2(0, 0) * t_size).g;
		blue_v[0] = texture(textureImage, textureCoord + vec2(0, 0) * t_size).b;

		red_v[1] = texture(textureImage, textureCoord + vec2(1, 0) * t_size).r;
		green_v[1] = texture(textureImage, textureCoord + vec2(1, 0) * t_size).g;
		blue_v[1] = texture(textureImage, textureCoord + vec2(1, 0) * t_size).b;

		red_v[2] = texture(textureImage, textureCoord + vec2(-1, 0) * t_size).r;
		green_v[2] = texture(textureImage, textureCoord + vec2(-1, 0) * t_size).g;
		blue_v[2] = texture(textureImage, textureCoord + vec2(-1, 0) * t_size).b;

		red_v[3] = texture(textureImage, textureCoord + vec2(0, 1) * t_size).r;
		green_v[3] = texture(textureImage, textureCoord + vec2(0, 1) * t_size).g;
		blue_v[3] = texture(textureImage, textureCoord + vec2(0, 1) * t_size).b;

		red_v[4] = texture(textureImage, textureCoord + vec2(1, 1) * t_size).r;
		green_v[4] = texture(textureImage, textureCoord + vec2(1, 1) * t_size).g;
		blue_v[4] = texture(textureImage, textureCoord + vec2(1, 1) * t_size).b;

		red_v[5] = texture(textureImage, textureCoord + vec2(-1, 1) * t_size).r;
		green_v[5] = texture(textureImage, textureCoord + vec2(-1, 1) * t_size).g;
		blue_v[5] = texture(textureImage, textureCoord + vec2(-1, 1) * t_size).b;

		red_v[6] = texture(textureImage, textureCoord + vec2(0, -1) * t_size).r;
		green_v[6] = texture(textureImage, textureCoord + vec2(0, -1) * t_size).g;
		blue_v[6] = texture(textureImage, textureCoord + vec2(0, -1) * t_size).b;

		red_v[7] = texture(textureImage, textureCoord + vec2(1, -1) * t_size).r;
		green_v[7] = texture(textureImage, textureCoord + vec2(1, -1) * t_size).g;
		blue_v[7] = texture(textureImage, textureCoord + vec2(1, -1) * t_size).b;

		red_v[8] = texture(textureImage, textureCoord + vec2(-1, -1) * t_size).r;
		green_v[8] = texture(textureImage, textureCoord + vec2(-1, -1) * t_size).g;
		blue_v[8] = texture(textureImage, textureCoord + vec2(-1, -1) * t_size).b;

		float color_r = insertion_sort(red_v);
		float color_g = insertion_sort(green_v);
		float color_b = insertion_sort(blue_v);
		
		return vec4(color_r, color_g, color_b, 0);
	}

}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main()
{
    switch (outputMode)
    {
        
		case 5:
		{
			out_color = median();
			vec4 textureColor = texture2D(textureImage, textureCoord);
			vec3 fragRGB = out_color.rgb;
			vec3 fragHSV = rgb2hsv(fragRGB).xyz;
			fragHSV.xyz = mod(fragHSV.xyz, 1.0);
			if (fragHSV.x > 0.225 && fragHSV.x < 0.38) {
				out_color = vec4(1);
			} else {
				out_color = vec4(vec3(0), 1);
			}

			break;
		}

        default:
            out_color = texture(textureImage, textureCoord);
            break;
    }
}
