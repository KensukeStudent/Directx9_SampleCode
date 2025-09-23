// ------------------------------------------------------------
// 被写界深度
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float4x4 mWVP;
float4   vCol;
float4	 vLightDir;	// ライトの方向
float4   vCenter;
float4   vScale;

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
sampler FloorSamp = sampler_state
{
    Texture = <SrcTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};
// ------------------------------------------------------------
texture BlurTex;
sampler BlurSamp = sampler_state
{
    Texture = <BlurTex>;
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
// 頂点シェーダプログラム（ライティングなし）
// ------------------------------------------------------------
VS_OUTPUT VS_pass0 (
      float4 Pos    : POSITION           // モデルの頂点
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    float4 pos = mul( Pos, mWVP );
    
	Out.Pos = pos;			// 位置座標
	
	Out.Color = pos.z/pos.w;// 色成分に深度値
	
    Out.Tex0 = Tex0;		// テクスチャ座標

    return Out;
}
// ------------------------------------------------------------
// ピクセルシェーダプログラム（ライティングなし）
// ------------------------------------------------------------
float4 PS_pass0 (VS_OUTPUT In) : COLOR
{
	float4 Out = tex2D( FloorSamp, In.Tex0 );
	
	Out.a = In.Color.w;		// アルファ成分に深度値
	
	return Out;
}
// ------------------------------------------------------------
// 頂点シェーダプログラム（ライティングあり）
// ------------------------------------------------------------
VS_OUTPUT VS_pass1 (
      float4 Pos    : POSITION           // モデルの頂点
    , float4 Normal : NORMAL             // 法線ベクトル
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    float4 pos = mul( Pos, mWVP );
    
	Out.Pos = pos;						// 位置座標
	
	// 色
	Out.Color = vCol * ( vLightDir.w					 //環境色
			   + max(dot(vLightDir.xyz, Normal.xyz), 0));//拡散色
	
	Out.Color.w = pos.z/pos.w;			// 深度値
    Out.Tex0 = Tex0;					// テクスチャ座標

    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム（ライティングあり）
// ------------------------------------------------------------
float4 PS_pass1 (VS_OUTPUT In) : COLOR
{
	float4 Out = In.Color * tex2D( SrcSamp, In.Tex0 );
	
	return Out;
}
// ------------------------------------------------------------
// 頂点シェーダプログラム(ぼかし)
// ------------------------------------------------------------
VS_OUTPUT VS_pass2 (
      float4 Pos    : POSITION           // モデルの頂点
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
     ,float4 Tex1   : TEXCOORD1	         // テクスチャ座標
     ,float4 Tex2   : TEXCOORD2	         // テクスチャ座標
     ,float4 Tex3   : TEXCOORD3	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    // テクスチャ座標
    Out.Tex0 = Tex0;
    Out.Tex1 = Tex1;
    Out.Tex2 = Tex2;
    Out.Tex3 = Tex3;
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム(ぼかし)
// ------------------------------------------------------------
float4 PS_pass2 (VS_OUTPUT In) : COLOR
{
	float4 col0 = tex2D( SrcSamp, In.Tex0 );
	float4 col1 = tex2D( SrcSamp, In.Tex1 );
	float4 col2 = tex2D( SrcSamp, In.Tex2 );
	float4 col3 = tex2D( SrcSamp, In.Tex3 );
	
	return 0.25*(col0+col1+col2+col3);
}


// ------------------------------------------------------------
// ピクセルシェーダプログラム（合成）
// ------------------------------------------------------------
//PixelShader PS_pass3 = asm
//{
//    ps_1_1
    
//    tex t0						// くっきりした画像
//    tex t1						// ぼかした画像
    
//    sub_x4     r0, t0.a,  c0    // フォーカスの中心をc0にする
//    mul_x4     r0, r0.a,  r0.a	// ２乗して、符号を消す
//    mul_x4_sat r0, r0,    c1	// 強さを調整する
    
//    lrp        r0, r0.a,  t1, t0// 線形合成
//};

float4 PS_pass3(VS_OUTPUT In) : COLOR
{
    float4 sharp = tex2D(SrcSamp, In.Tex0); // くっきりした画像 (t0)
    float4 blur = tex2D(BlurSamp, In.Tex0); // ぼかした画像 (t1)

	// フォーカス係数を組み立てる（assembly の動作を再現）
    float f = 4 * (sharp.a - vCenter.w); // sub_x4 r0, t0.a, c0
    f = 4 * (f * f); // mul_x4 r0, r0.a, r0.a  (二乗して符号消し)
    f = saturate(4 * f * vScale.w); // mul_x4_sat r0, r0, c1

	// 線形合成（lrp r0, r0.a, t1, t0）
    return lerp(blur, sharp, f);
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0 // 通常レンダリング＋深度レンダリング
    {		// ライティングなし
        VertexShader = compile vs_1_1 VS_pass0();
        PixelShader = compile ps_2_0 PS_pass0();
    }
    pass P1	// 通常レンダリング＋深度レンダリング
    {		// ライティングあり
        VertexShader = compile vs_1_1 VS_pass1();
        PixelShader = compile ps_2_0 PS_pass1();
    }
    pass P2	// ぼかし
    {
        VertexShader = compile vs_1_1 VS_pass2();
        PixelShader = compile ps_2_0 PS_pass2();
        AddressU[0] = Clamp;
        AddressV[0] = Clamp;
        AddressU[1] = Clamp;
        AddressV[1] = Clamp;
        AddressU[2] = Clamp;
        AddressV[2] = Clamp;
        AddressU[3] = Clamp;
        AddressV[3] = Clamp;
    }
 //   pass P3	// 合成
 //   {
 //       PixelShader  = <PS_pass3>;
 //       // ピクセル定数レジスタ
 //       PixelShaderConstant4[C_CENTER] = (vCenter);
 //       PixelShaderConstant4[C_SCALE]  = (vScale);

	//	Sampler[0] = (SrcSamp);
	//	Sampler[1] = (BlurSamp);
	//}

    pass P3 // 合成
    {
        PixelShader = compile ps_2_0 PS_pass3();

        Sampler[0] = (SrcSamp);
        Sampler[1] = (BlurSamp);
    }
}
