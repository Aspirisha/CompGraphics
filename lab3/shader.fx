cbuffer ConstantBuffer
{
	matrix World;
	matrix View;
	matrix Projection;
  float4 vLightColor[2];
  float4 vLightDir[2];
  float4 vLightPos[2];
  float4 vBulbLightColor[1];
  float4 vBulbLightPos[1];
  float4 vDirectedLightColor[1];
  float4 vDirectedLightDir[1];
  uint4 isLightEnabled; // 4th component is used to define if texture enabled
  //float vLightAngleX[2];
  //float vLightAngleY[2];
  //uint4 numberOfLights
	float4 vOutputColor;
}

Texture2D g_MeshTexture;            // Color texture for mesh

struct VS_INPUT
{
  float4 Pos : POSITION;
  float3 Norm : NORMAL;
  float4 Color: COLOR;
  float3 Tex : TEXCOORD; 
};

struct PS_INPUT
{
  float4 oldPos : POSITION;
  float4 Pos : SV_POSITION;
  float4 Color: COLOR;
  float3 Norm : TEXCOORD0;
  float3 Tex : TEXCOORD1; 
};

SamplerState MeshTextureSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

PS_INPUT VS( VS_INPUT input )
{
  PS_INPUT output = (PS_INPUT)0;
  output.oldPos = mul(input.Pos, World);
  output.Pos = mul( input.Pos, World );
  output.Pos = mul( output.Pos, View );
  output.Pos = mul( output.Pos, Projection );
  output.Norm = mul( input.Norm, World );
  output.Color = input.Color;
  output.Tex = input.Tex;
  return output;
}

float4 PS( PS_INPUT input) : SV_Target
{
  float4 finalColor = 0;

  if (isLightEnabled[3] == 0)
    finalColor = 0.3 * input.Color;	
  else
    finalColor = g_MeshTexture.Sample(MeshTextureSampler, input.Tex);

  input.Norm = normalize(input.Norm);
     
  if (isLightEnabled[0] != 0)
  {
    for (int i = 0; i < 2; i++)
    {
      float3 dr = (input.oldPos - vLightPos[i]);
      float dist2 = dot(dr, dr);
      float dist = sqrt(dist2);
      dr /= dist; // normalize dr
      float cosa = dot(vLightDir[i], dr);
    
      if (cosa > cos(0.4)) // hardcoded now
        finalColor += saturate( dot( -dr,input.Norm) / (dist * 0.1) * vLightColor[i] * cosa );
    }
  }

  if (isLightEnabled[1] != 0)
  {
    for (int i = 0; i < 1; i++)
    {
      float3 dr = (input.oldPos - vBulbLightPos[i]);
      float dist2 = dot(dr, dr);

      finalColor += saturate( dot( -dr, input.Norm) / (dist2 * 0.1) * vBulbLightColor[i]);
    }
  }

  if (isLightEnabled[2] != 0)
  {
    for (int i = 0; i < 1; i++)
    {
      float4 normalizedDir = normalize(vDirectedLightDir[i]);
      finalColor += saturate( dot( -normalizedDir,input.Norm) * vDirectedLightColor[i] );
    }
  }

  return finalColor;
}

float4 PSSolid( PS_INPUT input) : SV_Target
{
    return input.Color;
}