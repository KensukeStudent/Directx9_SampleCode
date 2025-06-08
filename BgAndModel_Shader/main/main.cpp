//-------------------------------------------------------------
// File: main.cpp
//
// Desc: HLSLのサンプル
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
    m_pEffect = NULL;
    m_hTechnique = NULL;
    m_hmWVP = NULL;
    m_pDecl = NULL;
    m_pMesh = new CD3DMesh();
    m_pMeshUfo = new CD3DMesh();

    m_fWorldRotX = -20 * 3.14 / 180; // ラジアン変換
    m_fWorldRotY = 0;

    m_viewX = 0;
	m_viewY = 0;

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

    // ピクセルシェーダプログラムのバージョンを調べる
    if (pCaps->PixelShaderVersion < D3DPS_VERSION(1, 1))
        return E_FAIL;

    // シェーダのチェック
    if (pCaps->VertexShaderVersion < D3DVS_VERSION(1, 1) &&
        !(dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING))
        return E_FAIL;	// 頂点シェーダ

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

    // メッシュの読み込み
    // ★★★追加：Xファイルの読み込み
    if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("map.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("Error"), MB_OK);
        return hr;
    }

    m_pMesh->UseMeshMaterials(FALSE);// テクスチャは自分で設定
    m_pMesh->SetFVF(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_TEX1);

    // メッシュの読み込み
// ★★★追加：Xファイルの読み込み
    if (FAILED(hr = m_pMeshUfo->Create(m_pd3dDevice, _T("ufo.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("Error"), MB_OK);
        return hr;
    }

    m_pMeshUfo->UseMeshMaterials(FALSE);// テクスチャは自分で設定
    m_pMeshUfo->SetFVF(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_TEX1);

    // 頂点宣言のオブジェクトの生成
    D3DVERTEXELEMENT9 decl[] =
    {
        {0, 0,D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,0},
        {0,12,D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,0},
        D3DDECL_END()
    };

    if (FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(
        decl, &m_pDecl)))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("CreateVertexDeclaration"), MB_OK);
        return hr;
    }

    // ★シェーダの読み込み
    LPD3DXBUFFER pErr = NULL;
    
    if (FAILED(hr = D3DXCreateEffectFromFile( m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, 0, NULL, &m_pEffect, &pErr))) {
        // シェーダの読み込みの失敗
        //MessageBox(NULL, (LPCTSTR)pErr->GetBufferPointer() , _T("Shader Load ERROR"), MB_OK);
        MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
    }
    else {
        m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
        m_hmWVP = m_pEffect->GetParameterByName(NULL, "mWVP");
    }
    SAFE_RELEASE(pErr);

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
    // 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define SAMP m_pd3dDevice->SetSamplerState

// レンダリング状態の設定
    RS(D3DRS_ZENABLE, TRUE);
    RS(D3DRS_LIGHTING, FALSE);

    // テクスチャの設定
    SAMP(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    SAMP(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);


    // ワールド行列
    D3DXMatrixIdentity(&m_mWorld);

    // ビュー行列
    D3DXVECTOR3 vFrom = D3DXVECTOR3(0.0f, 0.0f, -10.0f);
    D3DXVECTOR3 vLookat = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&m_mView, &vFrom, &vLookat, &vUp);

    // 射影行列の設定
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width)
        / ((FLOAT)m_d3dsdBackBuffer.Height);
    D3DXMatrixPerspectiveFovLH(&m_mProj, D3DX_PI / 4, fAspect
        , 1.0f, 100.0f);


    m_pMesh->RestoreDeviceObjects(m_pd3dDevice);
    m_pMeshUfo->RestoreDeviceObjects(m_pd3dDevice);
    if (m_pEffect != NULL) m_pEffect->OnResetDevice();// シェーダ

    m_pFont->RestoreDeviceObjects();	// フォント

    return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    UpdateInput(&m_UserInput); // 入力データの更新

    //---------------------------------------------------------
    // 入力に応じて座標系を更新する
    //---------------------------------------------------------
    // 回転
    if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
        m_fWorldRotY += m_fElapsedTime;
    else if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
        m_fWorldRotY -= m_fElapsedTime;

    if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
        m_fWorldRotX += m_fElapsedTime;
    else if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
        m_fWorldRotX -= m_fElapsedTime;

    //---------------------------------------------------------
    // ワールド行列の更新
    //---------------------------------------------------------
    D3DXMATRIX matRotX, matRotY;
    D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
    D3DXMatrixRotationY(&matRotY, m_fWorldRotY);
    D3DXMatrixMultiply(&m_mWorld, &matRotY, &matRotX);

    //---------------------------------------------------------
    // ビュー行列の更新
    //---------------------------------------------------------

