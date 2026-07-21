[shader("vertex")]
float4 vertMain() : SV_Position {
  return float4(0, 0, 0, 1);
}

struct GSInput {
  float4 position : SV_Position;
};

struct GSOutput {
  float4 position : SV_Position;
};

[shader("geometry")]
[maxvertexcount(3)]
void geomMain(
    point GSInput input[1],
    inout TriangleStream<GSOutput> outputStream
) {
  GSOutput res;
  res.position = float4(-1, -1, 0, 1);
  outputStream.Append(res);
  res.position = float4(-1, 3, 0, 1);
  outputStream.Append(res);
  res.position = float4(3, -1, 0, 1);
  outputStream.Append(res);

  outputStream.RestartStrip();
}

layout(set = 2) Sampler2D texture;

[shader("fragment")]
float4 fragMain(float4 pos: SV_Position) : SV_Target0 {
  const int2 sc = int2(pos.xy);
  return texture.Load(int3(sc, 0));
}
