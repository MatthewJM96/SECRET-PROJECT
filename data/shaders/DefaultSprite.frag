#version 330

// Uniforms - things that are the same for all vertices.
uniform sampler2D SpriteTexture;

// Data about this specific pixel (corresponds to the data we
// sent here from the vertex shader).
     in vec2 fRelativePosition;
flat in vec4 fUVDimensions;
     in vec4 fColour;

// The final colour of this pixel, this gets sent to the
// framebuffer which will be rendered to the screen.
out vec4 finalColour;

void main() {
    // Calculate the coordinates of the pixel to be taken from our texture.
    //     fUVDimensions represents the rectangle of pixels within the texture to be used,
    //     where .xy represent the bottom-left coords of that rectangle within the texture
    //     and .zw represent the size of the rectangle growing towards the top-right.
    //     fRelativePosition is the relative position of this pixel within our Sprite's 
    //     rectangle.
    vec2 textureCoords = fract(fRelativePosition.xy) * fUVDimensions.zw + fUVDimensions.xy;

    // The final colour is simply calculated as the product of the colour of 
    // the texture's pixel and the "tint" colour to be applied to that point 
    // on the sprite.
    //    Note: At some point fColour will be converted from unsigned byte to float, as it is used to multiply texture it is likely normalising this to be in [0, 1].
    finalColour = texture(SpriteTexture, textureCoords) * fColour;
}
