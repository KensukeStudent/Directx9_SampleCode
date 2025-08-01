// -------------------------------------------------------------
// クロスフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------

float4x4 mWVP;		// 座標変換の行列

float4 vLightDir;	// ライトの方向
float4 vColor;		// ライト＊メッシュの色
float3 vEyePos;		// カメラの位置（ローカル座標系）


static const int    MAX_SAMPLES = 16;    // 最大サンプリング数
float2 g_avSampleOffsets[MAX_SAMPLES];	// サンプリングの位置
float4 g_avSampleWeights[MAX_SAMPLES];	// サンプリングの重み

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
// 模様のテクスチャ
texture DecaleTex;
sampler DecaleSamp = sampler_state
{
    Texture = <DecaleTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// 法線マップ
texture NormalMap;
sampler NormalSamp = sampler_state
{
    Texture = <NormalMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

//-----------------------------------------------------------------------------
// サンプラと SetTexture との対応
//-----------------------------------------------------------------------------
sampler s0 : register(s0);
sampler s1 : register(s1);
sampler s2 : register(s2);
sampler s3 : register(s3);
sampler s4 : register(s4);
sampler s5 : register(s5);
sampler s6 : register(s6);
sampler s7 : register(s7);

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;		// 頂点色
    float2 Tex			: TEXCOORD0;	// デカールテクスチャ座標
    float3 L			: TEXCOORD1;	// ライトベクトル
    float3 E			: TEXCOORD2;	// 視線ベクトル
};
// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos      : POSITION,          // ローカル位置座標
      float3 Normal   : NORMAL,            // 法線ベクトル
      float3 Tangent  : TANGENT0,          // 接ベクトル
      float2 Texcoord : TEXCOORD0          // 法線ベクトル
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(Pos, mWVP);
	
	// メッシュの色
	Out.Color = vColor;
	
	// デカール用のテクスチャ座標
	Out.Tex = Texcoord;

	// 座標系の変換基底
	float3 N = Normal;
	float3 T = Tangent;
	float3 B = cross(N,T);

	// 鏡面反射用のベクトル
	float3 E = vEyePos - Pos.xyz;	// 視線ベクトル
	Out.E.x = dot(E,T);
	Out.E.y = dot(E,B);
	Out.E.z = dot(E,N);

	float3 L = -vLightDir.xyz;		// ライトベクトル
	Out.L.x = dot(L,T);
	Out.L.y = dot(L,B);
	Out.L.z = dot(L,N);
	
	return Out;
}
// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
	float3 N = 2.0f*tex2D( NormalSamp, In.Tex ).xyz-1.0;// 法線マップからの法線
	float3 L = normalize(In.L);						// ライトベクトル
	float3 R = reflect(-normalize(In.E), N);		// 反射ベクトル
	float amb = -vLightDir.w;						// 環境光の強さ
	
    return In.Color * tex2D( DecaleSamp, In.Tex )	// 拡散光と環境光には、
			   * (max(0, dot(N, L))+amb)			// 頂点色とテクスチャの色を合成する
			 + 2.0f * pow(max(0,dot(R, L)), 64);		// Phong 鏡面反射光
}




//-----------------------------------------------------------------------------
// Name: DownScale4x4
// Desc: 1/4の縮小バッファへシーンをコピー
//-----------------------------------------------------------------------------
float4 DownScale4x4PS ( in float2 uv : TEXCOORD0 ) : COLOR
{
    float4 sample = 0.0f;

	for( int i=0; i < 16; i++ ) {
		sample += tex2D( s0, uv + g_avSampleOffsets[i] );
	}
    
	return sample / 16;
}



//-----------------------------------------------------------------------------
// Name: BrightPassFilter
// Desc: 輝度の高い部分だけを抽出する
//-----------------------------------------------------------------------------
float4 BrightPassFilterPS(in float2 uv : TEXCOORD0) : COLOR
{
	float4 vSample = tex2D( s0, uv );
	
	// 暗い部分をさっぴく
	vSample.rgb -= 1.5f;
	
	// 下限を０にする
	vSample = 3.0f*max(vSample, 0.0f);

	return vSample;
}




