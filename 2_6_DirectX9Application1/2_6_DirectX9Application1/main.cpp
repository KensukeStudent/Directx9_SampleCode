//-------------------------------------------------------------
// File: main.cpp
//
// Desc: テクスチャを張る (DirectX 9 修正版)
//-------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr.h> // DXErr9.h -> DXErr.h に変更
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
// 頂点フォーマット
//-------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

typedef struct {
    FLOAT x, y, z, rhw;
    DWORD color;
    FLOAT tu, tv;
} CUSTOMVERTEX;

//-------------------------------------------------------------
// グローバル変数
//-------------------------------------------------------------
CMyD3DApplication* g_pApp = NULL;
HINSTANCE g_hInst = NULL;

//-------------------------------------------------------------
// Name: WinMain()
//-------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT) {
    CMyD3DApplication d3dApp;
    g_pApp = &d3dApp;
    g_hInst = hInst;

    InitCommonControls();
    if (FAILED(d3dApp.Create(hInst)))
        return 0;

    return d3dApp.Run();
}

//-------------------------------------------------------------
// Name: InitDeviceObjects()
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects() {
    //if (FAILED(D3DXCreateTextureFromFileEx(
    //    m_pd3dDevice, "earth.bmp", D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0,
    //    D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR,
    //    D3DX_FILTER_LINEAR, 0, NULL, NULL, &m_pTexture))) {
    //    return E_FAIL;
    //}

    //m_pFont->InitDeviceObjects(m_pd3dDevice);
    return S_OK;
}

//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects() {
    if (m_pTexture) {
        m_pTexture->Release();
        m_pTexture = NULL;
    }

    // m_pFont->DeleteDeviceObjects();
    return S_OK;
}

//-------------------------------------------------------------
// Name: InvalidateDeviceObjects()
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects() {
    if (m_pVB) {
        m_pVB->Release();
        m_pVB = NULL;
    }

    // m_pFont->InvalidateDeviceObjects();
    return S_OK;
}

//-------------------------------------------------------------
// Name: Render()
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render() {
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x000000ff, 1.0f, 0L);

    if (SUCCEEDED(m_pd3dDevice->BeginScene())) {
        m_pd3dDevice->SetTexture(0, m_pTexture);
        m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
        m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
        m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

        // RenderText();
        m_pd3dDevice->EndScene();
    }
    return S_OK;
}
