//-------------------------------------------------------------
// File: main.cpp
//
// Desc: プライオリティバッファシャドウ
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

#define SHADOW_MAP_SIZE   2048

//-------------------------------------------------------------
// デバッグ用に表示するテクスチャ用の構造体
//-------------------------------------------------------------
typedef struct {
	FLOAT       p[4];
	FLOAT       tu, tv;
} TVERTEX;

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
	m_pMesh = new CD3DMesh();
	m_pMeshBg = new CD3DMesh();
	m_pos = D3DXVECTOR3(0, 0, 0);

	m_pShadowMap = NULL;
	m_pShadowMapSurf = NULL;
	m_pShadowMapZ = NULL;

	m_pEffect = NULL;
	m_hTechnique = NULL;
	m_hmWVP = NULL;
	m_hmWLP = NULL;
	m_hmWVPT = NULL;
	m_hvCol = NULL;
	m_hvId = NULL;
	m_hvDir = NULL;
	m_htIdMap = NULL;
	m_pDecl = NULL;

	m_fWorldRotX = -30*3.14f/180;
	m_fWorldRotY = 0;
	m_fViewZoom = 10;
	m_LighPos = D3DXVECTOR3(-6.0f, 6.0f, -2.0f);

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
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps,
	DWORD dwBehavior, D3DFORMAT Format)
{
	UNREFERENCED_PARAMETER(Format);
	UNREFERENCED_PARAMETER(dwBehavior);
	UNREFERENCED_PARAMETER(pCaps);


	// ピクセルシェーダバージョンチェック
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		return E_FAIL;

	// 頂点シェーダバージョンが上位かソフトウェア頂点処理
	if (pCaps->VertexShaderVersion < D3DVS_VERSION(1, 1)
		&& 0 == (dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING))
		return E_FAIL;

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

	// 車の読み込み
	// UFOモデル
	if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("ufo.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("ufo.x load Error"), MB_OK);
		return hr;
	}
	m_pMesh->SetFVF(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	// 地形モデル
	if (FAILED(hr = m_pMeshBg->Create(m_pd3dDevice, _T("map.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("map.x load Error"), MB_OK);
		return hr;
	}
	m_pMeshBg->UseMeshMaterials(FALSE);

	// レンダリング時にテクスチャやマテリアルの設定をしない
	m_pMesh->UseMeshMaterials(FALSE);
	m_pMeshBg->UseMeshMaterials(FALSE);

	// フォント
	m_pFont->InitDeviceObjects(m_pd3dDevice);

	// シェーダの読み込み
	LPD3DXBUFFER pErr;
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
		// シェーダの読み込みの失敗
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
		return hr;
	}
	else {
		m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
		m_hmWVP = m_pEffect->GetParameterByName(NULL, "mWVP");
		m_hmWLP = m_pEffect->GetParameterByName(NULL, "mWLP");
		m_hmWVPT = m_pEffect->GetParameterByName(NULL, "mWVPT");
		m_hvCol = m_pEffect->GetParameterByName(NULL, "vCol");
		m_hvId = m_pEffect->GetParameterByName(NULL, "vId");
		m_hvDir = m_pEffect->GetParameterByName(NULL, "vLightDir");
		m_htIdMap = m_pEffect->GetParameterByName(NULL, "IdMap");

		m_hmWVP_ufo = m_pEffect->GetParameterByName(NULL, "mWVP_ufo");
		m_hmWLP_ufo = m_pEffect->GetParameterByName(NULL, "mWLP_ufo");
		m_hmWVPT_ufo = m_pEffect->GetParameterByName(NULL, "mWVPT_ufo");
		m_hvCol_ufo = m_pEffect->GetParameterByName(NULL, "vCol_ufo");
		m_hvId_ufo = m_pEffect->GetParameterByName(NULL, "vId_ufo");
		m_hvDir_ufo = m_pEffect->GetParameterByName(NULL, "vLightDir_ufo");
		m_htIdMap_ufo = m_pEffect->GetParameterByName(NULL, "IdMap_ufo");
	}

	// 頂点宣言のオブジェクトの生成(地形用)
	D3DVERTEXELEMENT9 decl[] =
	{
		{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	0},
		{0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	if (FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(decl, &m_pDecl)))
	{
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "CreateVertexDeclaration", MB_OK);
		return hr;
	}

	D3DCAPS9 d3dCaps;
	if (FAILED(m_pd3dDevice->GetDeviceCaps(&d3dCaps)))
	{
		MessageBoxA(NULL, "GetDeviceCaps failed", "Error", MB_OK);
		return E_FAIL;
	}
	DWORD maxRt = d3dCaps.NumSimultaneousRTs;
	if (maxRt <= 1) {
		MessageBoxA(NULL, "マルチレンダリングターゲットがサポートされていません。終了します。", "Error", MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//		確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	// プライオリティバッファの生成
	if (FAILED(m_pd3dDevice->CreateTexture(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pShadowMap, NULL)))
		return E_FAIL;
	if (FAILED(m_pShadowMap->GetSurfaceLevel(0, &m_pShadowMapSurf)))
		return E_FAIL;
	if (FAILED(m_pd3dDevice->CreateDepthStencilSurface(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pShadowMapZ, NULL)))
		return E_FAIL;

	// メッシュ
	m_pMesh->RestoreDeviceObjects(m_pd3dDevice);
	m_pMeshBg->RestoreDeviceObjects(m_pd3dDevice);

	// 質感の設定
	D3DMATERIAL9 mtrl;
	D3DUtil_InitMaterial(mtrl, 1.0f, 0.0f, 0.0f);
	m_pd3dDevice->SetMaterial(&mtrl);

	// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState

// レンダリング状態の設定
	RS(D3DRS_DITHERENABLE, FALSE);
	RS(D3DRS_SPECULARENABLE, FALSE);
	RS(D3DRS_ZENABLE, TRUE);
	RS(D3DRS_AMBIENT, 0x000F0F0F);

	TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	TSS(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	TSS(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	TSS(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// ワールド行列
	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&m_mWorld);

	// ビュー行列
	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -10.0f);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);

	// 射影行列
	FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
	D3DXMatrixPerspectiveFovLH(&m_mProj, D3DX_PI / 4, fAspect, 1.0f, 100.0f);

	// フォント
	m_pFont->RestoreDeviceObjects();

	m_pEffect->OnResetDevice();

	return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	UpdateInput(&m_UserInput); // 入力データの更新

	// UFOを動かす
	if (m_UserInput.arrowW)
	{
		// z軸方向に移動
		m_pos.z += 0.1f * 0.016f;
	}

	if (m_UserInput.arrowS)
	{
		// z軸方向に移動
		m_pos.z -= 0.1f * 0.016f;
	}

	if (m_UserInput.arrowD)
	{
		// x軸方向に移動
		m_pos.x += 0.1f * 0.016f;
	}

	if (m_UserInput.arrowA)
	{
		// x軸方向に移動
		m_pos.x -= 0.1f * 0.016f;
	}

	//m_pos.x = 1.5f * (FLOAT)cos(1.0f * this->m_fTime) + 1.0f;
	//m_pos.z = 1.5f * (FLOAT)sin(1.0f * this->m_fTime);
	m_pos.y = -2;

	//---------------------------------------------------------
	// 入力に応じて座標系を更新する
	//---------------------------------------------------------
	
	// 回転
	D3DXMATRIX matRotY;
	D3DXMATRIX matRotX;
	D3DXMATRIX mCamera;

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

	D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
	D3DXMatrixRotationY(&matRotY, m_fWorldRotY);

	D3DXMatrixMultiply(&mCamera, &matRotY, &matRotX);

	//---------------------------------------------------------
	// ビュー行列の設定
	//---------------------------------------------------------
	// ズーム
	if (m_UserInput.bZoomIn && !m_UserInput.bZoomOut)
		m_fViewZoom += m_fElapsedTime;
	else if (m_UserInput.bZoomOut && !m_UserInput.bZoomIn)
		m_fViewZoom -= m_fElapsedTime;

	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -m_fViewZoom);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);
	m_mView = mCamera * m_mView;

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

	pUserInput->bZoomIn = (m_bActive && (GetAsyncKeyState('Z') & 0x8000) == 0x8000);
	pUserInput->bZoomOut = (m_bActive && (GetAsyncKeyState('X') & 0x8000) == 0x8000);

	pUserInput->arrowW = (m_bActive && (GetAsyncKeyState('W') & 0x8000) == 0x8000);
	pUserInput->arrowS = (m_bActive && (GetAsyncKeyState('S') & 0x8000) == 0x8000);
	pUserInput->arrowD = (m_bActive && (GetAsyncKeyState('D') & 0x8000) == 0x8000);
	pUserInput->arrowA = (m_bActive && (GetAsyncKeyState('A') & 0x8000) == 0x8000);
}

VOID CMyD3DApplication::DrawModel(int pass)
{
	float fOffsetX = 0.5f + (0.5f / (float)SHADOW_MAP_SIZE);
	float fOffsetY = 0.5f + (0.5f / (float)SHADOW_MAP_SIZE);
	D3DXMATRIX mScaleBias(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		fOffsetX, fOffsetY, 0.0f, 1.0f
	);

	DrawUfo(pass, mScaleBias);
	DrawGround(pass, mScaleBias);
}

void CMyD3DApplication::DrawUfo(int pass, const D3DXMATRIX& mScaleBias)
{
	D3DXMATRIX mVP, mL, m;
	D3DXVECTOR4 v(0, 0, 0, 1);
	D3DMATERIAL9* pMtrl;
	DWORD i;

	m_pEffect->SetVector(m_hvId_ufo, &v);

	mVP = m_mView * m_mProj;
	D3DXMatrixTranslation(&mL, m_pos.x, m_pos.y, m_pos.z);

	if (pass == 0) // シャドウマップ用
	{
		m = mL * m_mLightVP;
		m_pEffect->SetMatrix(m_hmWLP_ufo, &m);
		m_pMesh->Render(m_pd3dDevice);
	}
	else if (pass == 3)
	{
		m = mL * mVP;
		m_pEffect->SetMatrix(m_hmWVP_ufo, &m);

		m = mL * m_mLightVP;
		m_pEffect->SetMatrix(m_hmWLP_ufo, &m);

		m = m * mScaleBias;
		m_pEffect->SetMatrix(m_hmWVPT_ufo, &m);

		// ローカル空間ライト方向の算出
		D3DXMatrixInverse(&m, NULL, &mL);
		D3DXVec3Transform(&v, &m_LighPos, &m);
		D3DXVec4Normalize(&v, &v); v.w = 0;
		m_pEffect->SetVector(m_hvDir_ufo, &v);

		// マテリアルごとに描画
		pMtrl = m_pMesh->m_pMaterials;
		for (i = 0; i < m_pMesh->m_dwNumMaterials; i++)
		{
			v.x = pMtrl->Diffuse.r;
			v.y = pMtrl->Diffuse.g;
			v.z = pMtrl->Diffuse.b;
			v.w = pMtrl->Diffuse.a;
			m_pEffect->SetVector(m_hvCol_ufo, &v);

			m_pMesh->m_pLocalMesh->DrawSubset(i);
			pMtrl++;
		}
	}
}

/// <summary>
/// 地面
/// </summary>
/// <param name="pass"></param>
/// <param name="mVP">回転・ビュー・射影行列</param>
/// <param name="mScaleBias"></param>
void CMyD3DApplication::DrawGround(int pass, const D3DXMATRIX& mScaleBias)
{
	D3DXMATRIX mVP, mL, m;
	D3DXVECTOR4 v(0.5f, 0.5f, 0.5f, 1.0f);
	D3DMATERIAL9* pMtrl;
	DWORD i;

	m_pEffect->SetVector(m_hvId, &v);
	mVP = m_mView * m_mProj;
	D3DXMatrixIdentity(&mL);

	if (pass == 1)
	{
		m = mL * m_mLightVP;
		m_pEffect->SetMatrix(m_hmWLP, &m);
		m_pMeshBg->Render(m_pd3dDevice);
	}
	else if (pass == 2)
	{
		//m = mL * mVP;
		m_pEffect->SetMatrix(m_hmWVP, &mVP);

		//m = mL * m_mLightVP;
		m_pEffect->SetMatrix(m_hmWLP, &m_mLightVP);

		m = m_mLightVP * mScaleBias;
		m_pEffect->SetMatrix(m_hmWVPT, &m);

		// ライトベクトル（ローカル空間）
		D3DXMatrixInverse(&m, NULL, &mL);
		D3DXVec3Transform(&v, &m_LighPos, &m);
		D3DXVec4Normalize(&v, &v); 
		v.w = 0;
		m_pEffect->SetVector(m_hvDir, &v);

		pMtrl = m_pMeshBg->m_pMaterials;
		for (i = 0; i < m_pMeshBg->m_dwNumMaterials; i++)
		{
			v.x = pMtrl->Diffuse.r;
			v.y = pMtrl->Diffuse.g;
			v.z = pMtrl->Diffuse.b;
			v.w = pMtrl->Diffuse.a;
			m_pEffect->SetVector(m_hvCol, &v);

			m_pEffect->SetTexture("DecaleMap", m_pMeshBg->m_pTextures[i]);
			m_pMeshBg->m_pLocalMesh->DrawSubset(i);
			pMtrl++;
		}
	}
}

//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	D3DXMATRIX mLP, mView, mProj;
	LPDIRECT3DSURFACE9 pOldBackBuffer, pOldZBuffer;
	D3DVIEWPORT9 oldViewport;

	//---------------------------------------------------------
	// 行列の作成
	//---------------------------------------------------------
	// ライト方向から見た射影空間への行列
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&mView, &m_LighPos, &vLookatPt, &vUp);

	D3DXMatrixPerspectiveFovLH(&mProj
		, 0.5f * D3DX_PI		// 視野角
		, 1.0f				// アスペクト比
		, 1.0f, 12.0f);	// near far
	m_mLightVP = mView * mProj;

	//---------------------------------------------------------
	// 描画
	//---------------------------------------------------------
	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		if (m_pEffect != NULL)
		{
			//-------------------------------------------------
			// シェーダの設定
			//-------------------------------------------------
			m_pEffect->SetTechnique(m_hTechnique);
			m_pEffect->Begin(NULL, 0);

			//-------------------------------------------------
			// レンダリングターゲットの保存
			//-------------------------------------------------
			m_pd3dDevice->GetRenderTarget(0, &pOldBackBuffer);
			m_pd3dDevice->GetDepthStencilSurface(&pOldZBuffer);
			m_pd3dDevice->GetViewport(&oldViewport);

			//-------------------------------------------------
			// レンダリングターゲットの変更
			//-------------------------------------------------
			m_pd3dDevice->SetRenderTarget(0, m_pShadowMapSurf);
			m_pd3dDevice->SetDepthStencilSurface(m_pShadowMapZ);
			// ビューポートの変更
			D3DVIEWPORT9 viewport = { 0,0      // 左上の座標
							, SHADOW_MAP_SIZE // 幅
							, SHADOW_MAP_SIZE // 高さ
							, 0.0f,1.0f };     // 前面、後面
			m_pd3dDevice->SetViewport(&viewport);

			UINT numPasses = 0;
			m_pEffect->Begin(&numPasses, 0);

			for (UINT i = 0; i < numPasses; ++i)
			{
				m_pEffect->BeginPass(i);

				// テクスチャの設定
				m_pEffect->SetTexture(m_htIdMap, m_pShadowMap);
				m_pd3dDevice->SetVertexDeclaration(m_pDecl);

				DrawModel(i);

				if (i == 1)
				{
					//-------------------------------------------------
					// レンダリングターゲットを元に戻す
					//-------------------------------------------------
					m_pd3dDevice->SetRenderTarget(0, pOldBackBuffer);
					m_pd3dDevice->SetDepthStencilSurface(pOldZBuffer);
					m_pd3dDevice->SetViewport(&oldViewport);
					pOldBackBuffer->Release();
					pOldZBuffer->Release();

					// シャドウマップのクリア
					m_pd3dDevice->Clear(0L, NULL
						, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
						, 0xFFFFFFFF, 1.0f, 0L);
				}

				m_pEffect->EndPass();
			}

			m_pEffect->End();
		}

		// ヘルプの表示
		RenderText();

//#if 1 // デバッグ用にテクスチャを表示する
//		{
//			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
//			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//			m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
//			float scale = 128.0f;
//			TVERTEX Vertex[4] = {
//				// x  y  z rhw tu tv
//				{    0,    0,0, 1, 0, 0,},
//				{scale,    0,0, 1, 1, 0,},
//				{scale,scale,0, 1, 1, 1,},
//				{    0,scale,0, 1, 0, 1,},
//			};
//			m_pd3dDevice->SetTexture(0, m_pShadowMap);
//			m_pd3dDevice->SetVertexShader(NULL);
//			m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
//			m_pd3dDevice->SetPixelShader(0);
//			m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
//		}
//#endif		

		// 描画の終了
		m_pd3dDevice->EndScene();
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

	FLOAT fNextLine = 40.0f; // 表示する高さ

	// 操作法やパラメータを表示する
	fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
	lstrcpy(szMsg, TEXT("Use arrow keys to rotate camera"));
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Press 'F2' to configure display"));
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	lstrcpy(szMsg, m_strDeviceStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, m_strFrameStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	lstrcpy(szMsg, TEXT("aaaaaaaaaa"));
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
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
			DrawText(hDC, strMsg, -1, &rct
				, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
	// シャドウマップ
	SAFE_RELEASE(m_pShadowMapSurf);
	SAFE_RELEASE(m_pShadowMap);
	SAFE_RELEASE(m_pShadowMapZ);

	m_pMesh->InvalidateDeviceObjects(); // メッシュ
	m_pMeshBg->InvalidateDeviceObjects();

	m_pFont->InvalidateDeviceObjects();	// フォント

	// シェーダ
	if (m_pEffect != NULL) m_pEffect->OnLostDevice();

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	// シェーダ
	SAFE_RELEASE(m_pEffect);
	SAFE_RELEASE(m_pDecl);		// 頂点宣言

	// メッシュ
	m_pMesh->Destroy();
	m_pMeshBg->Destroy();

	// フォント
	m_pFont->DeleteDeviceObjects();

	return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: 終了する直前に呼ばれる
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
	SAFE_DELETE(m_pMeshBg); // メッシュ
	SAFE_DELETE(m_pMesh);

	SAFE_DELETE(m_pFont);	// フォント

	return S_OK;
}




