// ------------------------------------------------------------
// 16ボックスフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float WIDTH;
float HEIGHT;

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
    float4 Pos : POSITION;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float2 Tex2 : TEXCOORD2;
    float2 Tex3 : TEXCOORD3;
};

// ------------------------------------------------------------
// 頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos : POSITION // モデルの頂点
     , float4 Tex : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(-1.0f / WIDTH, -1.0f / HEIGHT);
    Out.Tex1 = Tex + float2(+1.0f / WIDTH, -1.0f / HEIGHT);
    Out.Tex2 = Tex + float2(-1.0f / WIDTH, +1.0f / HEIGHT);
    Out.Tex3 = Tex + float2(+1.0f / WIDTH, +1.0f / HEIGHT);
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR0
{
    // 4つのテクスチャサンプルを取得
    float4 color0 = tex2D(SrcSamp, In.Tex0);
    float4 color1 = tex2D(SrcSamp, In.Tex1);
    float4 color2 = tex2D(SrcSamp, In.Tex2);
    float4 color3 = tex2D(SrcSamp, In.Tex3);
    
    // 4つのサンプルの平均を計算（ボックスフィルター）
    return (color0 + color1 + color2 + color3) * 0.25f;
}

// ------------------------------------------------------------
// ９コーンフィルタサンプリング
// ------------------------------------------------------------
VS_OUTPUT VS9(
      float4 Pos : POSITION // モデルの頂点
     , float4 Tex : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(0.0f / WIDTH, 0.0f / HEIGHT);
    Out.Tex1 = Tex + float2(+1.0f / WIDTH, 0.0f / HEIGHT);
    Out.Tex2 = Tex + float2(0.0f / WIDTH, +1.0f / HEIGHT);
    Out.Tex3 = Tex + float2(+1.0f / WIDTH, +1.0f / HEIGHT);
    
    return Out;
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0 // １６ボックスフィルタサンプリング
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 PS();
        
        // サンプラ
        Sampler[0] = (SrcSamp);
        Sampler[1] = (SrcSamp);
        Sampler[2] = (SrcSamp);
        Sampler[3] = (SrcSamp);
    }
    pass P1 // ９コーンフィルタサンプリング
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS9();
        PixelShader = compile ps_2_0 PS();
        
        // サンプラ
        Sampler[0] = (SrcSamp);
        Sampler[1] = (SrcSamp);
        Sampler[2] = (SrcSamp);
        Sampler[3] = (SrcSamp);
    }
}