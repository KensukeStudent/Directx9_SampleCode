// -------------------------------------------------------------
// �g�U��
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// �O���[�o���ϐ�
// -------------------------------------------------------------

float4x4 mWVP;
float4x4 mWIT;

float3 vLightDir;							// ���C�g�̕���

// ���̋���
float4 I_a = { 0.3f, 0.3f, 0.3f, 0.0f };    // ambient
float4 I_d = { 0.7f, 0.7f, 0.7f, 0.0f };    // diffuse

// ���˗�
float4 k_a = { 1.0f, 1.0f, 1.0f, 1.0f };    // ambient
float4 k_d = { 1.0f, 1.0f, 1.0f, 1.0f };    // diffuse

// -------------------------------------------------------------
// ���_�V�F�[�_����s�N�Z���V�F�[�_�ɓn���f�[�^
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;
};

// -------------------------------------------------------------
// �V�[���̕`��
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos    : POSITION,          // ���[�J���ʒu���W
      float3 Normal : NORMAL            // �@���x�N�g��
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // �o�̓f�[�^
	
	// ���W�ϊ�
	Out.Pos = mul(Pos, mWVP);
	
	// ���_�F
	float3 L = -vLightDir;
	float3 N = normalize(mul(Normal, (float3x3)mWIT)); // ���[���h���W�n�ł̖@��

	Out.Color = I_a * k_a					   // ����
	          + I_d * k_d * max(0, dot(N, L)); // �g�U��
	
	return Out;
}

// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
    return In.Color;
}

// -------------------------------------------------------------
// �e�N�j�b�N
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // �V�F�[�_
        VertexShader = compile vs_2_0 VS(); // vs_1_1 -> ps_2_0
        PixelShader = compile ps_2_0 PS(); // ps_1_1 -> ps_2_0
    }
}
