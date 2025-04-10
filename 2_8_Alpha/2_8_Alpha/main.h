//-------------------------------------------------------------
// File: main.h
//
// Desc: �e�N�X�`���𒣂�
//-------------------------------------------------------------
#pragma once



//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	LPDIRECT3DTEXTURE9		m_pEarthTexture;// �n���e�N�X�`���[
	LPDIRECT3DTEXTURE9		m_pCloudTexture;// �_�e�N�X�`���[
	LPDIRECT3DVERTEXBUFFER9 m_pVB;			// ���_�����i�[����


	BOOL					m_bLoadingApp;	// ���[�h���H
    CD3DFont*				m_pFont;		// �t�H���g

protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();
    virtual HRESULT ConfirmDevice( D3DCAPS9*, DWORD, D3DFORMAT );

    HRESULT RenderText();

public:
    LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
	LPDIRECT3DDEVICE9 GetD3DDevice() { return m_pd3dDevice; }
};

