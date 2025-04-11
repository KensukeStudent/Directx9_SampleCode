//-------------------------------------------------------------
// File: main.h
//
// Desc: テクスチャを張る
//-------------------------------------------------------------
#pragma once



//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	LPDIRECT3DTEXTURE9		m_pEarthTexture;// 地球テクスチャー
	LPDIRECT3DTEXTURE9		m_pCloudTexture;// 雲テクスチャー
	LPDIRECT3DVERTEXBUFFER9 m_pVB;			// 頂点情報を格納する


	BOOL					m_bLoadingApp;	// ロード中？
    CD3DFont*				m_pFont;		// フォント

	int                     m_AlphaType;	// アルファブレンドの種類 Max6

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
	HWND GetWindowHandle() { return m_hWnd; }
};

