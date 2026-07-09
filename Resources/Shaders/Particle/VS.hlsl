#include "Particle.hlsl"

VS_OUT VS(uint id : SV_VertexID) {
    Particle p = particles[id];

    VS_OUT o;
    o.Position = p.Position;
    o.Color = p.Color;
    o.TexIndex = p.TextureIndex;
    o.Size = p.Size;

    return o;
}