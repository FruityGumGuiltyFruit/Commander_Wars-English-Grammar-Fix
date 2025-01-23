varying lowp vec4 result_color;
varying mediump vec2 result_uv;
uniform lowp vec4 add_color;
uniform lowp vec4 brightness_color;
uniform lowp sampler2D base_texture;
uniform mediump float gamma;

void main()
{
    gl_FragColor = texture2D(base_texture, result_uv) * result_color;
    gl_FragColor = gl_FragColor + add_color * gl_FragColor.a;
    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(1.0 / gamma));
    gl_FragColor = gl_FragColor - brightness_color;
}
