//-------------------------------------------------------------
// File: main.cpp
//
// Desc: 3D�\��
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



//-------------------------------------------------------------
// ���_�t�H�[�}�b�g
//-------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)// ������ �ύX

typedef struct {
    FLOAT x, y, z;    // ������ �ύX�F�X�N���[�����W�ł̈ʒu
    DWORD color;      // ���_�F
} CUSTOMVERTEX;



//-------------------------------------------------------------
// �O���[�o���ϐ�
//-------------------------------------------------------------
CMyD3DApplication* g_pApp  = NULL;
HINSTANCE          g_hInst = NULL;


//-------------------------------------------------------------
// Name: WinMain()
// Desc: ���C���֐�
//-------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    g_pApp  = &d3dApp;
    g_hInst = hInst;

    InitCommonControls();
    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;

    return d3dApp.Run();
}




//-------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: �A�v���P�[�V�����̃R���X�g���N�^
//-------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_pVB                       = NULL;

    m_dwCreationWidth           = 300;
    m_dwCreationHeight          = 300;
    m_strWindowTitle            = TEXT( "main" );
    m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
    m_bStartFullscreen          = false;
    m_bShowCursorWhenFullscreen = false;

    m_pFont                     = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_bLoadingApp               = TRUE;
}




//-------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: �f�X�g���N�^
//-------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: ��x�����s��������
//        �E�B���h�E�̏�������IDirect3D9�̏������͏I����Ă܂��B
//        �����ALPDIRECT3DDEVICE9 �̏������͏I����Ă��܂���B
//-------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // ���[�f�B���O���b�Z�[�W��\������
    SendMessage( m_hWnd, WM_PAINT, 0, 0 );

    m_bLoadingApp = FALSE;

    return S_OK;
}




//-------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: �������̎��ɌĂ΂�܂��B�K�v�Ȕ\�͂��`�F�b�N���܂��B
//-------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS9* pCaps,
                     DWORD dwBehavior,    D3DFORMAT Format )
{
    UNREFERENCED_PARAMETER( Format );
    UNREFERENCED_PARAMETER( dwBehavior );
    UNREFERENCED_PARAMETER( pCaps );
    
    BOOL bCapsAcceptable;

    // �O���t�B�b�N�X�{�[�h���v���O���������s����\�͂����邩�m�F����
    bCapsAcceptable = TRUE;

    if( bCapsAcceptable )         
        return S_OK;
    else
        return E_FAIL;
}




