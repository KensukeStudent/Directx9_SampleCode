// -------------------------------------------------------------
// シャドウマップ
// 
// Copyright (c) 2002-2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// ローカルから射影空間への座標変換
float4x4 mWLP;		// ローカルから射影空間への座標変換
float4x4 mWLPB;		// テクスチャ座標系への射影
float4   vCol;		// メッシュの色
float4	 vLightDir;	// ライトの方向

float4x4 mWVP_ufo; // ローカルから射影空間への座標変換
float4x4 mWLP_ufo; // ローカルから射影空間への座標変換
float4x4 mWLPB_ufo; // テクスチャ座標系への射影
float4 vCol_ufo; // メッシュの色
float4 vLightDir_ufo; // ライトの方向

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
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
// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
	float4 Diffuse		: COLOR0;
	float4 Ambient		: COLOR1;
	float4 ShadowMapUV	: TEXCOORD0;
	float4 Depth		: TEXCOORD1;
	float2 DecaleTex	: TEXCOORD2;
};

// -------------------------------------------------------------
// 0パス目：頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_Ground0(
      float4 Pos    : POSITION,          // モデルの頂点
      float3 Normal : NORMAL	         // モデルの法線
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 座標変換
	float4 pos = mul( Pos, mWLP );
	
    // 位置座標
    Out.Pos = pos;
    
    // カメラ座標系での深度をテクスチャに入れる
    Out.ShadowMapUV = pos;

    return Out;
}

// -------------------------------------------------------------
// 1パス目：頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_Ufo1(
      float4 Pos : POSITION, // モデルの頂点
      float3 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 座標変換
    float4 pos = mul(Pos, mWLP_ufo);
	
    // 位置座標
    Out.Pos = pos;
    
    // カメラ座標系での深度をテクスチャに入れる
    Out.ShadowMapUV = pos;

    return Out;
}

// -------------------------------------------------------------
// 0,1パス目：ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_Common01(VS_OUTPUT In) : COLOR
{   
    float4 Out;
    
    Out = In.ShadowMapUV.z / In.ShadowMapUV.w;
    
    return Out;
}

// -------------------------------------------------------------
// 2パス目：ピクセルシェーダプログラム (テクスチャあり)
// -------------------------------------------------------------
VS_OUTPUT VS_Ground2(
      float4 Pos    : POSITION,          // モデルの頂点
      float4 Normal : NORMAL,	         // モデルの法線
      float2 Tex    : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	float4	uv;
	
	// 座標変換
    Out.Pos = mul(Pos, mWVP);
	// 色
	Out.Diffuse = vCol * max( dot(vLightDir, Normal), 0);// 拡散色
	Out.Ambient = vCol * 0.3f;						     // 環境色
	
	// シャドウマップ
	Out.ShadowMapUV = mul(Pos, mWLPB);
	
	// 比較のための深度値
	Out.Depth       = mul(Pos, mWLP);
		
	// デカールテクスチャ
	Out.DecaleTex   = Tex;
		
    return Out;
}
float4 PS_pass2(VS_OUTPUT In) : COLOR
{   
    float4 Color;
	float  shadow = tex2Dproj( ShadowMapSamp, In.ShadowMapUV ).x;
	float4 decale = tex2D( DecaleMapSamp, In.DecaleTex );
    
    Color = In.Ambient
		 + ((shadow * In.Depth.w < In.Depth.z-0.03f) ? 0 : In.Diffuse);

    return Color * decale;
}  

// -------------------------------------------------------------
// 3パス目：ピクセルシェーダプログラム (テクスチャなし)
// -------------------------------------------------------------
VS_OUTPUT VS_Ufo3(
      float4 Pos : POSITION, // モデルの頂点
      float4 Normal : NORMAL, // モデルの法線
      float2 Tex : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    float4 uv;
	
	// 座標変換
    Out.Pos = mul(Pos, mWVP_ufo);
	// 色
    Out.Diffuse = vCol_ufo * max(dot(vLightDir_ufo, Normal), 0); // 拡散色
    Out.Ambient = vCol_ufo * 0.3f; // 環境色
	
	// シャドウマップ
    Out.ShadowMapUV = mul(Pos, mWLPB_ufo);
	
	// 比較のための深度値
    Out.Depth = mul(Pos, mWLP_ufo);
		
	// デカールテクスチャ
    Out.DecaleTex = Tex;
		
    return Out;
}
float4 PS_pass3(VS_OUTPUT In) : COLOR
{   
    float4 Color;
	float  shadow = tex2Dproj( ShadowMapSamp, In.ShadowMapUV ).x;
    
    Color = In.Ambient
		 + ((shadow * In.Depth.w < In.Depth.z-0.03f) ? 0 : In.Diffuse);

    return Color;
}  

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0// シャドウマップの作成
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_Ground0();
        PixelShader  = compile ps_2_0 PS_Common01();
    }
    pass P1 // シャドウマップの作成
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_Ufo1();
        PixelShader = compile ps_2_0 PS_Common01();
    }
    pass P2// テクスチャあり 地面
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_Ground2();
        PixelShader = compile ps_2_0 PS_pass2();
    }
    pass P3// テクスチャなし UFO
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_Ufo3();
        PixelShader  = compile ps_2_0 PS_pass3();
    }
}
