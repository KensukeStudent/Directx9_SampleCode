//-------------------------------------------------------------
// File: main.cpp
//
// Desc: ボリュームシャドウ
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

#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState

//-------------------------------------------------------------
// 全画面描画ポリゴン
//-------------------------------------------------------------
const DWORD CBigSquare::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

//-------------------------------------------------------------
// コンストラクタ
//-------------------------------------------------------------
CBigSquare::CBigSquare()
{
	m_pVB = NULL;
}
//-------------------------------------------------------------
// メモリ確保
//-------------------------------------------------------------
HRESULT CBigSquare::Create(LPDIRECT3DDEVICE9 pd3dDevice)
{
	if (FAILED(pd3dDevice->CreateVertexBuffer(
		4 * sizeof(SHADOWVERTEX),	// 頂点バッファサイズ
		D3DUSAGE_WRITEONLY,		// 使用法
		FVF,					// 頂点フォーマット
		D3DPOOL_MANAGED,		// 有効なメモリクラス
		&m_pVB,					// 作成された頂点バッファ
		NULL)))				// NULL 固定
		return E_FAIL;

	return S_OK;
}
//-------------------------------------------------------------
// データ生成
//-------------------------------------------------------------
void CBigSquare::RestoreDeviceObjects(FLOAT sx, FLOAT sy)
{
	SHADOWVERTEX* v;
	m_pVB->Lock(0, 0, (void**)&v, 0);
	v[0].p = D3DXVECTOR4(0, sy, 0.0f, 1.0f);
	v[1].p = D3DXVECTOR4(0, 0, 0.0f, 1.0f);
	v[2].p = D3DXVECTOR4(sx, sy, 0.0f, 1.0f);
	v[3].p = D3DXVECTOR4(sx, 0, 0.0f, 1.0f);
	v[0].color = D3DCOLOR_RGBA(0, 0, 0, 0x7f);
	v[1].color = D3DCOLOR_RGBA(0, 0, 0, 0x7f);
	v[2].color = D3DCOLOR_RGBA(0, 0, 0, 0x7f);
	v[3].color = D3DCOLOR_RGBA(0, 0, 0, 0x7f);
	m_pVB->Unlock();
}
//-------------------------------------------------------------
// メモリの開放
//-------------------------------------------------------------
void CBigSquare::Destroy()
{
	SAFE_RELEASE(m_pVB);
}
//-------------------------------------------------------------
// 画面描画
//-------------------------------------------------------------
void CBigSquare::Render(LPDIRECT3DDEVICE9 pd3dDevice)
{
	pd3dDevice->SetFVF(FVF);
	pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(SHADOWVERTEX));
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}




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
	m_pBigSquare = new CBigSquare();
	m_pMeshBG = new CD3DMesh();
	m_pMeshBox = new CD3DMesh();
	m_pShadowBox = new CShadowVolume();

	m_pEffect = NULL;
	
	m_hmWVP_smallBox = NULL;
	m_hmWVP_largeBox = NULL;

	m_hvPos_samllBox = NULL;
	m_hvPos_largeBox = NULL;

	// 選択されるシーンの設定
	m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
	m_d3dEnumeration.AppMinDepthBits = 15;
	m_d3dEnumeration.AppMinStencilBits = 1;

	m_dwCreationWidth = 500;
	m_dwCreationHeight = 375;
	m_strWindowTitle = TEXT("main");
	m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
	m_bStartFullscreen = false;
	m_bShowCursorWhenFullscreen = false;

	// フォント
	m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
	m_bLoadingApp = TRUE;

	ZeroMemory(&m_UserInput, sizeof(m_UserInput));
	m_fWorldRotX = 0.0f;
	m_fWorldRotY = 0.0f;
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
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior,
	D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat)
{
	// シェーダのチェック
	if (pCaps->VertexShaderVersion < D3DVS_VERSION(1, 1))
		if ((dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING) == 0)
			return E_FAIL;

	// 両面ステンシル機能の確認
	if (!(pCaps->StencilCaps & D3DSTENCILCAPS_TWOSIDED)) return E_FAIL;

	// ステンシル機能をサポートしているかチェック
	if (FAILED(m_pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal
		, pCaps->DeviceType
		, adapterFormat
		, D3DUSAGE_RENDERTARGET
		// ピクセル シェーダ後のブレンディングのサポートが存在するか
		| D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING
		, D3DRTYPE_SURFACE
		, backBufferFormat)))
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
	DWORD i;

	// エフェクト
	LPD3DXBUFFER pErr;
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
		// シェーダの読み込みの失敗
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
		return hr;
	}
	else
	{
		m_hmWVP_smallBox = m_pEffect->GetParameterByName(NULL, "mWVP_SmallBox");
		m_hmWVP_largeBox = m_pEffect->GetParameterByName(NULL, "mWVP_LargeBox");
		m_hvPos_samllBox = m_pEffect->GetParameterByName(NULL, "vLightPos_smallBox");
		m_hvPos_largeBox = m_pEffect->GetParameterByName(NULL, "vLightPos_largeBox");
	}

	// 全画面描画ポリゴンの初期化
	if (FAILED(m_pBigSquare->Create(m_pd3dDevice))) return E_FAIL;

	// 背景メッシュの読み込み
	if (FAILED(hr = m_pMeshBG->Create(m_pd3dDevice, _T("CornellNoBox.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("CornellNoBox.x load Error"), MB_OK);
		return hr;
	}
	for (i = 0; i < m_pMeshBG->m_dwNumMaterials; i++) {
		m_pMeshBG->m_pMaterials[i].Ambient.r = m_pMeshBG->m_pMaterials[i].Diffuse.r *= 2;
		m_pMeshBG->m_pMaterials[i].Ambient.g = m_pMeshBG->m_pMaterials[i].Diffuse.g *= 2;
		m_pMeshBG->m_pMaterials[i].Ambient.b = m_pMeshBG->m_pMaterials[i].Diffuse.b *= 2;
	}

	// 箱メッシュの読み込み
	if (FAILED(hr = m_pMeshBox->Create(m_pd3dDevice, _T("box.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("box.x load Error"), MB_OK);
		return hr;
	}
	for (i = 0; i < m_pMeshBox->m_dwNumMaterials; i++) {
		m_pMeshBox->m_pMaterials[i].Ambient.r = m_pMeshBox->m_pMaterials[i].Diffuse.r *= 1.13f;
		m_pMeshBox->m_pMaterials[i].Ambient.g = m_pMeshBox->m_pMaterials[i].Diffuse.g *= 0.93f;
		m_pMeshBox->m_pMaterials[i].Ambient.b = m_pMeshBox->m_pMaterials[i].Diffuse.b *= 0.53f;
	}

	// 影ボリュームの生成
	m_pShadowBox->Create(m_pMeshBox->GetSysMemMesh());

	// フォントの初期化
	m_pFont->InitDeviceObjects(m_pd3dDevice);

	return S_OK;
}




//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//		確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	m_LighPos = D3DXVECTOR3(0.0f, 5.488f, 2.770f);

	// 質感の設定
	D3DMATERIAL9 mtrl;
	D3DUtil_InitMaterial(mtrl, 1.0f, 0.0f, 0.0f);
	m_pd3dDevice->SetMaterial(&mtrl);

	// テクスチャの設定
	TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	TSS(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	TSS(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	TSS(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// レンダリング状態の設定
	RS(D3DRS_DITHERENABLE, FALSE);
	RS(D3DRS_SPECULARENABLE, FALSE);
	RS(D3DRS_ZENABLE, TRUE);
	RS(D3DRS_AMBIENT, 0x000F0F0F);

	// ワールド行列
	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&matIdentity);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity);

	// ビュー行列
	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_mView);

	// 射影行列の設定
	FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
	D3DXMatrixPerspectiveFovLH(&m_mProj, 0.21f * D3DX_PI, fAspect, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mProj);

	// ライトの設定
	D3DLIGHT9 light;
	D3DUtil_InitLight(light, D3DLIGHT_DIRECTIONAL, -0.0f, -1.0f, 0.2f);
	light.Diffuse.r = 0.5f;
	light.Diffuse.g = 0.5f;
	light.Diffuse.b = 0.5f;
	light.Ambient.r = 0.5f;
	light.Ambient.g = 0.5f;
	light.Ambient.b = 0.5f;
	m_pd3dDevice->SetLight(0, &light);
	m_pd3dDevice->LightEnable(0, TRUE);
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

	// 四角形
	m_pBigSquare->RestoreDeviceObjects((FLOAT)m_d3dsdBackBuffer.Width,
		(FLOAT)m_d3dsdBackBuffer.Height);
	// メッシュ
	m_pMeshBG->RestoreDeviceObjects(m_pd3dDevice);
	m_pMeshBox->RestoreDeviceObjects(m_pd3dDevice);

	// エフェクト
	if (m_pEffect != NULL) m_pEffect->OnResetDevice();

	m_pFont->RestoreDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	UpdateInput(&m_UserInput);// 入力データの更新

	//---------------------------------------------------------
	// 入力に応じて座標系を更新する
	//---------------------------------------------------------
	// 回転
	D3DXMATRIX m;
	D3DXMATRIX matRotY;
	D3DXMATRIX matRotX;

	if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
		m_fWorldRotY += m_fElapsedTime;
	else if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
		m_fWorldRotY -= m_fElapsedTime;

	if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
		m_fWorldRotX += m_fElapsedTime;
	else if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
		m_fWorldRotX -= m_fElapsedTime;

	//---------------------------------------------------------
	// 行列の更新
	//---------------------------------------------------------
	// ワールドの回転
	D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
	D3DXMatrixRotationY(&matRotY, m_fWorldRotY);
	D3DXMatrixMultiply(&m, &matRotX, &matRotY);

	// ビュー行列
	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 2.73f, -8.0f);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 2.73f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);
	m_mView = m * m_mView;
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_mView);

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
}




//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	D3DXMATRIX m, mW, mS, mR, mT;
	D3DXVECTOR4 v;


	//画面のクリア
	m_pd3dDevice->Clear(0L, NULL
		, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL
		//   色     深度 ステンシル
		, 0xffffff, 1.0f, 0L);

	// 描画の開始
	if (SUCCEEDED(m_pd3dDevice->BeginScene())) {

		// ----------------------------------------------------
		// 下準備：影なし部分の描画
		// ----------------------------------------------------
		D3DXMatrixIdentity(&m);
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMeshBG->Render(m_pd3dDevice);

		// 小さい箱
		D3DXMatrixScaling(&mS, 1.82f, 1.65f, 1.82f);
		D3DXMatrixRotationY(&mR, 0.59f * D3DX_PI);
		D3DXMatrixTranslation(&mT, 2.73f - 1.85f, 0.f, 1.69f);
		m = mS * mR * mT;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMeshBox->Render(m_pd3dDevice);

		// 大きい箱
		D3DXMatrixScaling(&mS, 1.69f, 3.30f, 1.69f);
		D3DXMatrixRotationY(&mR, 0.91f * D3DX_PI);
		D3DXMatrixTranslation(&mT, 2.73f - 3.685f, 0, 3.51f);
		m = mS * mR * mT;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMeshBox->Render(m_pd3dDevice);

		// ----------------------------------------------------
		// パス2:影ボリュームの描画
		// ----------------------------------------------------
		// 深度バッファに書き込みはしない
		RS(D3DRS_ZWRITEENABLE, FALSE);
		// レンダリングターゲットに書き込みはしない
		RS(D3DRS_COLORWRITEENABLE, FALSE);
		// フラットシェーディングする
		RS(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		// 両面描く
		RS(D3DRS_CULLMODE, D3DCULL_NONE);

		// 両面ステンシルを使用する
		RS(D3DRS_STENCILENABLE, TRUE);
		RS(D3DRS_TWOSIDEDSTENCILMODE, TRUE);

		// ステンシルテストは常に合格する（＝テストしない）
		RS(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		RS(D3DRS_CCW_STENCILFUNC, D3DCMP_ALWAYS);
		// ステンシルバッファの増減を1に設定する
		RS(D3DRS_STENCILREF, 0x1);
		RS(D3DRS_STENCILMASK, 0xffffffff);
		RS(D3DRS_STENCILWRITEMASK, 0xffffffff);
		// 表面は深度テストに合格したらステンシルバッファの内容を+1する
		RS(D3DRS_STENCILPASS, D3DSTENCILOP_INCR);
		RS(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
		RS(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		// 裏面は深度テストに合格したらステンシルバッファの内容を-1する
		RS(D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR);
		RS(D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP);
		RS(D3DRS_CCW_STENCILFAIL, D3DSTENCILOP_KEEP);

		// レンダリングする
		if (m_pEffect != NULL) {
			D3DXHANDLE hTechnique = m_pEffect->GetTechniqueByName("TShader");
			m_pEffect->SetTechnique(hTechnique);
			m_pEffect->Begin(NULL, 0);

			// --------------------------------------------------------------------
			// 小さい箱
			m_pEffect->BeginPass(0);

			D3DXMatrixScaling(&mS, 1.82f, 1.65f, 1.82f);
			D3DXMatrixRotationY(&mR, 0.59f * D3DX_PI);
			D3DXMatrixTranslation(&mT, 2.73f - 1.85f, 0.f, 1.69f);
			mW = mS * mR * mT;
			m = mW * m_mView * m_mProj;
			m_pEffect->SetMatrix(m_hmWVP_smallBox, &m);
			D3DXMatrixInverse(&m, NULL, &mW); // 箱のワールド行列をローカル行列に変換する
			D3DXVec3Transform(&v, &m_LighPos, &m); // 箱のローカル座標系でのライトの位置を求める
			m_pEffect->SetVector(m_hvPos_samllBox, &v);
			m_pShadowBox->Render(m_pd3dDevice);

			m_pEffect->EndPass();
			
			// --------------------------------------------------------------------
			
			m_pEffect->BeginPass(1);

			// 大きい箱
			D3DXMatrixScaling(&mS, 1.69f, 3.30f, 1.69f);
			D3DXMatrixRotationY(&mR, 0.91f * D3DX_PI);
			D3DXMatrixTranslation(&mT, 2.73f - 3.685f, 0, 3.51f);
			mW = mS * mR * mT;
			m = mW * m_mView * m_mProj;
			m_pEffect->SetMatrix(m_hmWVP_largeBox, &m);
			D3DXMatrixInverse(&m, NULL, &mW);
			D3DXVec3Transform(&v, &m_LighPos, &m);
			m_pEffect->SetVector(m_hvPos_largeBox, &v);
			m_pShadowBox->Render(m_pd3dDevice);
			
			m_pEffect->EndPass();
			
			// --------------------------------------------------------------------

			m_pEffect->End();
		}

		// 状態を元に戻す
		RS(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		RS(D3DRS_CULLMODE, D3DCULL_CCW);
		RS(D3DRS_ZWRITEENABLE, TRUE);
		RS(D3DRS_COLORWRITEENABLE, 0xf);
		RS(D3DRS_STENCILENABLE, FALSE);
		RS(D3DRS_ALPHABLENDENABLE, FALSE);
		RS(D3DRS_TWOSIDEDSTENCILMODE, FALSE);

		// ----------------------------------------------------
		// パス3:影の描画
		// ----------------------------------------------------
		// 深度テストはしない
		RS(D3DRS_ZENABLE, FALSE);
		// ステンシルテストはする
		RS(D3DRS_STENCILENABLE, TRUE);
		// アルファブレンディングは線形に掛ける
		RS(D3DRS_ALPHABLENDENABLE, TRUE);
		RS(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		RS(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		// ポリゴンを描画するときには、テクスチャと頂点色の両方を見る
		TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		TSS(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		TSS(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		TSS(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);



		// ステンシルバッファの値が１以上のときに書き込む
		RS(D3DRS_STENCILREF, 0x1);
		RS(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
		RS(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

		m_pBigSquare->Render(m_pd3dDevice);

		// 状態を元に戻す
		RS(D3DRS_ZENABLE, TRUE);
		RS(D3DRS_STENCILENABLE, FALSE);
		RS(D3DRS_ALPHABLENDENABLE, FALSE);

		// 状態表示の文字の描画  
		RenderText();

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

	// ディスプレイの状態を表示する
	FLOAT fNextLine = 40.0f;

	lstrcpy(szMsg, m_strDeviceStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	lstrcpy(szMsg, m_strFrameStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	// 操作法やパラメータを表示する
	fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
	wsprintf(szMsg, TEXT("Arrow keys: Up=%d Down=%d Left=%d Right=%d"),
		m_UserInput.bRotateUp, m_UserInput.bRotateDown, m_UserInput.bRotateLeft, m_UserInput.bRotateRight);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Use arrow keys to rotate object"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Press 'F2' to configure display"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg, WPARAM wParam,
	LPARAM lParam)
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
	// エフェクト
	if (m_pEffect != NULL) m_pEffect->OnLostDevice();
	// メッシュ
	m_pMeshBG->InvalidateDeviceObjects();
	m_pMeshBox->InvalidateDeviceObjects();
	// フォント
	m_pFont->InvalidateDeviceObjects();

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	// エフェクト
	SAFE_RELEASE(m_pEffect);
	// メッシュ
	m_pMeshBG->Destroy();
	m_pMeshBox->Destroy();
	m_pShadowBox->Destroy();

	m_pBigSquare->Destroy();
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
	SAFE_DELETE(m_pShadowBox);
	SAFE_DELETE(m_pMeshBox);
	SAFE_DELETE(m_pMeshBG);
	SAFE_DELETE(m_pBigSquare);

	SAFE_DELETE(m_pFont);

	return S_OK;
}