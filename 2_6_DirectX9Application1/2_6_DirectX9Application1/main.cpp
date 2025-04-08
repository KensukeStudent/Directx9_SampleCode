//-------------------------------------------------------------
// File: main.cpp
//
// Desc: DirectX AppWizardで作成したアプリケーションの雛形 2025年でも起動できるように
//-------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
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

//-----------------------------------------------------------------------------
// グローバル変数
//-----------------------------------------------------------------------------
CMyD3DApplication* g_pApp = NULL;
HINSTANCE          g_hInst = NULL;


// 関数宣言
void ReportHResultError(LPCTSTR functionName, HRESULT hr);


//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: メイン関数。全てを初期化した後、メッセージのループに入る。
//       暇なときは画面のレンダリングが行われる。
//-----------------------------------------------------------------------------
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




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: アプリケーションのコンストラクタ。~CMyD3DApplication()と対になっている。
//       メンバ変数は、ここで初期化するべき。アプリケーションのウィンドウは、まだ
//       生成されていなくて、DirectX のデバイスもまだ無い。ウィンドウやDirect3D
//       に依存する部分の初期化はまだ後の場所でやらなくてはいけません。
//
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_dwCreationWidth = 500;
    m_dwCreationHeight = 375;
    m_strWindowTitle = TEXT("DirectX9Application1");
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
    m_bStartFullscreen = false;
    m_bShowCursorWhenFullscreen = false;

    // d3dfont.cpp を使ったフォント
    m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
    m_bLoadingApp = TRUE;
    m_pD3DXMesh = NULL;

    ZeroMemory(&m_UserInput, sizeof(m_UserInput));
    m_fWorldRotX = 0.0f;
    m_fWorldRotY = 0.0f;
}




//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: デストラクタ.  CMyD3DApplication()と対になっている。
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: FinalCleanup()と対になっています。
//       ウィンドウは生成されて、IDirect3D9も生成されていますが、
//       デバイスはまだ生成されていません。
//       ここでは、デバイスに依存しないようなアプリケーションの
//       初期化や開放を行います。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // TODO: 一回だけの初期化をします。

    // ローディングメッセージを表示する
    SendMessage(m_hWnd, WM_PAINT, 0, 0);

    m_bLoadingApp = FALSE;

    return S_OK;
}









//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: デバイスの初期化中に呼ばれます。このプログラムは、アプリケーションが
//       正常に動くためのグラフィックスチップの最低限の機能を調べます。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps, DWORD dwBehavior,
    D3DFORMAT Format)
{
    UNREFERENCED_PARAMETER(Format);
    UNREFERENCED_PARAMETER(dwBehavior);
    UNREFERENCED_PARAMETER(pCaps);

    BOOL bCapsAcceptable;

    // TODO: 呼ばれたデバイスの能力(CAPS)が動作するものか調べる
    bCapsAcceptable = TRUE;

    if (bCapsAcceptable)
        return S_OK;
    else
        return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: DeleteDeviceObjects()と対になっている。
//       デバイスは作成されている。
//       D3DPOOL_MANAGED や D3DPOOL_SCRATCH、D3DPOOL_SYSTEMMEMのフラグによって
//       指定されるような、Reset()によって消失しないオブジェクトは、
//       ここで作成することができる。CreateImageSurface を使って生成した
//       サーフェイスは決して消失しないので、ここで作成することができる。
//       頂点シェーダやピクセルシェーダも消失しないので、生成することができる。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // TODO: デバイスに依存するオブジェクトを生成する。

    HRESULT hr;

    // フォントの初期化
    m_pFont->InitDeviceObjects(m_pd3dDevice);

    // D3DX を使って、ティーポットのポリゴンモデルを生成する。
    if (FAILED(hr = D3DXCreateTeapot(m_pd3dDevice, &m_pD3DXMesh, NULL)))
    {
        ReportHResultError(TEXT("D3DXCreateTeapot"), hr);
        return hr;
    }

    return S_OK;
}

void ReportHResultError(LPCTSTR functionName, HRESULT hr)
{
    TCHAR message[512];

    // HRESULT を人間が読めるエラー文字列に変換
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        0,  // 言語はデフォルト
        message,
        sizeof(message) / sizeof(TCHAR),
        NULL);

    TCHAR output[1024];
    swprintf_s(output, 1024, TEXT("%s failed with HRESULT 0x%08X: %s\n"), functionName, hr, message);

    OutputDebugString(output);
}


