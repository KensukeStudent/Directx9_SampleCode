//-------------------------------------------------------------
// File: main.h
//
// Desc: ボリュームシャドウ
//-------------------------------------------------------------
#pragma once
#include "CShadowVolume.h"



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
// 全画面描画ポリゴン
//-------------------------------------------------------------
class CBigSquare {
private:
	typedef struct {
		D3DXVECTOR4 p;
		D3DCOLOR	color;
	} SHADOWVERTEX;
	static const DWORD FVF;

	LPDIRECT3DVERTEXBUFFER9 m_pVB;
public:
	CBigSquare();
	HRESULT		Create(LPDIRECT3DDEVICE9 pd3dDevice);
	void		RestoreDeviceObjects(FLOAT sx, FLOAT sy);
	void		Destroy();
	void		Render(LPDIRECT3DDEVICE9 pd3dDevice);
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	CBigSquare* m_pBigSquare;	// 影描画用全画面ポリゴン
	CD3DMesh* m_pMeshBG;		// 背景モデル
	CD3DMesh* m_pMeshBox;		// 箱モデル
	CShadowVolume* m_pShadowBox;	// 箱モデルの影

	// シェーダ
	LPD3DXEFFECT			m_pEffect;		// エフェクト
	D3DXHANDLE				m_hmWVP_smallBox;		// ワールド～射影行列
	D3DXHANDLE				m_hmWVP_largeBox;		// ワールド～射影行列
	D3DXHANDLE				m_hvPos;		// ライトの位置

	BOOL					m_bLoadingApp;	// ロード中？
	CD3DFont* m_pFont;		// フォント
	UserInput				m_UserInput;	// 入力データ

	D3DXMATRIX				m_mView;		// ビュー行列
	D3DXMATRIX				m_mProj;		// 射影行列
	D3DXVECTOR3				m_LighPos;		// ライトの位置

	FLOAT					m_fWorldRotX;	// Ｘ軸回転
	FLOAT					m_fWorldRotY;	// Ｙ軸回転

protected:
	virtual HRESULT OneTimeSceneInit();
	virtual HRESULT InitDeviceObjects();
	virtual HRESULT RestoreDeviceObjects();
	virtual HRESULT InvalidateDeviceObjects();
	virtual HRESULT DeleteDeviceObjects();
	virtual HRESULT Render();
	virtual HRESULT FrameMove();
	virtual HRESULT FinalCleanup();
	virtual HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT, D3DFORMAT);

	HRESULT RenderText();

	void    UpdateInput(UserInput* pUserInput);
public:
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CMyD3DApplication();
	virtual ~CMyD3DApplication();
};

