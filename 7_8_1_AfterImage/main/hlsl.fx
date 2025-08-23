// ------------------------------------------------------------
// 残像
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
    float4 Pos			: POSITION;
	float2 Tex			: TEXCOORD0;
};

// ------------------------------------------------------------
// 頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS (
      float4 Pos    : POSITION,          // モデルの頂点
      float4 Tex    : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex = Tex;
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS (VS_OUTPUT In) : COLOR
{   
    float4 Out = tex2D( SrcSamp, In.Tex );
	
	Out.a = 0.9f;
	
	
    return Out;
}
// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
        
		Sampler[0] = (SrcSamp);
		
        // レンダーステート
        AlphaBlendEnable = True;
        SrcBlend = SrcAlpha;
        DestBlend = InvSrcAlpha;
    }
}
