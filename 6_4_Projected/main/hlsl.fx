// -------------------------------------------------------------
// 投影テクスチャシャドウ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// ローカルから射影空間への座標変換
float4x4 mWVPT;		// ローカルからテクスチャ空間への座標変換
float4	 vLightPos;	// ライトの位置

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture DecaleMap;
sampler DecaleMapSamp = sampler_state
{
    Texture = <DecaleMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

texture ShadowMap;
sampler ShadowMapSamp = sampler_state
{
    Texture = <ShadowMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};


// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: POSITION;
	float4 Color	: COLOR0;
	float2 TexDecale: TEXCOORD0;
	float4 TexShadow: TEXCOORD1;
};

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS (
	  float4 Pos	: POSITION          // 頂点位置
	, float4 Normal	: NORMAL            // 法線ベクトル
	, float2 Tex	: TEXCOORD0			// テクスチャ座標
){
	VS_OUTPUT Out;        // 出力データ
	
	// 位置座標
	Out.Pos = mul( Pos, mWVP );
	
	Out.Color = max( dot(normalize(vLightPos.xyz-Pos.xyz), Normal), 0);
	
	// テクスチャ座標
	Out.TexDecale = Tex;
	
	// テクスチャ座標
	Out.TexShadow = mul( Pos, mWVPT );
	
	return Out;
}
// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS ( VS_OUTPUT In) : COLOR
{
	float4 decale = tex2D( DecaleMapSamp, In.TexDecale );
	float4 shadow = tex2Dproj( ShadowMapSamp, In.TexShadow );
	
	return decale * (saturate(In.Color-0.5f*shadow)+0.3f);
}
// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}
