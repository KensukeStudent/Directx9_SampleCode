//-------------------------------------------------------------
// File: main.h
//
// Desc: メディアンフィルタ
//-------------------------------------------------------------
#pragma once



//-------------------------------------------------------------
// 定義や定数
//-------------------------------------------------------------
// 現在の入力データを保存する構造体
struct UserInput
{
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    LPDIRECT3DTEXTURE9		m_pTex;	// 元画像
    UINT					m_TexWidth;
    UINT					m_TexHeight;

    // シェーダ
    LPD3DXEFFECT		    m_pEffect;		// エフェクト
    D3DXHANDLE				m_hTechnique;	// テクニック
    D3DXHANDLE				m_htSrcMap;		// テクスチャ
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言

    BOOL					m_bLoadingApp;	// ロード中？
    CD3DFont* m_pFont;		// フォント
    UserInput				m_UserInput;	// 入力データ

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

