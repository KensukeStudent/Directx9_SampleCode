// ------------------------------------------------------------
// 輪郭抽出
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// 
// 画像フィルタの考え方
// http://rs.aoyaman.com/img_pro/b6.html
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float4x4 mWVP0;
float4x4 mWVP1;

float4   vCol;
float4	 vLightDir;	// ライトの方向

// ------------------------------------------------------------
// テクスチャ
// ------------------------------------------------------------
texture SrcTex;
sampler SrcSamp = sampler_state
{
    Texture = <SrcTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

// ------------------------------------------------------------
texture FloorTex;
sampler FloorSamp = sampler_state
{
    Texture = <FloorTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

// ------------------------------------------------------------
texture OriginalTex;
sampler OriginalSamp = sampler_state
{
    Texture = <OriginalTex>;
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
    float4 Color		: COLOR0;
	float2 Tex0			: TEXCOORD0;
	float2 Tex1			: TEXCOORD1;
	float2 Tex2			: TEXCOORD2;
	float2 Tex3			: TEXCOORD3;
};

// ------------------------------------------------------------
// 照明計算なし頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass0 (
      float4 Pos    : POSITION           // モデルの頂点
     ,float4 Normal : NORMAL             // 法線ベクトル
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP0);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算なしピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass0 (VS_OUTPUT In) : COLOR
{
	float4 Out;
	
	// 色
    Out = tex2D(FloorSamp, In.Tex0);
	
	return Out;
}


// ------------------------------------------------------------
// 照明計算あり頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass1 (
      float4 Pos    : POSITION           // モデルの頂点
    , float4 Normal : NORMAL             // 法線ベクトル
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP1);
	
	// 色
	float diffuse = max(dot(vLightDir.xyz, Normal.xyz), 0);//拡散色
	float ambient = vLightDir.w;						   //環境色
	Out.Color = vCol * ( diffuse + ambient );
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算ありピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass1 (VS_OUTPUT In) : COLOR
{
	float4 Out;
	
	// 色
    Out = In.Color * tex2D(SrcSamp, In.Tex0);
	
	return Out;
}


// ------------------------------------------------------------
// 照明計算なし頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass2(
      float4 Pos : POSITION // モデルの頂点
     , float4 Normal : NORMAL // 法線ベクトル
     , float4 Tex0 : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP1);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}


// ------------------------------------------------------------
// 法線エッジピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_NormalEdge(VS_OUTPUT In) : COLOR0
{
    // 参考書のアセンブラをHLSLに移植したもの
    // ロバーツフィルタ斜め方向の差から求める計算公式
    float4 tl = tex2D(OriginalSamp, In.Tex0);
    float4 br = tex2D(OriginalSamp, In.Tex1);
    float4 bl = tex2D(OriginalSamp, In.Tex2);
    float4 tr = tex2D(OriginalSamp, In.Tex3);

    float3 d1 = tl.rgb - br.rgb;
    float3 d2 = bl.rgb - tr.rgb;
    
    int d = 8; // 差を強調する係数
    float diff = d * sqrt(d1 * d1 + d2 * d2); // diffの値を入れると輪郭が白く抽出できる
    //float diff = d * 2.0f * (dot(d1, d1) + dot(d2, d2)); // 2乗和だとエッジが軽くなるので係数で調整する

    float edge = saturate(4 * (1.0 - diff));
    return float4(1, 1, 1, edge);
}

// ------------------------------------------------------------
// 輝度エッジピクセルシェーダプログラム(Y)
// ------------------------------------------------------------
float4 PS_LumEdge(VS_OUTPUT In) : COLOR0
{
	// 輝度の重み(色の強さRGB2Yへの変換)
    float3 lumW = float3(0.299f, 0.587f, 0.114f);

	// サンプル（左上, 右下, 左下, 右上）
    float4 tl = tex2D(OriginalSamp, In.Tex0);
    float4 br = tex2D(OriginalSamp, In.Tex1);
    float4 bl = tex2D(OriginalSamp, In.Tex2);
    float4 tr = tex2D(OriginalSamp, In.Tex3);

    float Lt = dot(tl.rgb, lumW);
    float Lb = dot(br.rgb, lumW);
    float Ll = dot(bl.rgb, lumW);
    float Lr = dot(tr.rgb, lumW);

	// Roberts クロス演算（斜め方向の差分）
    float d1 = Lr - Ll; // 右上 - 左下
    float d2 = Lt - Lb; // 左上 - 右下

	// 元のアセンブラの定数に合わせたスケーリング
    float d = 8;
    //float diff = d * sqrt(d1 * d1 + d2 * d2);
    float diff = d * 10 * (dot(d1, d1) + dot(d2, d2)); // 2乗和だとエッジが軽くなるので係数で調整する
    
    float e = 4;
    float edge = saturate(e * (1.0f - diff));
    //float edge = saturate(1.0f - e * diff);

    return float4(1, 1, 1, edge);
};

// ------------------------------------------------------------
// 色相エッジピクセルシェーダプログラム(CbCr)
// ------------------------------------------------------------
float4 PS_CbCrEdge(VS_OUTPUT In) : COLOR
{    
    float3 d0 = tex2D(OriginalSamp, In.Tex0).rgb - tex2D(OriginalSamp, In.Tex1).rgb;
    float3 d1 = tex2D(OriginalSamp, In.Tex2).rgb - tex2D(OriginalSamp, In.Tex3).rgb;
	
	// レンダリングターゲット２：CbCr
    float3x3 RGB2CrCb =
    {
        { 0, 0, 0 }, // Y(=0)
        { 0.50000, -0.41869, -0.08131 }, // Cr
        { -0.16874, -0.33126, +0.50000 }, // Cb
    };
    d0 = mul(RGB2CrCb, d0);
    d1 = mul(RGB2CrCb, d1);
	
    float diff = dot(d0, d0) + dot(d1, d1);
    // 本来の勾配ノルム計算式は以下だけど、sqrtは重いので2乗和で代用する
    // float diff = sqrt(dot(d0, d0) + dot(d1, d1));
	
    float edge = saturate(1.0 - 100.0 * diff);
    return float4(1, 1, 1, edge);
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0 // 照明計算なし
    {
        VertexShader = compile vs_1_1 VS_pass0();
        PixelShader  = compile ps_2_0 PS_pass0();
    }

    pass P1 // 照明計算あり
    {
        VertexShader = compile vs_1_1 VS_pass1();
        PixelShader = compile ps_2_0 PS_pass1();
    }

    pass P2 // 法線エッジ
    {
        PixelShader = compile ps_2_0 PS_NormalEdge();
    }

    pass P3 // 輝度エッジ抽出
    {
        PixelShader = compile ps_2_0 PS_LumEdge();
    }

    pass P3 // 色相エッジ抽出
    {
        PixelShader = compile ps_2_0 PS_CbCrEdge();
    }
}
