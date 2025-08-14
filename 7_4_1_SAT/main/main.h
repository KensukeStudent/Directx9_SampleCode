//-------------------------------------------------------------
// File: main.h
//
// Desc: エリア総和テーブル
//-------------------------------------------------------------
#pragma once



//-------------------------------------------------------------
// 定義・定数
//-------------------------------------------------------------
// 現在の入力データを保持する構造体
struct UserInput
{
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
    BOOL bZoomIn;
    BOOL bZoomOut;
    BOOL bSizeUp;
    BOOL bSizeDown;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    // レンダリングターゲット
    LPDIRECT3DSURFACE9       m_pMapZ;        // 深度バッファ
    LPDIRECT3DTEXTURE9       m_pSatTex;      // SATテクスチャ (A)
    LPDIRECT3DSURFACE9       m_pSatSurf;     // SATサーフェス (A)
    LPDIRECT3DTEXTURE9       m_pSatWorkTex;  // Ping-Pong用 (B)
    LPDIRECT3DSURFACE9       m_pSatWorkSurf; // Ping-Pong用 (B)
    FLOAT                    m_size;         // ぼかし量

    // シェーダ
    LPD3DXEFFECT             m_pEffect;      // エフェクト
    D3DXHANDLE               m_hTechnique;   // テクニック
    D3DXHANDLE               m_htSrcMap;     // テクスチャハンドル

    // メッシュ
    CD3DMesh* m_pMesh;
    CD3DMesh* m_pMeshBg;

    // 各種の座標変換行列
    D3DXMATRIX				m_mWorld;
    D3DXMATRIX				m_mView;
    D3DXMATRIX				m_mProj;

    BOOL					m_bLoadingApp;	// ロード中フラグ
    CD3DFont* m_pFont;		// フォント
    UserInput				m_UserInput;	// 入力データ

    FLOAT                   m_fWorldRotX;   // 上下回転
    FLOAT                   m_fWorldRotY;   // 左右回転
    FLOAT                   m_fViewZoom;    // ズーム量

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
    void    UpdateParam();
public:
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

