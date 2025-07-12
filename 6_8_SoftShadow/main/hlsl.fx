// -------------------------------------------------------------
// ソフトシャドウ
// 
// Copyright (c) 2002-2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------

float4x4 mWVP_box;		// ローカルから射影空間への座標変換
float4x4 mWLP_box; // ローカルから射影空間への座標変換
float4x4 mWLPB_box; // テクスチャ座標系への射影
float4 vCol_box; // メッシュの色

float4x4 mWVP_bg; // ローカルから射影空間への座標変換
float4x4 mWLP_bg; // ローカルから射影空間への座標変換
float4x4 mWLPB_bg; // テクスチャ座標系への射影
float4 vCol_bg; // メッシュの色

float4	 vLightDir;	// ライトの方向

static float MAP_SIZE = 512.0f;

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
// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
	float4 Diffuse		: COLOR0;
	float4 Ambient		: COLOR1;
	float4 Depth		: TEXCOORD0;
	float4 ShadowMapUV	: TEXCOORD1;
};
// -------------------------------------------------------------
struct VS_OUTPUT_QUAD
{
    float4 Pos			: POSITION;
	float2 Tex0			: TEXCOORD0;
	float2 Tex1			: TEXCOORD1;
	float2 Tex2			: TEXCOORD2;
	float2 Tex3			: TEXCOORD3;
	float2 Tex4			: TEXCOORD4;
	float2 Tex5			: TEXCOORD5;
	float2 Tex6			: TEXCOORD6;
	float2 Tex7			: TEXCOORD7;
};

// -------------------------------------------------------------
// 1パス目：深度マップの作成 (シャドウマップ) technique: BoxAndBgTShader
// -------------------------------------------------------------

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_boxShadow(
      float4 Pos    : POSITION,          // モデルの頂点
      float3 Normal : NORMAL	         // モデルの法線
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 座標変換
	float4 pos = mul( Pos, mWLP_box );
	
    // 位置座標
    Out.Pos = pos;
    
    // カメラ座標系での深度をテクスチャに入れる
    Out.Depth = pos.zzzw;

    return Out;
}

VS_OUTPUT VS_bgShadow(
      float4 Pos : POSITION, // モデルの頂点
      float3 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 座標変換
    float4 pos = mul(Pos, mWLP_bg);
	
    // 位置座標
    Out.Pos = pos;
    
    // カメラ座標系での深度をテクスチャに入れる
    Out.Depth = pos.zzzw;

    return Out;
}

// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_pass0(VS_OUTPUT In) : COLOR
{   
    float4 Color = In.Depth / In.Depth.w;

    return Color;
}  

// -------------------------------------------------------------
// 2パス目：合成
// -------------------------------------------------------------

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_boxDraw(
      float4 Pos : POSITION, // モデルの頂点
      float4 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    float4 uv;
	
	// 座標変換
    Out.Pos = mul(Pos, mWVP_box);
	// 色
    Out.Diffuse = vCol_box * max(dot(vLightDir, Normal), 0); // 拡散色
    Out.Ambient = vCol_box * 0.3f; // 環境色
	
	// テクスチャ座標
    uv = mul(Pos, mWLPB_box);
    Out.ShadowMapUV = uv;
    uv = mul(Pos, mWLP_box);
    Out.Depth = uv.zzzw;
		
    return Out;
}

VS_OUTPUT VS_bgDraw(
      float4 Pos : POSITION, // モデルの頂点
      float4 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    float4 uv;
	
	// 座標変換
    Out.Pos = mul(Pos, mWVP_bg);
	// 色
    Out.Diffuse = vCol_bg * max(dot(vLightDir, Normal), 0); // 拡散色
    Out.Ambient = vCol_bg * 0.3f; // 環境色
	
	// テクスチャ座標
    uv = mul(Pos, mWLPB_bg);
    Out.ShadowMapUV = uv;
    uv = mul(Pos, mWLP_bg);
    Out.Depth = uv.zzzw;
		
    return Out;
}

// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_pass1(VS_OUTPUT In) : COLOR
{
    float4 Color = In.Ambient;
    
    float shadow_map = tex2Dproj(ShadowMapSamp, In.ShadowMapUV).x;
    
    Color += In.Diffuse * ((shadow_map < In.Depth.z / In.Depth.w - 0.01)
					 ? tex2Dproj(SrcSamp, In.ShadowMapUV) : 1);

    return Color;
}

// ----------------------------------------------------------------------------------------------------------------
// 影の整形 technique: ShadowTShader
// ----------------------------------------------------------------------------------------------------------------

