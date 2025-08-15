//-------------------------------------------------------------
// File: main.h
//
// Desc: モザイク
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
    BOOL bZoomIn;
    BOOL bZoomOut;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    // モザイクを掛けるテクスチャ
    LPDIRECT3DTEXTURE9		m_pTex;
    DWORD					m_Size;
    FLOAT					m_pos[2];
    FLOAT					m_vel[2];

    CD3DMesh* m_pMesh;
    CD3DMesh* m_pMeshBg;

    // シャドウマップ
    LPDIRECT3DSURFACE9		m_pMapZ;			// 深度バッファ
    LPDIRECT3DTEXTURE9		m_pOriginalMap;		// シーン描画
    LPDIRECT3DSURFACE9		m_pOriginalMapSurf;	// 
    LPDIRECT3DTEXTURE9		m_pSmallTex;		// 縮小バッファ
    LPDIRECT3DSURFACE9		m_pSmallSurf;		// 

    // 通常の座標変換行列
    D3DXMATRIX				m_mWorld;
    D3DXMATRIX				m_mView;
    D3DXMATRIX				m_mProj;

    BOOL					m_bLoadingApp;	// ロード中？
    CD3DFont* m_pFont;		// フォント
    UserInput				m_UserInput;	// 入力データ

    FLOAT                   m_fWorldRotX;   // Ｘ軸回転
    FLOAT                   m_fWorldRotY;   // Ｙ軸回転
    FLOAT                   m_fViewZoom;    // 視点の距離

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

