// -------------------------------------------------------------
// ���e�e�N�X�`���V���h�E
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// �O���[�o���ϐ�
// -------------------------------------------------------------
float4x4 mWVP;		// ���[�J������ˉe��Ԃւ̍��W�ϊ�
float4x4 mWVPT;		// ���[�J������e�N�X�`����Ԃւ̍��W�ϊ�
float4	 vLightPos;	// ���C�g�̈ʒu

// -------------------------------------------------------------
// �e�N�X�`��
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
// ���_�V�F�[�_����s�N�Z���V�F�[�_�ɓn���f�[�^
// -------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: POSITION;
	float4 Color	: COLOR0;
	float2 TexDecale: TEXCOORD0;
	float4 TexShadow: TEXCOORD1;
};

// -------------------------------------------------------------
// ���_�V�F�[�_�v���O����
// -------------------------------------------------------------
VS_OUTPUT VS (
	  float4 Pos	: POSITION          // ���_�ʒu
	, float4 Normal	: NORMAL            // �@���x�N�g��
	, float2 Tex	: TEXCOORD0			// �e�N�X�`�����W
){
	VS_OUTPUT Out;        // �o�̓f�[�^
	
	// �ʒu���W
	Out.Pos = mul( Pos, mWVP );
	
	Out.Color = max( dot(normalize(vLightPos.xyz-Pos.xyz), Normal), 0);
	
	// �e�N�X�`�����W
	Out.TexDecale = Tex;
	
	// �e�N�X�`�����W
	Out.TexShadow = mul( Pos, mWVPT );
	
	return Out;
}
// -------------------------------------------------------------
// �s�N�Z���V�F�[�_�v���O����
// -------------------------------------------------------------
float4 PS ( VS_OUTPUT In) : COLOR
{
	float4 decale = tex2D( DecaleMapSamp, In.TexDecale );
	float4 shadow = tex2Dproj( ShadowMapSamp, In.TexShadow );
	
	return decale * (saturate(In.Color-0.5f*shadow)+0.3f);
}
// -------------------------------------------------------------
// �e�N�j�b�N
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}