// -------------------------------------------------------------
// 1パス目：エッジ抽出
// -------------------------------------------------------------

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT_QUAD VS_shadow1(
      float4 Pos    : POSITION,          // モデルの頂点
      float2 Tex    : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT_QUAD Out = (VS_OUTPUT_QUAD)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(-0.5f/MAP_SIZE, -0.5f/MAP_SIZE);
    Out.Tex1 = Tex + float2(+0.5f/MAP_SIZE, +0.5f/MAP_SIZE);
    Out.Tex2 = Tex + float2(-0.5f/MAP_SIZE, +0.5f/MAP_SIZE);
    Out.Tex3 = Tex + float2(+0.5f/MAP_SIZE, -0.5f/MAP_SIZE);

    return Out;
}
// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_shadow1(VS_OUTPUT_QUAD In) : COLOR
{   
    float4 Color;
	
	float d0 = tex2D( SrcSamp, In.Tex0 ) - tex2D( SrcSamp, In.Tex1 );
	float d1 = tex2D( SrcSamp, In.Tex2 ) - tex2D( SrcSamp, In.Tex3 );
	
	Color = d0*d0 + d1*d1;
	
    return Color;
}
// -------------------------------------------------------------
// 2パス目：ぼかす
// -------------------------------------------------------------

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT_QUAD VS_shadow2(
      float4 Pos    : POSITION,          // モデルの頂点
      float2 Tex    : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT_QUAD Out = (VS_OUTPUT_QUAD)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(-3.0f/MAP_SIZE, -3.0f/MAP_SIZE);
    Out.Tex1 = Tex + float2(-3.0f/MAP_SIZE, -1.0f/MAP_SIZE);
    Out.Tex2 = Tex + float2(-3.0f/MAP_SIZE,  1.0f/MAP_SIZE);
    Out.Tex3 = Tex + float2(-3.0f/MAP_SIZE,  3.0f/MAP_SIZE);
    Out.Tex4 = Tex + float2(-1.0f/MAP_SIZE, -3.0f/MAP_SIZE);
    Out.Tex5 = Tex + float2(-1.0f/MAP_SIZE, -1.0f/MAP_SIZE);
    Out.Tex6 = Tex + float2(-1.0f/MAP_SIZE,  1.0f/MAP_SIZE);
    Out.Tex7 = Tex + float2(-1.0f/MAP_SIZE,  3.0f/MAP_SIZE);

    return Out;
}

// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS_shadow2(VS_OUTPUT_QUAD In) : COLOR
{   
    float4 Color;
    float2 dvu = float2( 4.0f/MAP_SIZE, 0 );
	
	Color =   tex2D( SrcSamp, In.Tex0 )
			+ tex2D( SrcSamp, In.Tex1 )
			+ tex2D( SrcSamp, In.Tex2 )
			+ tex2D( SrcSamp, In.Tex3 )
			+ tex2D( SrcSamp, In.Tex4 )
			+ tex2D( SrcSamp, In.Tex5 )
			+ tex2D( SrcSamp, In.Tex6 )
			+ tex2D( SrcSamp, In.Tex7 )
			+ tex2D( SrcSamp, In.Tex0 + dvu)
			+ tex2D( SrcSamp, In.Tex1 + dvu)
			+ tex2D( SrcSamp, In.Tex2 + dvu)
			+ tex2D( SrcSamp, In.Tex3 + dvu)
			+ tex2D( SrcSamp, In.Tex4 + dvu)
			+ tex2D( SrcSamp, In.Tex5 + dvu)
			+ tex2D( SrcSamp, In.Tex6 + dvu)
			+ tex2D( SrcSamp, In.Tex7 + dvu)
			;
	
    return 0.7f * Color;
}

// -------------------------------------------------------------
// テクニック BOXと地形のシャドウマップの作成と描画
// -------------------------------------------------------------
technique BoxOnlyTShader
{
    // シャドウマップの作成
    pass Box_Shadow0
    {
        VertexShader = compile vs_1_1 VS_boxShadow();
        PixelShader = compile ps_2_0 PS_pass0();
    }

    // 描画
    pass Box_Draw1
    {
        VertexShader = compile vs_1_1 VS_boxDraw();
        PixelShader = compile ps_2_0 PS_pass1();
    }
}

technique BgOnlyTShader
{
    pass Bg_Shadow0
    {
        VertexShader = compile vs_1_1 VS_bgShadow();
        PixelShader = compile ps_2_0 PS_pass0();
    }

    pass Bg_Draw1
    {
        VertexShader = compile vs_1_1 VS_bgDraw();
        PixelShader = compile ps_2_0 PS_pass1();
    }
}

// 影の整形
technique ShadowTShader
{
    pass P0 // エッジの抽出
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_shadow1();
        PixelShader = compile ps_2_0 PS_shadow1();
        
		Sampler[0] = (SrcSamp);
    }
    pass P1 // ぼかし
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_shadow2();
        PixelShader = compile ps_2_0 PS_shadow2();
        
		Sampler[0] = (SrcSamp);
    }
}