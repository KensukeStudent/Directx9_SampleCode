// ------------------------------------------------------------
// 輪郭抽出
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// 各エッジの挙動の違い：https://www.karvan1230.com/entry/2024/08/06/213207
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float4x4 mWVP;
float m_hfId;

float fNear = 1.0f;
float fFar = 7.0f;

float4 vCol;
float4 vLightDir; // ライトの方向

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
    float4 Pos : POSITION;
    float4 Color : COLOR0;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float2 Tex2 : TEXCOORD2;
    float2 Tex3 : TEXCOORD3;
};

// ------------------------------------------------------------
// 照明計算なし頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass0(
      float4 Pos : POSITION // モデルの頂点
     , float4 Normal : NORMAL // 法線ベクトル
     , float4 Tex0 : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算なしピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass0(VS_OUTPUT In) : COLOR
{
    float4 Out;
	
	// 色
    Out = tex2D(FloorSamp, In.Tex0);
    Out.a = m_hfId;
	
    return Out;
}


// ------------------------------------------------------------
// 照明計算あり頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass1(
      float4 Pos : POSITION // モデルの頂点
    , float4 Normal : NORMAL // 法線ベクトル
     , float4 Tex0 : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP);
	
	// 色
    float diffuse = max(dot(vLightDir.xyz, Normal.xyz), 0); //拡散色
    float ambient = vLightDir.w; //環境色
    Out.Color = vCol * (diffuse + ambient);
    Out.Color.w = (Out.Pos.w - fNear) / (fFar - fNear); // 深度
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算ありピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass1(VS_OUTPUT In) : COLOR
{
    float4 Out;
	
	// 色
    Out = In.Color * tex2D(SrcSamp, In.Tex0);
    Out.a = m_hfId * In.Color.w;
    //Out.a = m_hfId + In.Color.w;
    
    // 加算型
    // 1: 1 + 0.3 = 1.3, 1 + 0.8 = 1.8: 結果両方が1を超える影響で手前のオブジェクトにエッジがのらない？
    
    // 乗算型
    // 1: 1 * 0.3 = 0.3, 1 * 0.8 = 0.8: 結果両方が1を超えないので手前のオブジェクトにエッジがのる
	
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
    Out.Pos = mul(Pos, mWVP);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}

// ------------------------------------------------------------
// エッジ抽出ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_IdEdge(VS_OUTPUT In) : COLOR
{
    //tex t0	// カレントフレーム（左上）
	//tex t1	// カレントフレーム（右下）
	//tex t2	// カレントフレーム（左下）
	//tex t3	// カレントフレーム（右上）
	
	//// 引数にt#レジスタを２つ使えないので一度コピー
	//mov r0, t0
	//mov r1, t2
	
	//add_x4     r0,   r0,  -t1	;       r0                  r1
	//add_x4     r1,   r1,  -t3	;     (t0-t1)            4*(t2-t3)
    
    float d0 = tex2D(OriginalSamp, In.Tex0).a - tex2D(OriginalSamp, In.Tex1).a;
    float d1 = tex2D(OriginalSamp, In.Tex2).a - tex2D(OriginalSamp, In.Tex3).a;
    
	//mul_x4     r0,   r0,   r0	;
	//mul_x4     r1,   r1,   r1	; 16*(t0-t1の深度)^2,  16*(t2-t3の深度)^2)
	//add_x4     r0,   r0,   r1	; r0.a = 64((t0-t0の深度)^2+(t3-t1の深度)^2)
    
    float diff = dot(d0, d0) + dot(d1, d1);
    
    //mov        r0,   1-r0		; r0.a = (1-64((t0-t0の深度)^2+(t3-t1の深度)^2))
    float edge = saturate(1 - 100 * diff);
    
    return float4(1, 1, 1, edge);
};

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
        PixelShader  = compile ps_2_0 PS_pass1();
    }
    pass P2 // 法線エッジ
    {
        // シェーダ
        PixelShader = compile ps_2_0 PS_IdEdge();
	}
}
