// -------------------------------------------------------------
// プライオリティバッファシャドウ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// ローカルから射影空間への座標変換
float4x4 mWLP;		// ローカルから射影空間への座標変換
float4x4 mWVPT;		// テクスチャ座標系への射影
float4   vCol;		// メッシュの色
float4   vId;		// プライオリティ番号
float4	 vLightDir;	// ライトの方向

float4x4 mWVP_ufo; // ローカルから射影空間への座標変換
float4x4 mWLP_ufo; // ローカルから射影空間への座標変換
float4x4 mWVPT_ufo; // テクスチャ座標系への射影
float4 vCol_ufo; // メッシュの色
float4 vId_ufo; // プライオリティ番号
float4 vLightDir_ufo; // ライトの方向

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture IdMap;
sampler IdMapSamp = sampler_state
{
    Texture = <IdMap>;
    MinFilter = POINT;
    MagFilter = POINT;
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
	float4 ID			: TEXCOORD1;
	float2 TexDecale	: TEXCOORD2;
};


// -------------------------------------------------------------
// 1パス目：頂点シェーダプログラム シャドーマップ UFO
// -------------------------------------------------------------
VS_OUTPUT VS_01(
      float4 Pos : POSITION, // モデルの頂点
      float3 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 位置座標
    Out.Pos = mul(Pos, mWLP_ufo);
    
    // IDを色として出力する
    Out.Diffuse = vId;

    return Out;
}

// -------------------------------------------------------------
// 1パス目：頂点シェーダプログラム シャドーマップ 地面
// -------------------------------------------------------------
VS_OUTPUT VS_02(
      float4 Pos    : POSITION,          // モデルの頂点
      float3 Normal : NORMAL	         // モデルの法線
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 位置座標
    Out.Pos =  mul( Pos, mWLP );
    
    // IDを色として出力する
    Out.Diffuse = vId;

    return Out;
}

// -------------------------------------------------------------
// 1パス目：ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_pass0(VS_OUTPUT In) : COLOR
{
    return In.Diffuse; // ID値を色として出力
}



// 地面


// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_1(
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
	Out.Ambient = vCol * 0.3f;							 // 環境色
	
	// テクスチャ座標
	uv = mul(Pos, mWVPT);
	Out.ShadowMapUV = uv;

	// ID 値
	Out.ID = vId;
	
	// デカールテクスチャ
	Out.TexDecale = Tex;
		
    return Out;
}

// -------------------------------------------------------------
// 1パス目：ピクセルシェーダプログラム(テクスチャあり) 地面
// -------------------------------------------------------------
float4 PS_pass1(VS_OUTPUT In) : COLOR
{   
    float4 Color = In.Ambient;
    float4 zero = { 0, 0, 0, 0 };
    float ofset = 1.0f / 256.0f;
    
    float id_map = tex2Dproj(IdMapSamp, In.ShadowMapUV);
    float4 decale = tex2D(DecaleMapSamp, In.TexDecale);
    
    Color += (id_map.x < In.ID.x + ofset && In.ID.x - ofset < id_map.x)
				 ? In.Diffuse : zero;

    return decale * Color;
}


// UFO

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_2(
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
	
	// テクスチャ座標
    uv = mul(Pos, mWVPT_ufo);
    Out.ShadowMapUV = uv;

	// ID 値
    Out.ID = vId_ufo;
	
	// デカールテクスチャ
    Out.TexDecale = Tex;
		
    return Out;
}

// -------------------------------------------------------------
// 2パス目：ピクセルシェーダプログラム(テクスチャなし) UFO
// -------------------------------------------------------------
float4 PS_pass2(VS_OUTPUT In) : COLOR
{   
    float4 Color = In.Ambient;
    float4 zero = { 0, 0, 0, 0 };
    float ofset = 0.01f;
    
    float id_map = tex2Dproj(IdMapSamp, In.ShadowMapUV);
    float4 decale = tex2D(DecaleMapSamp, In.TexDecale);
    
    Color += (id_map.x < In.ID.x + ofset && In.ID.x - ofset < id_map.x)
				 ? In.Diffuse : zero;

    return Color;
}  
// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0 // ufo シャドーマップ
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_01();
        PixelShader = compile ps_2_0 PS_pass0(); // ← HLSL 版に切り替え
    }
    pass P1 // 地面 シャドーマップ
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_02();
        PixelShader = compile ps_2_0 PS_pass0(); // ← HLSL 版に切り替え
    }
    pass P2 // 地面
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_1();
        PixelShader = compile ps_2_0 PS_pass1();
    }
    pass P3 // UFO
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_2();
        PixelShader = compile ps_2_0 PS_pass2();
    }
}