//-----------------------------------------------------------------------------
// Name: GaussBlur5x5
// Desc: 中心に近い13個のテクセルをサンプリングする
//      （係数にガウス分布が入っているので、結果的にガウス平均になる）
//-----------------------------------------------------------------------------
float4 GaussBlur5x5PS (in float2 uv : TEXCOORD0) : COLOR
{
    float4 sample = 0.0f;

	for( int i=0; i < 13; i++ ) {
		sample += g_avSampleWeights[i]
					 * tex2D( s0, uv + g_avSampleOffsets[i] );
	}

	return sample;
}




//-----------------------------------------------------------------------------
// Name: Star
// Desc: ８サンプリングして、光芒の線を徐々に作成する
//-----------------------------------------------------------------------------
float4 StarPS ( in float2 uv : TEXCOORD0 ) : COLOR
{
    float4 vColor = 0.0f;
    
    // 光芒にそった８つの点をサンプリングする
    for(int i = 0; i < 8; i++) {
        vColor += g_avSampleWeights[i] * tex2D(s0, uv + g_avSampleOffsets[i]);
    }
    	
    return vColor;
}




//-----------------------------------------------------------------------------
// Name: MergeTextures_6
// Desc: ６本の光芒を重ね合わせる
//-----------------------------------------------------------------------------
float4 MergeTextures_6PS(in float2 uv : TEXCOORD0 ) : COLOR
{
	float4 vColor = 0.0f;
	
	vColor = ( tex2D(s0, uv)
	         + tex2D(s1, uv)
	         + tex2D(s2, uv)
	         + tex2D(s3, uv)
	         + tex2D(s4, uv)
	         + tex2D(s5, uv) )/6.0f;
		
	return vColor;
}





// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}




//-----------------------------------------------------------------------------
// Name: DownScale4x4
// Type: Technique                                     
// Desc: 1/4の縮小バッファへシーンをコピー
//-----------------------------------------------------------------------------
technique DownScale4x4
{
    pass P0
    {
        PixelShader  = compile ps_2_0 DownScale4x4PS();
        MinFilter[0] = Point;
        AddressU[0] = Clamp;
        AddressV[0] = Clamp;
    }
}




//-----------------------------------------------------------------------------
// Name: BrightPassFilter
// Desc: 輝度の高い部分を抽出する
//-----------------------------------------------------------------------------
technique BrightPassFilter
{
    pass P0
    {
        PixelShader = compile ps_2_0 BrightPassFilterPS();
        MinFilter[0] = Point;
        MagFilter[0] = Point;
    }
}





//-----------------------------------------------------------------------------
// Name: GaussBlur5x5
// Desc: １３テクセルのサンプリングをして、ガウスぼかしをする
//-----------------------------------------------------------------------------
technique GaussBlur5x5
{
    pass P0
    {
        PixelShader = compile ps_2_0 GaussBlur5x5PS();
        MinFilter[0] = Point;
        AddressU[0] = Clamp;
        AddressV[0] = Clamp;
    }
}




//-----------------------------------------------------------------------------
// Name: Star
// Desc: ８サンプリングして、光芒の線を徐々に作成する
//-----------------------------------------------------------------------------
technique Star
{
    pass P0
    {
        PixelShader = compile ps_2_0 StarPS();
        MagFilter[0] = Linear;
        MinFilter[0] = Linear;
    }

}




//-----------------------------------------------------------------------------
// Name: MergeTextures_N
// Desc: ６本の光芒を重ね合わせる
//-----------------------------------------------------------------------------
technique MergeTextures
{
    pass P0
    {
        PixelShader = compile ps_2_0 MergeTextures_6PS();
        MagFilter[0] = Point;
        MinFilter[0] = Point;
        MagFilter[1] = Point;
        MinFilter[1] = Point;
        MagFilter[2] = Point;
        MinFilter[2] = Point;
        MagFilter[3] = Point;
        MinFilter[3] = Point;
        MagFilter[4] = Point;
        MinFilter[4] = Point;
        MagFilter[5] = Point;
        MinFilter[5] = Point;
    }
}
