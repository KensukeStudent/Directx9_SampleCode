//-------------------------------------------------------------
// File: main.h
//
// Desc: X�t�@�C��
//-------------------------------------------------------------
#pragma once



//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DMesh* m_pMesh;		// �������ǉ�


    BOOL					m_bLoadingApp;	// ���[�h���H
    CD3DFont* m_pFont;		// �t�H���g

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

public:
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

