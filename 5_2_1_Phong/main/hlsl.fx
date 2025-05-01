// -------------------------------------------------------------
// ���ʔ��ˌ�
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// �O���[�o���ϐ�
// -------------------------------------------------------------

float4x4 mWVP;

float4 vLightDir;	// ���C�g�̕���
float4 vColor;		// ���C�g�����b�V���̐F
float3 vEyePos;		// �J�����̈ʒu�i���[�J�����W�n�j

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
      float4 Normal : NORMAL            // �@���x�N�g��
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // �o�̓f�[�^
	
	// ���W�ϊ�
	Out.Pos = mul(Pos, mWVP);
	
	// ���_�F
	float ambient = -vLightDir.w;	// �����̋���
	
	float3 eye = normalize(vEyePos - Pos.xyz);
	float3 L = -vLightDir; // ���[�J�����W�n�ł̃��C�g�x�N�g��
	float3 N = Normal.xyz;
	float3 R = -eye + 2.0f*dot(N,eye)*N;	// ���˃x�N�g�� ����: R = -E + 2.0 * dot(N�EE) * N
	
	// ���� + �g�U���� + ���ʔ���
    float4 ambientAndDiffuse = vColor * max(ambient, dot(Normal, -vLightDir));
    float4 phong = pow(max(0, dot(L, R)), 10);
    Out.Color = ambientAndDiffuse + phong;
	
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
