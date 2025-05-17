//-------------------------------------------------------------
// File: main.cpp
//
// Desc: 丸影
//-------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <comdef.h>  // _com_error
#include <tchar.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "resource.h"
#include "main.h"

//-------------------------------------------------------------
// グローバル変数
//-------------------------------------------------------------
CMyD3DApplication* g_pApp = NULL;
HINSTANCE          g_hInst = NULL;


//-------------------------------------------------------------
// Name: WinMain()
// Desc: メイン関数
//-------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	CMyD3DApplication d3dApp;

	g_pApp = &d3dApp;
	g_hInst = hInst;

	InitCommonControls();
	if (FAILED(d3dApp.Create(hInst)))
		return 0;

	return d3dApp.Run();
}




//-------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: アプリケーションのコンストラクタ
//-------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
	m_pMeshBG = new CD3DMesh();
	m_pMesh = new CD3DMesh();
	m_pTex = NULL;

	m_pos = D3DXVECTOR3(-0.5f, 1, 0);
	m_shadowH = 0.25f;

	m_fWorldRotX = -0.2f * D3DX_PI;
	m_fWorldRotY = -0.25f * 2.0f * D3DX_PI;
	m_zoom = 5.0f;

	m_dwCreationWidth = 500;
	m_dwCreationHeight = 375;
	m_strWindowTitle = TEXT("main");
	m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
	m_bStartFullscreen = false;
	m_bShowCursorWhenFullscreen = false;

	m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
	m_bLoadingApp = TRUE;

	ZeroMemory(&m_UserInput, sizeof(m_UserInput));
}




//-------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: デストラクタ
//-------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: 一度だけ行う初期化
//		ウィンドウの初期化やIDirect3D9の初期化は終わってます。
//		ただ、LPDIRECT3DDEVICE9 の初期化は終わっていません。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
	// ローディングメッセージを表示する
	SendMessage(m_hWnd, WM_PAINT, 0, 0);

	m_bLoadingApp = FALSE;

	return S_OK;
}




//-------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: 初期化の時に呼ばれます。必要な能力をチェックします。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps
	, DWORD dwBehavior, D3DFORMAT Format)
{
	UNREFERENCED_PARAMETER(Format);
	UNREFERENCED_PARAMETER(dwBehavior);
	UNREFERENCED_PARAMETER(pCaps);

	return S_OK;
}


