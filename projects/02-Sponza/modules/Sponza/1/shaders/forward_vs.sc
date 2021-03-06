$input a_position, a_texcoord0, a_normal
$output v_texcoord0, v_normal

#include <bgfx_shader.sh>

void main()
{
    // Standard: Vertex Position
    vec4 vertPosition = vec4(a_position, 1.0);

    // Standard: UV Coordinates
    v_texcoord0 = a_texcoord0;

    v_normal = a_normal.xyz;

    // Standard: Output Final Vertex Position
    gl_Position = mul(u_modelViewProj, vertPosition);
}
