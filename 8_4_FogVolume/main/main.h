//-------------------------------------------------------------
// File: main.h
//
// Desc: ボリュームフォグ
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
	BOOL bZ;
	BOOL bX;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	CD3DMesh* m_pMesh;	// モデル
	CD3DMesh* m_pMeshBg;	// 背景モデル
	LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言
	LPD3DXEFFECT					m_pEffect;	// シェーダ
	D3DXHANDLE						m_hmWVP;	// ワールド～射影行列
	D3DXHANDLE						m_hmWVPT;	// ワールド～テクスチャ行列
	D3DXHANDLE						m_hvCol;	// メッシュの色
	D3DXHANDLE						m_hvDir;	// ライトの方向
	D3DXHANDLE						m_hvDecaleTex; // デカールテクスチャー
	D3DXHANDLE						m_hvDepthTex; // 深度テクスチャー
	D3DXHANDLE						m_hvFrameBufferTex; // テクスチャー

	// レンダリングターゲット
	LPDIRECT3DSURFACE9				m_pMapZ;	// 共通Ｚバッファ
	LPDIRECT3DTEXTURE9				m_pColorMap;// 色
	LPDIRECT3DSURFACE9				m_pColorMapSurf;
	LPDIRECT3DTEXTURE9				m_pDepthMap;// 深度
	LPDIRECT3DSURFACE9				m_pDepthMapSurf;
	LPDIRECT3DTEXTURE9				m_pFogMap;	// フォグの厚み
	LPDIRECT3DSURFACE9				m_pFogMapSurf;

	DWORD							m_Width;	// ビューポートの幅
	DWORD							m_Height;	// 高さ
	UINT							m_MapW;		// テクスチャの幅
	UINT							m_MapH;		// 高さ

	D3DXMATRIX						m_mWorld;	// ワールド行列
	D3DXMATRIX						m_mView;	// ビュー行列
	D3DXMATRIX						m_mProj;	// 射影行列
	D3DXVECTOR4						m_LightDir;	// 光源の方向

	FLOAT					m_zoom;				// ズーム
	FLOAT                   m_fWorldRotX;       // Ｘ軸回転
	FLOAT                   m_fWorldRotY;       // Ｙ軸回転

	BOOL                    m_bLoadingApp;      // ロード中？
	CD3DFont* m_pFont;            // フォント
	UserInput               m_UserInput;        // 入力データ

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
