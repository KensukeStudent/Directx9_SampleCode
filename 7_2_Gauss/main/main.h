//-------------------------------------------------------------
// File: main.h
//
// Desc: Gaussian フィルタ
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
	enum {
		WEIGHT_MUN = 8,						// 重みを計算する個数
	};
	FLOAT					m_dispersion_sq;// 分散の平方根
	FLOAT					m_tbl[WEIGHT_MUN];// 重みの配列
	VOID		UpdateWeight(FLOAT param);// 重みの配列の計算

	CD3DMesh* m_pMesh;
	CD3DMesh* m_pMeshBg;

	// シェーダ
	LPD3DXEFFECT		    m_pEffect;		// エフェクト
	D3DXHANDLE				m_hTechnique;	// テクニック
	D3DXHANDLE				m_hafWeight;	// 重みの配列
	D3DXHANDLE				m_htSrcMap;		// テクスチャ
	D3DXHANDLE				m_xGaussMap;		// テクスチャ

	// シャドウマップ
	LPDIRECT3DSURFACE9		m_pMapZ;			// 深度バッファ
	LPDIRECT3DTEXTURE9		m_pOriginalMap;		// テクスチャ
	LPDIRECT3DSURFACE9		m_pOriginalMapSurf;	// サーフェス
	LPDIRECT3DTEXTURE9		m_pXMap;		// テクスチャ
	LPDIRECT3DSURFACE9		m_pXMapSurf;	// サーフェス
	LPDIRECT3DTEXTURE9		m_pXYMap;		// テクスチャ
	LPDIRECT3DSURFACE9		m_pXYMapSurf;	// サーフェス

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

