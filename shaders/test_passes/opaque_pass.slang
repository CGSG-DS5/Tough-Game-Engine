struct VSInput {
  float3 pos : POSITION;
  float4 color : COLOR;
};

struct PSInput {
  float4 pos : SV_Position;
  float4 color : COLOR0;
};

[shader("vertex")]
PSInput vertMain(VSInput input) {
  PSInput output;
  output.pos = float4(input.pos, 1);
  output.color = input.color;
  return output;
}

[shader("fragment")]
float4 fragMain(PSInput input) : SV_Target0 {
  return input.color;
}
