// ------------------------------------------------------------
// セピアフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

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
    float4 OutColor = 0;
    float3 YCbCr;
    
    const float3 RGB2Y = {0.29900, 0.58700, 0.11400};

    float3 Color = tex2D( SrcSamp, In.Tex ).xyz; // 元になる色
    
    OutColor.rgb = dot(Color, RGB2Y); // Yをそのまま色へ
    OutColor.a = 1.0f; // アルファ値は1.0に設定
    return OutColor;
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
