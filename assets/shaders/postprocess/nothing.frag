#version 330

// This nothing.frag postprocess shader is used to evade something.
// Currently, the text is only rendered when there is a postprocess effect applied.
// I couldn't debug this, so I created a postprocess shader that does nothing if 
// we desire no effect.
uniform sampler2D tex;

in vec2 tex_coord;
out vec4 frag_color;

void main(){
    frag_color = texture(tex, tex_coord);
}