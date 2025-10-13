// -------------------------------------------------------------
// ボリュームフォグ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// ローカルから射影空間への座標変換
float4x4 mWVPT;		// ローカルからテクスチャ空間への座標変換
float4	 vLightDir;	// ライトの位置
float4   vCol;		// メッシュの色

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture DecaleTex;
sampler DecaleMapSamp = sampler_state
{
    Texture = <DecaleTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};


// -------------------------------------------------------------
// -------------------------------------------------------------
// パス１:色バッファと深度バッファの作成
// -------------------------------------------------------------
// -------------------------------------------------------------

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: POSITION;
	float4 Col		: COLOR0;
	float2 Tex		: TEXCOORD0;
	float2 Depth	: TEXCOORD1;
};

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS (
	  float4 Pos	: POSITION          // 頂点位置
	, float4 Normal	: NORMAL			// 法線ベクトル
	, float4 Tex	: TEXCOORD0			// テクスチャ座標
){
	VS_OUTPUT Out = (VS_OUTPUT)0;       // 出力データ
	
	float4 pos = mul( Pos, mWVP );		// 座標変換
	
	Out.Pos = pos;						// 位置座標
	
	Out.Col = vCol * max( dot(vLightDir, Normal), 0);	// 照明計算
	
	Out.Tex = Tex;						// テクスチャ座標
	
	Out.Depth = 0.1f*pos.w;				// 深度
	
	return Out;
}
// -------------------------------------------------------------
// ピクセルシェーダ出力データ
// -------------------------------------------------------------
struct PS_OUTPUT {
	float4 Color : COLOR0;
	float4 Depth : COLOR1;
};
// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
PS_OUTPUT PS ( VS_OUTPUT In ) {
	
	PS_OUTPUT Out = ( PS_OUTPUT ) 0;
	
	// 通常色
	Out.Color = In.Col * tex2D( DecaleMapSamp, In.Tex );
	
	// 深度
	Out.Depth.x = In.Depth;

    return Out;
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

		Sampler[0] = (DecaleMapSamp);
    }
}


// -------------------------------------------------------------
// -------------------------------------------------------------
// パス２，３:フォグマップの作成
// -------------------------------------------------------------
// -------------------------------------------------------------

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture DepthTex;
sampler DepthMapSamp = sampler_state
{
    Texture = <DepthTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
texture FrameBufferTex;
sampler FrameBufferSamp = sampler_state
{
    Texture = <FrameBufferTex>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT_VOLUME
{
	float4 Pos		: POSITION;
	float4 Depth	: COLOR0;
	float4 Tex		: TEXCOORD0;
};
// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT_VOLUME VS_VOLUME (
	  float4 Pos	: POSITION          // 頂点位置
	, float4 Tex	: TEXCOORD0			// テクスチャ座標
){
	VS_OUTPUT_VOLUME Out = (VS_OUTPUT_VOLUME)0;        // 出力データ
	
	float4 pos = mul( Pos, mWVP );	// 座標変換
	
	Out.Pos = pos;					// 位置座標
	
    Out.Tex = mul(Pos, mWVPT); // テクスチャ座標
	
	Out.Depth = 0.1f*pos.w;			// 深度
	
	return Out;
}
// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_VOLUME1( VS_OUTPUT_VOLUME In) : COLOR
{
	float depth_map    = tex2Dproj(    DepthMapSamp, In.Tex ).x;
	float frame_buffer = tex2Dproj( FrameBufferSamp, In.Tex ).x;
	
	return frame_buffer
	 + 10.0f*((depth_map < In.Depth.x) ? depth_map : In.Depth.x);
}

// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_VOLUME2 ( VS_OUTPUT_VOLUME In) : COLOR
{
	float depth_map    = tex2Dproj(    DepthMapSamp, In.Tex ).x;
	float frame_buffer = tex2Dproj( FrameBufferSamp, In.Tex ).x;
	
	return frame_buffer
	 - 10.0f*((depth_map < In.Depth.x) ? depth_map : In.Depth.x);
}

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TVolume
{
    pass P0
    {
		Sampler[0] = (DepthMapSamp);
		Sampler[1] = (FrameBufferSamp);
		
		// レンダリングステート
        CullMode = CW;// 裏面をレンダリング
		Zenable = False;
        
        // シェーダ
        VertexShader = compile vs_1_1 VS_VOLUME ();
        PixelShader  = compile ps_2_0 PS_VOLUME1();

    }
    pass P1
    {
		Sampler[0] = (DepthMapSamp);
		Sampler[1] = (FrameBufferSamp);
		
		// レンダリングステート
        CullMode = CCW;// 前面をレンダリング
		Zenable = False;
        
        // シェーダ
        VertexShader = compile vs_1_1 VS_VOLUME ();
        PixelShader  = compile ps_2_0 PS_VOLUME2();
    }
}

// -------------------------------------------------------------
// -------------------------------------------------------------
// パス４:フォグの合成
// -------------------------------------------------------------
// -------------------------------------------------------------

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture FogMap;
sampler FogMapSamp = sampler_state
{
    Texture = <FogMap>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
texture ColorMap;
sampler ColorMapSamp = sampler_state
{
    Texture = <ColorMap>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT_FINAL
{
	float4 Pos		: POSITION;
	float2 Tex		: TEXCOORD0;
};
// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT_FINAL VS_FINAL (
	  float4 Pos	: POSITION          // 頂点位置
	, float2 Tex	: TEXCOORD0			// テクスチャ座標
){
	VS_OUTPUT_FINAL Out;        // 出力データ
	
	// 位置座標
	Out.Pos = Pos;
	
	// テクスチャ座標
	Out.Tex = Tex;
	
	return Out;
}
// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_FINAL ( VS_OUTPUT_FINAL In) : COLOR
{
	float  fog_map = tex2D( FogMapSamp,   In.Tex ).x;
	float4 col_map = tex2D( ColorMapSamp, In.Tex );
	float4 fog_color = {0.84f, 0.88f, 1.0f, 1.0f};
	
	return lerp(col_map, fog_color, fog_map);
}

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TFinal
{
    pass P0
    {
		Sampler[0] = (FogMapSamp);
		Sampler[1] = (ColorMapSamp);
		
		// レンダリングステート
        AlphaBlendEnable = False;

        // シェーダ
        VertexShader = compile vs_1_1 VS_FINAL();
        PixelShader  = compile ps_2_0 PS_FINAL();
    }
}

// -------------------------------------------------------------
// デバッグ
// -------------------------------------------------------------
float4 PS_DEBUG_DEPTH(VS_OUTPUT_FINAL In) : COLOR
{
    float d = tex2D(DepthMapSamp, In.Tex).x;
    return float4(d, d, d, 1.0f);
}

float4 PS_DEBUG_FOG(VS_OUTPUT_FINAL In) : COLOR
{
    float d = tex2D(FogMapSamp, In.Tex).x;
    return float4(d, d, d, 1.0f);
}

technique TDebugDepth
{
    pass P0
    {
        Sampler[0] = (DepthMapSamp);
        PixelShader = compile ps_2_0 PS_DEBUG_DEPTH();
    }
    pass P0
    {
        Sampler[0] = (FogMapSamp);
        PixelShader = compile ps_2_0 PS_DEBUG_FOG();
    }
}