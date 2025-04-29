//-------------------------------------------------------------
// File: main.h
//
// Desc: ���ʔ��ˌ�
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
    BOOL bZoomIn;
    BOOL bZoomOut;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DMesh* m_pMesh;
    CD3DMesh* m_pMeshBg;

    // �V�F�[�_
    LPD3DXEFFECT		    m_pEffect;		// �G�t�F�N�g
    D3DXHANDLE				m_hTechnique;	// �e�N�j�b�N
    D3DXHANDLE				m_hmWVP;		// ���[�J��-�ˉe�ϊ��s��
    D3DXHANDLE				m_hmW;			// ���[�J��-���[���h�ϊ��s��
    D3DXHANDLE				m_hvLightDir;	// ���C�g�̕���
    D3DXHANDLE				m_hvColor;		// ���_�F
    D3DXHANDLE				m_hvEyePos;		// ���_�̈ʒu

    // �ʏ�̍��W�ϊ��s��
    D3DXVECTOR4				m_vFromPt;
    D3DXMATRIX				m_mView;
    D3DXMATRIX				m_mProj;

    BOOL					m_bLoadingApp;	// ���[�h���H
    CD3DFont* m_pFont;		// �t�H���g
    UserInput				m_UserInput;	// ���̓f�[�^

    FLOAT                   m_fWorldRotX;   // �w����]
    FLOAT                   m_fWorldRotY;   // �x����]
    FLOAT                   m_fViewZoom;    // ���_�̋���

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
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