//-------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: �f�o�C�X���������ꂽ��̏����������܂��B
//        �t���[���o�b�t�@�t�H�[�}�b�g��f�o�C�X�̎�ނ��ς����
//        ��ɒʉ߂��܂��B
//        �����Ŋm�ۂ�����������DeleteDeviceObjects()�ŊJ�����܂�
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // �t�H���g
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: ��ʂ̃T�C�Y���ύX���ꂽ�����ɌĂ΂�܂��B
//        �m�ۂ�����������InvalidateDeviceObjects()�ŊJ�����܂��B
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // ���_�ƐF���
    CUSTOMVERTEX vertices[] = {
        //   x,     y,      z,    ���_�̐F ( ��   ��   ��   �� )
        {  -1.0f,-1.0f, 0.0f, D3DCOLOR_RGBA(0xff,0x00,0x00,0xff)},
        {   1.0f,-1.0f, 0.0f, D3DCOLOR_RGBA(0x00,0xff,0x00,0xff)},
        {   0.0f, 1.0f, 0.0f, D3DCOLOR_RGBA(0x00,0x00,0xff,0xff)},
    };
    
    // ���_�o�b�t�@�𐶐�����
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 
                3*sizeof(CUSTOMVERTEX),        // ���_�o�b�t�@�̃T�C�Y
                0, D3DFVF_CUSTOMVERTEX,        // �g�p�@�A���_�t�H�[�}�b�g
                D3DPOOL_DEFAULT,            // �������N���X
                &m_pVB, NULL )))            // ���_�o�b�t�@���\�[�X �\��� 
        return E_FAIL;

    // ���_�o�b�t�@�ɏ����i�[����
    VOID* pVertices;
    if(FAILED( m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
        return E_FAIL;
    memcpy( pVertices, vertices, sizeof(vertices) );
    m_pVB->Unlock();

    // ��������Z�k�`������Ă݂�
    #define RS   m_pd3dDevice->SetRenderState
    #define TSS  m_pd3dDevice->SetTextureStageState
    #define SAMP m_pd3dDevice->SetSamplerState

    // �����_�����O��Ԃ̐ݒ�
    RS( D3DRS_DITHERENABLE,   FALSE );
    RS( D3DRS_SPECULARENABLE, FALSE );
    RS( D3DRS_ZENABLE,        TRUE );
    RS( D3DRS_AMBIENT,        0x000F0F0F );
    
    // �J�����O�����Ȃ��ŁA���ʕ`�悷��
    RS( D3DRS_CULLMODE,        D3DCULL_NONE );
    // �����v�Z�����Ȃ��ŁA���_�̐F�����̂܂܎g���B
    RS( D3DRS_LIGHTING,        FALSE );

	TSS( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    TSS( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );

    // �t�H���g
    m_pFont->RestoreDeviceObjects();

    return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: ���t���[���Ă΂�܂��B�A�j���̏����Ȃǂ��s���܂��B
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    return S_OK;
}




//-------------------------------------------------------------
// Name: Render()
// Desc: ��ʂ�`�悷��.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    D3DXMATRIXA16 mWorld, mView, mProj;

	// ��ʂ��N���A����
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         D3DCOLOR_RGBA(0x00,0x00,0x00,0x00), 1.0f, 0L );

    //---------------------------------------------------------
    // �`��
    //---------------------------------------------------------
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
        // ---------------------------------------------------------
        // ���[���h�s���ݒ肷��
        // ---------------------------------------------------------
        UINT  iTime  = timeGetTime() % 1000; // �~���b�Ŏ��Ԃ𒲂ׂ�
        FLOAT fAngle = iTime * (2.0f*D3DX_PI) / 1000.0f;//�p�x�����
        D3DXMatrixRotationY( &mWorld, fAngle );// Y���̉�]�s��
        m_pd3dDevice->SetTransform( D3DTS_WORLD, &mWorld );

        // ---------------------------------------------------------
        // �r���[�s���ݒ肷��
        // ---------------------------------------------------------
        D3DXVECTOR3 vEyePt   ( 0.0f, 3.0f,-5.0f ); // ���ړ_
        D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f ); // �J�����̈ʒu
        D3DXVECTOR3 vUp      ( 0.0f, 1.0f, 0.0f ); // ���������
        D3DXMatrixLookAtLH( &mView, &vEyePt, &vLookatPt, &vUp );
        m_pd3dDevice->SetTransform( D3DTS_VIEW, &mView );

        // ---------------------------------------------------------
        // �ˉe�s���ݒ肷��
        // ---------------------------------------------------------
        D3DXMatrixPerspectiveFovLH( &mProj
                            , D3DX_PI/4      // ����p
                            , 1.0f           // �A�X�y�N�g��
                            , 1.0f, 100.0f );// �O���A����N���b�v��
        m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &mProj );

        // ---------------------------------------------------------
        // �|���S����`�悷��
        // ---------------------------------------------------------
		m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(CUSTOMVERTEX) );
        m_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 1 );

        RenderText();    // ��ʂ̏�Ԃ�w���v��\������

        // �`��̏I��
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-------------------------------------------------------------
// Name: RenderText()
// Desc: ��Ԃ�w���v����ʂɕ\������
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor        = D3DCOLOR_ARGB(255,255,255,0);
    TCHAR szMsg[MAX_PATH] = TEXT("");

    // ��ʂ̏�Ԃ�\������
    FLOAT fNextLine = 40.0f; 

    lstrcpy( szMsg, m_strDeviceStats );
    fNextLine -= 20.0f;
    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    lstrcpy( szMsg, m_strFrameStats );
    fNextLine -= 20.0f;
    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

    // ����@��p�����[�^��\������
    fNextLine = (FLOAT) m_d3dsdBackBuffer.Height; 
    lstrcpy( szMsg, TEXT("Press 'F2' to configure display") );
    fNextLine -= 20.0f; m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );
    return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc ���I�[�o�[���C�h��������
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT msg,
                                 WPARAM wParam, LPARAM lParam )
{
    switch( msg )
    {
        case WM_PAINT:
        {
            if( m_bLoadingApp )
            {
                // ���[�h��
                HDC hDC = GetDC( hWnd );
                TCHAR strMsg[MAX_PATH];
                wsprintf(strMsg, TEXT("Loading... Please wait"));
                RECT rct;
                GetClientRect( hWnd, &rct );
                DrawText( hDC, strMsg, -1, &rct
                        , DT_CENTER|DT_VCENTER|DT_SINGLELINE );
                ReleaseDC( hWnd, hDC );
            }
            break;
        }

    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}




//-------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: RestoreDeviceObjects() �ō쐬�����I�u�W�F�N�g�̊J��
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    SAFE_RELEASE( m_pVB );        // ���_�o�b�t�@

    m_pFont->InvalidateDeviceObjects();    // �t�H���g

    return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() �ō쐬�����I�u�W�F�N�g���J������
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	// �t�H���g
    m_pFont->DeleteDeviceObjects();

    return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: �I�����钼�O�ɌĂ΂��
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE( m_pFont );    // �t�H���g

    return S_OK;
}