//-------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: デバイスが生成された後の初期化をします。
//		フレームバッファフォーマットやデバイスの種類が変わった
//		後に通過します。
//		ここで確保したメモリはDeleteDeviceObjects()で開放します
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
	HRESULT hr;

	// 背景メッシュの読み込み
	if (FAILED(hr = m_pMeshBG->Create(m_pd3dDevice, _T("map.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("map.x Error"), MB_OK);
		return hr;
	}

	// UFOモデル
	if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("ufo.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("ufo.x load Error"), MB_OK);
		return hr;
	}

	// 影テクスチャの読み込み
	D3DXCreateTextureFromFileEx(m_pd3dDevice, _T("shadow.bmp")
		, 0, 0, 0, 0, D3DFMT_A8R8G8B8
		, D3DPOOL_MANAGED
		, D3DX_FILTER_LINEAR
		, D3DX_FILTER_LINEAR
		, 0, NULL, NULL, &m_pTex);

	m_pFont->InitDeviceObjects(m_pd3dDevice);// フォント

	return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//		確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	m_pMeshBG->RestoreDeviceObjects(m_pd3dDevice);
	m_pMesh->RestoreDeviceObjects(m_pd3dDevice);

	// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define SAMP m_pd3dDevice->SetSamplerState

// レンダリング状態の設定
	RS(D3DRS_ZENABLE, TRUE);
	SAMP(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// 平行光源のライトの設定
	D3DLIGHT9 light;
	D3DUtil_InitLight(light
		, D3DLIGHT_DIRECTIONAL, -0.5f, -2.0f, 1.0f);
	m_pd3dDevice->SetLight(0, &light);
	m_pd3dDevice->LightEnable(0, TRUE);
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// ワールド行列
	D3DXMatrixIdentity(&m_mWorld);

	// 射影行列の設定
	FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width)
		/ ((FLOAT)m_d3dsdBackBuffer.Height);
	D3DXMatrixPerspectiveFovLH(&m_mProj, D3DX_PI / 4, fAspect
		, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mProj);


	m_pFont->RestoreDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	m_pos.x = 1.5f * (FLOAT)cos(1.0f * this->m_fTime) + 1.0f;
	m_pos.z = 1.5f * (FLOAT)sin(1.0f * this->m_fTime) + 0.0f;
	m_pos.y = -0.5f * (FLOAT)sin(0.2f * this->m_fTime) + 1.0f;

	UpdateInput(&m_UserInput); // 入力データの更新

	//---------------------------------------------------------
	// 入力に応じて座標系を更新する
	//---------------------------------------------------------
	// 回転
	if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
		m_fWorldRotY += m_fElapsedTime;
	else
		if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
			m_fWorldRotY -= m_fElapsedTime;

	if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
		m_fWorldRotX += m_fElapsedTime;
	else
		if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
			m_fWorldRotX -= m_fElapsedTime;

	// ズーム
	if (m_UserInput.bZ && !m_UserInput.bX)
		m_zoom += 0.01f;
	else if (m_UserInput.bX && !m_UserInput.bZ)
		m_zoom -= 0.01f;

	//---------------------------------------------------------
	// 行列の更新
	//---------------------------------------------------------
	// カメラの回転
	D3DXMATRIX m, matRotX, matRotY;
	D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
	D3DXMatrixRotationY(&matRotY, m_fWorldRotY);
	D3DXMatrixMultiply(&m, &matRotY, &matRotX);

	// ビュー行列
	D3DXVECTOR3 vEye = D3DXVECTOR3(0.0f, 0.0f, -m_zoom);
	D3DXVECTOR3 vLookat = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vEye, &vLookat, &vUp);
	m = m * m_mView;
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m);


	return S_OK;
}




//-------------------------------------------------------------
// Name: UpdateInput()
// Desc: 入力データを更新する
//-------------------------------------------------------------
void CMyD3DApplication::UpdateInput(UserInput* pUserInput)
{
	pUserInput->bRotateUp = (m_bActive && (GetAsyncKeyState(VK_UP) & 0x8000) == 0x8000);
	pUserInput->bRotateDown = (m_bActive && (GetAsyncKeyState(VK_DOWN) & 0x8000) == 0x8000);
	pUserInput->bRotateLeft = (m_bActive && (GetAsyncKeyState(VK_LEFT) & 0x8000) == 0x8000);
	pUserInput->bRotateRight = (m_bActive && (GetAsyncKeyState(VK_RIGHT) & 0x8000) == 0x8000);
	pUserInput->bZ = (m_bActive && (GetAsyncKeyState('Z') & 0x8000) == 0x8000);
	pUserInput->bX = (m_bActive && (GetAsyncKeyState('X') & 0x8000) == 0x8000);
}




//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	D3DXMATRIX m, mL;
	D3DXVECTOR4 v;

