// ------------------------------------------------------------
// セピアフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float t;

// ------------------------------------------------------------
// テクスチャ
// ------------------------------------------------------------
texture SrcMap;
sampler SrcSamp = sampler_state
{
    Texture = <SrcMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// ------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// ------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos            : POSITION;
    float2 Tex            : TEXCOORD0;
};

// ------------------------------------------------------------
// 頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS (
      float4 Pos    : POSITION,          // モデルの頂点
      float4 Tex    : TEXCOORD0             // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    // テクスチャ座標
    Out.Tex = Tex;
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
    float4 Out = (float4)0;
    float3 YCbCr;
    
    const float3 RGB2Y  = {0.29900, 0.58700, 0.11400};
    const float3 Cb2RGB = { 0.00000f, -0.34414f, 1.77200f};
    const float3 Cr2RGB = {+1.40200f, -0.71414f, 0.00000f};

    float3 Color = tex2D( SrcSamp, In.Tex ).xyz; // 元になる色
    
    YCbCr.x = dot(Color, RGB2Y);    // Y
    YCbCr.y = -0.2f;				// Cb
    YCbCr.z =  0.1f;				// Cr
    
    float3 Sepia = YCbCr.x
				 + mul(Cb2RGB, YCbCr.y)
				 + mul(Cr2RGB, YCbCr.z);// YCbCr から RGB へ
    
    Out.rgb = lerp(Color, Sepia, t); // 元の色と補間
    return Out;
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}