//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: InvalidateDeviceObjects()と対になっている。
//       デバイスは存在しているが、消失状態にある。D3DPOOL_DEFAULT のリソースか、
//       レンダリングの間中存在するレンダリング環境、行列、テクスチャ等の
//       オブジェクトはここで作成する。これらがレンダリングする間に
//       変わらないのなら、ここで設定することによって、Render()やFrameMove()
//       において、毎回設定するといった余分な処理は必要なくなる。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // TODO: レンダリングの環境を設定する

    // 質感を設定する
    D3DMATERIAL9 mtrl;
    D3DUtil_InitMaterial(mtrl, 1.0f, 0.0f, 0.0f);
    m_pd3dDevice->SetMaterial(&mtrl);

    // テクスチャを設定する
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // レンダリングの環境の雑多な設定を行う
    m_pd3dDevice->SetRenderState(D3DRS_DITHERENABLE, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0x000F0F0F);

    // ワールド行列を設定する
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity(&matIdentity);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &matIdentity);

    // ビュー行列を設定する。ビュー行列は視点と注目点と上方向
    // を使って定義することができる。ここでは、5だけｚ軸方向に
    // 下がって3だけ上に上がった視点を使う。また、原点を見て、
    // 「上方向」はｙ軸の方向とする。
    D3DXMATRIX matView;
    D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&matView, &vFromPt, &vLookatPt, &vUpVec);
    m_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    // 射影行列を設定する。
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, fAspect, 1.0f, 100.0f);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

    // ライトの環境を設定する。
    D3DLIGHT9 light;
    D3DUtil_InitLight(light, D3DLIGHT_DIRECTIONAL, -1.0f, -1.0f, 2.0f);
    m_pd3dDevice->SetLight(0, &light);
    m_pd3dDevice->LightEnable(0, TRUE);
    m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);

    // フォントを再構築する。
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: 各フレームに１度呼び出される。アニメーションの計算を行う
//       始まりの場所になる。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // TODO: 世界を更新する

    // ユーザによる入力を更新する。
    UpdateInput(&m_UserInput);

    // ユーザの入力に応じてワールドの情報を更新する。
    D3DXMATRIX matWorld;
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

    D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
    D3DXMatrixRotationY(&matRotY, m_fWorldRotY);

    D3DXMatrixMultiply(&matWorld, &matRotX, &matRotY);
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateInput()
// Desc: ユーザによる入力を更新する。各フレームに１度呼び出される。
//-----------------------------------------------------------------------------
void CMyD3DApplication::UpdateInput(UserInput* pUserInput)
{
    pUserInput->bRotateUp = (m_bActive && (GetAsyncKeyState(VK_UP) & 0x8000) == 0x8000);
    pUserInput->bRotateDown = (m_bActive && (GetAsyncKeyState(VK_DOWN) & 0x8000) == 0x8000);
    pUserInput->bRotateLeft = (m_bActive && (GetAsyncKeyState(VK_LEFT) & 0x8000) == 0x8000);
    pUserInput->bRotateRight = (m_bActive && (GetAsyncKeyState(VK_RIGHT) & 0x8000) == 0x8000);
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: 各フレームに１度呼び出される。３Ｄのレンダリングの始まりの場所になる。
//       この関数は描画の環境を設定し、画面をクリアして、
//       情景をレンダリングする。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    // ビューポートを初期化する
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0x000000ff, 1.0f, 0L);

    // 描画を始める
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
        // TODO: 世界を描画する

        // ティーポットを表示する
        m_pD3DXMesh->DrawSubset(0);

        // 状態やヘルプを表示する
        RenderText();

        // 描画の終了
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderText()
// Desc: 状態やヘルプを表示する
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 0);
    TCHAR szMsg[MAX_PATH] = TEXT("");

    // ディスプレイの状態を出力する
    FLOAT fNextLine = 40.0f;

    lstrcpy(szMsg, m_strDeviceStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    lstrcpy(szMsg, m_strFrameStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    // やヘルプを出力する
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




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの。独自のイベント処理が行える。
//       (たとえば、マウス、キーボード、メニューの処理).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg, WPARAM wParam,
    LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
    {
        if (m_bLoadingApp)
        {
            // ロード中であることをユーザーに伝える
            // TODO: 必要に応じて変更する
            HDC hDC = GetDC(hWnd);
            TCHAR strMsg[MAX_PATH];
            wsprintf(strMsg, TEXT("Loading... Please wait"));
            RECT rct;
            GetClientRect(hWnd, &rct);
            DrawText(hDC, strMsg, -1, &rct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            ReleaseDC(hWnd, hDC);
        }
        break;
    }

    }

    return CD3DApplication::MsgProc(hWnd, msg, wParam, lParam);
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: デバイス依存のオブジェクトを無効にする。RestoreDeviceObjects()と対になっている。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    // TODO: RestoreDeviceObjects()で生成された全てのオブジェクトを開放する
    m_pFont->InvalidateDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects()と対になっている。
//       アプリケーションが終了するかデバイスが変更される場合に呼ばれる。
//       この関数ではデバイスに依存する全てのオブジェクトを開放する。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    // TODO: InitDeviceObjects()で生成された全てのオブジェクトを開放する
    m_pFont->DeleteDeviceObjects();
    SAFE_RELEASE(m_pD3DXMesh);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: OneTimeSceneInit()と対になっている。
//       アプリケーションが終わる直前に呼ばれる。
//       この関数は、終了する前にそれ自身によって後片付けする機会を与える。
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // TODO: 必要とされる全ての後片付けを実行する
    // フォントの後始末
    SAFE_DELETE(m_pFont);

    return S_OK;
}