#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState

	if (SUCCEEDED(m_pd3dDevice->BeginScene()))	// 描画の開始
	{
		// 画面を塗りつぶす
		m_pd3dDevice->Clear(0L, NULL,
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
			0x4004080, 1.0f, 0L);

		// 背景の描画
		D3DXMatrixIdentity(&m);
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMeshBG->Render(m_pd3dDevice);

		// 飛行モデルの描画
		D3DXMatrixTranslation(&m, m_pos.x, m_pos.y, m_pos.z);
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMesh->Render(m_pd3dDevice);

		// 影の位置の行列の設定
		D3DXMatrixTranslation(&m, m_pos.x, m_shadowH, m_pos.z);
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);

		// アルファ合成の設定
		RS(D3DRS_ALPHABLENDENABLE, TRUE);
		RS(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		RS(D3DRS_SRCBLEND, D3DBLEND_ZERO);
		RS(D3DRS_LIGHTING, FALSE);
		// テクスチャと頂点色の両方を反映させる
		TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		typedef struct {
			FLOAT		p[3];		// 頂点座標
			D3DCOLOR	color;		// 頂点色
			FLOAT		tu, tv;		// テクスチャ座標
		} MyVERTEX;

		// 影の大きさ
		FLOAT size = 0.6f * (m_pos.y - m_shadowH) + 0.4f;
		// UFOが上にあれば影を大きくする
// 影の色（濃さ）
		FLOAT fc = 100.0f + 200.0f * (1.0f - size);
		// サイズに比例して色を落とす
		DWORD c = min(255, (DWORD)((0 < fc) ? fc : 0));
		// 値の範囲を0～255に制限します

		MyVERTEX Vertex[4] = {
			// x    y    z          色      (赤緑青α) tu tv
			{{-size, 0, -size}, D3DCOLOR_RGBA(c,c,c,0), 0, 0,},
			{{-size, 0,  size}, D3DCOLOR_RGBA(c,c,c,0), 0, 1,},
			{{ size, 0,  size}, D3DCOLOR_RGBA(c,c,c,0), 1, 1,},
			{{ size, 0, -size}, D3DCOLOR_RGBA(c,c,c,0), 1, 0,},
		};
		m_pd3dDevice->SetTexture(0, m_pTex);
		m_pd3dDevice->SetFVF(
			D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
		m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
			, 2, Vertex, sizeof(MyVERTEX));

		// 設定を元に戻す
		RS(D3DRS_ALPHABLENDENABLE, FALSE);
		RS(D3DRS_LIGHTING, TRUE);

		RenderText();				// ヘルプ等の表示

		m_pd3dDevice->EndScene();	// 描画の終了
	}

	return S_OK;
}




//-------------------------------------------------------------
// Name: RenderText()
// Desc: 状態やヘルプを画面に表示する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 0);
	TCHAR szMsg[MAX_PATH] = TEXT("");

	FLOAT fNextLine = 40.0f;

	// 操作法やパラメータを表示する
	fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
	wsprintf(szMsg, _T("Zoom: %f"), m_zoom);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Use arrow keys to rotate object"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Press 'z' or 'x' to change zoom"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	// ディスプレイの状態を表示する
	lstrcpy(szMsg, m_strDeviceStats);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, m_strFrameStats);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd,
	UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		if (m_bLoadingApp)
		{
			// ロード中
			HDC hDC = GetDC(hWnd);
			TCHAR strMsg[MAX_PATH];
			wsprintf(strMsg, TEXT("Loading... Please wait"));
			RECT rct;
			GetClientRect(hWnd, &rct);
			DrawText(hDC, strMsg, -1, &rct,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			ReleaseDC(hWnd, hDC);
		}
		break;
	}

	}

	return CD3DApplication::MsgProc(hWnd, msg, wParam, lParam);
}




//-------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: RestoreDeviceObjects() で作成したオブジェクトの開放
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
	m_pMeshBG->InvalidateDeviceObjects(); // メッシュ
	m_pMesh->InvalidateDeviceObjects();

	m_pFont->InvalidateDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	m_pMeshBG->Destroy();
	m_pMesh->Destroy();

	SAFE_RELEASE(m_pTex);

	m_pFont->DeleteDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: 終了する直前に呼ばれる
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
	SAFE_DELETE(m_pMesh); // メッシュ
	SAFE_DELETE(m_pMeshBG);

	SAFE_DELETE(m_pFont);	// フォント

	return S_OK;
}