#pragma region カメラ平行移動
    if (m_UserInput.bviewL) // 右
    {
        m_viewX += m_fElapsedTime * 2;
    }
    else if (m_UserInput.bviewJ) // 左
    {
        m_viewX -= m_fElapsedTime * 2;
    }

    if (m_UserInput.bviewI) // 上
    {
        m_viewY += m_fElapsedTime * 2;
    }
    else if (m_UserInput.bviewK) // 下
    {
        m_viewY -= m_fElapsedTime * 2;
    }

    D3DXVECTOR3 eye(m_viewX, m_viewY, -10.0);
    D3DXVECTOR3 target(m_viewX, m_viewY, 0.0f);

    // Y軸上方向を固定する（世界座標で常に上）
    D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&m_mView, &eye, &target, &up);
#pragma endregion

#pragma region カメラ回転
    //    // カメラの位置（固定）
    //D3DXVECTOR3 eye(0.0f, 5.0f, -10.0f);

    //// 初期視線方向（前方を見るベクトル）
    //D3DXVECTOR3 lookDir(0.0f, 0.0f, 1.0f); // Z+方向

    //// 時間または入力によって角度を変化
    //static float angle = 0.0f;
    //if (m_UserInput.bRotateRight) angle += m_fElapsedTime;
    //if (m_UserInput.bRotateLeft)  angle -= m_fElapsedTime;

    //// Y軸回転行列を作成（時計回りに回す）
    //D3DXMATRIX rotY;
    //D3DXMatrixRotationY(&rotY, angle);

    //// 視線方向ベクトルを回転
    //D3DXVec3TransformNormal(&lookDir, &lookDir, &rotY);

    //// 注視点を算出（視線方向に進んだ位置）
    //D3DXVECTOR3 target = eye + lookDir;

    //// 上方向はY軸
    //D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

    //// ビュー行列生成
    //D3DXMatrixLookAtLH(&m_mView, &eye, &target, &up);
#pragma endregion

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
    pUserInput->bA = (m_bActive && (GetAsyncKeyState('A') & 0x8000) == 0x8000);
    pUserInput->bS = (m_bActive && (GetAsyncKeyState('S') & 0x8000) == 0x8000);

    // ビュー行列
    pUserInput->bviewL = (m_bActive && (GetAsyncKeyState('L') & 0x8000) == 0x8000);
    pUserInput->bviewJ = (m_bActive && (GetAsyncKeyState('J') & 0x8000) == 0x8000);
    pUserInput->bviewI = (m_bActive && (GetAsyncKeyState('I') & 0x8000) == 0x8000);
    pUserInput->bviewK = (m_bActive && (GetAsyncKeyState('K') & 0x8000) == 0x8000);
}




//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    D3DXMATRIX m;
    D3DXVECTOR4 v;

    // 画面をクリアする
    m_pd3dDevice->Clear(0L, NULL,
        D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0x000040, 1.0f, 0L);

    if (SUCCEEDED(m_pd3dDevice->BeginScene()))	// 描画の開始
    {
        if (m_pEffect != NULL)
        {
            //-------------------------------------------------
            // ★シェーダの設定
            //-------------------------------------------------
            m_pEffect->SetTechnique(m_hTechnique);

            UINT numPasses = 0;
            m_pEffect->Begin(&numPasses, 0);

            for (UINT i = 0; i < numPasses; ++i)
            {
                m_pEffect->BeginPass(i);

                //-------------------------------------------------
                // ★シェーダ定数の設定
                //-------------------------------------------------
                m = m_mWorld * m_mView * m_mProj;
                m_pEffect->SetMatrix(m_hmWVP, &m);

                m_pEffect->SetTexture("Tex", m_pMesh->m_pTextures[0]);

                //-------------------------------------------------
                // 描画
                //-------------------------------------------------

                // モデルのRender関数の前で宣言する
                // モデルごとにhlslで必要な引数（POSITION, TEXCOORDが異なれば、都度切り替えて実装する）
                m_pd3dDevice->SetVertexDeclaration(m_pDecl);

                m_pMesh->Render(m_pd3dDevice); // 描画
                m_pMeshUfo->Render(m_pd3dDevice); // 描画

                m_pEffect->EndPass();
            }

            m_pEffect->End();
        }

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

    // ディスプレイの状態を表示する
    lstrcpy(szMsg, m_strDeviceStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    lstrcpy(szMsg, m_strFrameStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    // 操作法やパラメータを表示する
    fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
    lstrcpy(szMsg, TEXT("Use arrow keys to rotate object"));
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
    m_pMesh->InvalidateDeviceObjects();				// メッシュ
    m_pMeshUfo->InvalidateDeviceObjects();				// メッシュ
    if (m_pEffect != NULL) m_pEffect->OnLostDevice();	// シェーダ

    m_pFont->InvalidateDeviceObjects();	// フォント

    return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pMesh->Destroy();				// メッシュ
    m_pMeshUfo->Destroy();				// メッシュ
    SAFE_RELEASE(m_pEffect);		// シェーダ
    SAFE_RELEASE(m_pDecl);		// 頂点宣言

    m_pFont->DeleteDeviceObjects();	// フォント

    return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: 終了する直前に呼ばれる
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE(m_pMesh);	// メッシュ
    SAFE_DELETE(m_pMeshUfo);	// メッシュ

    SAFE_DELETE(m_pFont);	// フォント

    return S_OK;
}




