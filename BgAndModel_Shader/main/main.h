//-------------------------------------------------------------
// File: main.h
//
// Desc: HLSL�T���v���̃w�b�_�[
//-------------------------------------------------------------
#pragma once




//-------------------------------------------------------------
// ��`��萔
//-------------------------------------------------------------
// ���݂̓��̓f�[�^��ۑ�����\����
struct UserInput
{
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;

    // UFO�ړ�
    BOOL bW;
    BOOL bS;
    BOOL bD;
    BOOL bA;

    // �r���[�s��
    BOOL bviewL; // ��
    BOOL bviewJ; // �E
    BOOL bviewI; // ��
    BOOL bviewK; // ��
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    LPD3DXEFFECT					m_pEffect;	// ���V�F�[�_
    D3DXHANDLE						m_hTechnique;// ���e�N�j�b�N
    D3DXHANDLE						m_hmWVP;	// �����[���h�`�ˉe�s��

    D3DXHANDLE						m_hmWVP_Ufo;	// �����[���h�`�ˉe�s��
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// ���_�錾

    CD3DMesh* m_pMesh;	// ���f��
    CD3DMesh* m_pMeshUfo;	// ���f��

    D3DXMATRIX						m_mWorld;	// ���[���h�s��
    D3DXMATRIX						m_mView;	// �r���[�s��
    D3DXMATRIX						m_mProj;	// �ˉe�s��

    FLOAT						m_fWorldRotX;	// �w����]
    FLOAT						m_fWorldRotY;	// �x����]

    FLOAT						m_viewX;	// ���_X
    FLOAT						m_viewY;	// ���_Y

    FLOAT						m_ufoX;	// UFO X
    FLOAT						m_ufoZ;	// UFO Y

    BOOL						m_bLoadingApp;	// ���[�h���H
    CD3DFont* m_pFont;		// �t�H���g
    UserInput					m_UserInput;	// ���̓f�[�^

protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();
    virtual HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT);

    HRESULT RenderText();

    void    UpdateInput(UserInput* pUserInput);
public:
    LRESULT MsgProc(HWND hWnd, UINT msg
        , WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};
