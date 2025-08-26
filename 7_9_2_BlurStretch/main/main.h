//-------------------------------------------------------------
// File: main.h
//
// Desc: モーションブラー
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
    BOOL bDispersionUp;
    BOOL bDispersionDown;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DMesh* m_pMesh;
    CD3DMesh* m_pMeshBg;

    // シェーダ
    LPD3DXEFFECT		    m_pEffect;		// エフェクト
    D3DXHANDLE				m_hTechnique;	// テクニック
    D3DXHANDLE				m_hmWV;			// ローカル-ビュー変換行列
    D3DXHANDLE				m_hmWV2;			// ローカル-ビュー変換行列
    D3DXHANDLE				m_hmVP;			// ビュー-射影変換行列
    D3DXHANDLE				m_hmLastWV;		// １つ前の変換行列
    D3DXHANDLE				m_hvLightDir;	// ライトの方向
    D3DXHANDLE				m_hvEyePos;		// 視点
    D3DXHANDLE				m_hvCol;		// 頂点色

    // 通常の座標変換行列
    D3DXMATRIX				m_mWorld;
    D3DXMATRIX				m_mView;
    D3DXMATRIX				m_mProj;
    D3DXMATRIX				m_mLastWV;

    BOOL					m_bLoadingApp;	// ロード中？
    CD3DFont* m_pFont;		// フォント
    UserInput				m_UserInput;	// 入力データ

    FLOAT                   m_fWorldRotX;   // Ｘ軸回転
    FLOAT                   m_fWorldRotY;   // Ｙ軸回転
    FLOAT                   m_fViewZoom;    // 視点の距離
	FLOAT                   m_sleepTime;    // スリープ時間

    // ufo本体の座標
	D3DXVECTOR3				    m_fUfoPos;
	D3DXVECTOR3				    m_fUfoRot;
	D3DXVECTOR3				    m_fUfoPos2;

    // Lerp用ufo本体の座標
    D3DXVECTOR3				    m_fUfoPos_lerp;
    D3DXVECTOR3				    m_fUfoRot_lerp;
    D3DXVECTOR3				    m_fUfoPos2_lerp;

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
	void    DrawSubset(int pass);
public:
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

