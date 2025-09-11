// ------------------------------------------------------------
// 輪郭抽出
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
//
// 法線はどう取得しているのか、Normalのところで0.5とか係数は何か
// 各成分-1~1の範囲を0~1に変換している
// 検証して1.5fにしたときに背景にエッジがかからなかった、そのため1を超えると想定外の描画になるぽい
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float4x4 mWVP0;
float4x4 mWVP1;

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
    float4 Normal : COLOR1;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float2 Tex2 : TEXCOORD2;
    float2 Tex3 : TEXCOORD3;
};
// ------------------------------------------------------------
// ピクセルシェーダから出力するデータ
// ------------------------------------------------------------
struct PS_OUTPUT
{
    float4 Color : COLOR0;
    float4 Normal : COLOR1;
};

// 背景

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
    Out.Pos = mul(Pos, mWVP0);
		
	// 法線 0~1に線形変換
    Out.Normal = 0.5f * Normal + 0.5f; // ホントはワールドに変換
    
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算なしピクセルシェーダプログラム
// ------------------------------------------------------------
PS_OUTPUT PS_pass0(VS_OUTPUT In) : COLOR
{
    PS_OUTPUT Out;
	
	// レンダリングターゲット１：色
    Out.Color = tex2D(FloorSamp, In.Tex0);
	
	// レンダリングターゲット２：法線
    Out.Normal = In.Normal;
	
    return Out;
}

// ボックスモデル

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
    Out.Pos = mul(Pos, mWVP1);
	
	// 色
    float diffuse = max(dot(vLightDir.xyz, Normal.xyz), 0); //拡散色
    float ambient = vLightDir.w; //環境色
    Out.Color = vCol * (diffuse + ambient);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;
    
    // 法線 0~1に線形変換
    Out.Normal = 0.5f * Normal + 0.5f; // ホントはワールドに変換

    return Out;
}
// ------------------------------------------------------------
// 照明計算ありピクセルシェーダプログラム
// ------------------------------------------------------------
PS_OUTPUT PS_pass1(VS_OUTPUT In) : COLOR
{
    PS_OUTPUT Out;
	
	// レンダリングターゲット１：色
    Out.Color = In.Color * tex2D(SrcSamp, In.Tex0);
	
	// レンダリングターゲット２：法線
    Out.Normal = In.Normal;
	
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
// エッジ抽出ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_NormalEdge(VS_OUTPUT In) : COLOR
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
    
    float4 d0 = tex2D(OriginalSamp, In.Tex0) - tex2D(OriginalSamp, In.Tex1);
    float4 d1 = tex2D(OriginalSamp, In.Tex2) - tex2D(OriginalSamp, In.Tex3);
    
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
        PixelShader = compile ps_2_0 PS_pass0();
    }
    pass P1 // 照明計算あり
    {
        VertexShader = compile vs_1_1 VS_pass1();
        PixelShader = compile ps_2_0 PS_pass1();
    }
    pass P2 // 法線エッジ
    {
        // シェーダ
        PixelShader = compile ps_2_0 PS_NormalEdge();
    }
}
