//-------------------------------------------------------------
// File: main.cpp
//
// Desc: �A���t�@�u�����f�B���O
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
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct {
    FLOAT x, y, z, rhw;    // �X�N���[�����W�ł̈ʒu
    DWORD color;           // ���_�F
    FLOAT tu, tv;          // �e�N�X�`���[���W
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
	m_pEarthTexture				= NULL;
	m_pCloudTexture				= NULL;
	m_pVB						= NULL;

	m_dwCreationWidth           = 300;
    m_dwCreationHeight          = 300;
    m_strWindowTitle            = TEXT( "main" );
    m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
	m_bStartFullscreen			= false;
	m_bShowCursorWhenFullscreen	= false;

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
//		�E�B���h�E�̏�������IDirect3D9�̏������͏I����Ă܂��B
//		�����ALPDIRECT3DDEVICE9 �̏������͏I����Ă��܂���B
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

static HRESULT LoadTexture(LPCSTR pSrcFile, LPDIRECT3DTEXTURE9* pTexture)
{
    D3DXIMAGE_INFO info;

    // �t�@�C������e�N�X�`���[�𐶐�����
    HRESULT hr = D3DXCreateTextureFromFileExA(
        g_pApp->GetD3DDevice(),	// Direct3DDevice
        pSrcFile,		// �t�@�C����
        D3DX_DEFAULT,		// ����(D3DX_DEFAULT�Ńt�@�C�����画��)
        D3DX_DEFAULT,		// ����(D3DX_DEFAULT�Ńt�@�C�����画��)
        1,			// �~�b�v�}�b�v�̐�
        0,			// �g�p�p�r
        D3DFMT_A8R8G8B8,	// �t�H�[�}�b�g
        D3DPOOL_MANAGED,	// �������̊Ǘ��ݒ�
        D3DX_FILTER_NONE,	// �t�B���^�[�ݒ�
        D3DX_DEFAULT,		// �~�b�v�}�b�v�t�B���^�[�̐ݒ�
        0x00000000,		// �J���[�L�[
        &info,			// �摜���
        NULL,			// �p���b�g�f�[�^
        pTexture);		// ���������e�N�X�`���[�̊i�[��
    
    return hr;
}


//-------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: �f�o�C�X���������ꂽ��̏����������܂��B
//		�t���[���o�b�t�@�t�H�[�}�b�g��f�o�C�X�̎�ނ��ς����
//		��ɒʉ߂��܂��B
//		�����Ŋm�ۂ�����������DeleteDeviceObjects()�ŊJ�����܂�
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // �n���e�N�X�`���[��ǂݍ���
    if(FAILED(LoadTexture(
					 "earth.bmp"			// �t�@�C����
					, &m_pEarthTexture ) ) )// �e�N�X�`���I�u�W�F�N�g
        return E_FAIL;

    // �_�e�N�X�`���[��ǂݍ���
    if( FAILED(LoadTexture( 
					  "cloud.bmp"			// �t�@�C����
					, &m_pCloudTexture ) ) )// �e�N�X�`���I�u�W�F�N�g
        return E_FAIL;

	// �t�H���g
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: ��ʂ̃T�C�Y���ύX���ꂽ�����ɌĂ΂�܂��B
//		�m�ۂ�����������InvalidateDeviceObjects()�ŊJ�����܂��B
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // ���_�ƐF���
    CUSTOMVERTEX vertices[] = {
		//   x,     y,      z,  rhw,   ���_�̐F   ( ��    ��   ��   �� )  U  V
        {  50.0f,  50.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0x80), 0, 0},
        { 250.0f,  50.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0x80), 1, 0},
        {  50.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0x80), 0, 1},
        { 250.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0x80), 1, 1},
    };
	
	// ���_�o�b�t�@�𐶐�����
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 
				4*sizeof(CUSTOMVERTEX),		// ���_�o�b�t�@�̃T�C�Y
                0, D3DFVF_CUSTOMVERTEX,		// �g�p�@�A���_�t�H�[�}�b�g
                D3DPOOL_DEFAULT,			// �������N���X
				&m_pVB, NULL )))			// ���_�o�b�t�@���\�[�X �\��� 
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

    // �������@��ݒ肷��
	TSS( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);// �����̐�������Z����
    TSS( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    TSS( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );

#define ALPHA_TYPE 0	// ������ς���ƁA�������@���ς��

#if ALPHA_TYPE == 0
	// ���`���� C = Cd(1-As)+CsAs
	RS(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	RS(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
#else
#if ALPHA_TYPE == 1
	// ���Z���� C = Cd+CsAs
	RS(D3DRS_DESTBLEND, D3DBLEND_ONE);
	RS(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
#else
#if ALPHA_TYPE == 2
	// ���Z���� C = Cd-CsAs
	RS(D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT);
	RS(D3DRS_DESTBLEND, D3DBLEND_ONE);
	RS(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
#else
#if ALPHA_TYPE == 3
	// ��Z���� C = Cd*Cs
	RS(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	RS(D3DRS_SRCBLEND, D3DBLEND_ZERO);
#else
#if ALPHA_TYPE == 4
	// �Ă����� Cd*Cd
	RS(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
	RS(D3DRS_SRCBLEND, D3DBLEND_ZERO);
#else
#if ALPHA_TYPE == 5
	// �l�K�|�W���] C = (1-Cd)*Cs
	RS(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RS(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
#else
#if ALPHA_TYPE == 6
	// �s���� C = Cs
	RS(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RS(D3DRS_SRCBLEND, D3DBLEND_ONE);
#endif
#endif
#endif
#endif
#endif
#endif
#endif

    // �����_�����O��Ԃ̐ݒ�
    RS( D3DRS_DITHERENABLE,   FALSE );
    RS( D3DRS_SPECULARENABLE, FALSE );
    RS( D3DRS_ZENABLE,        TRUE );
    RS( D3DRS_AMBIENT,        0x000F0F0F );
    
    // �e�N�X�`���[�Ɋւ���ݒ�
    TSS( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    TSS( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    TSS( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

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
    // ��ʂ��N���A����
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0L );

	//---------------------------------------------------------
	// �`��
	//---------------------------------------------------------
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
	    m_pd3dDevice->SetStreamSource( 0, m_pVB, 0, sizeof(CUSTOMVERTEX) );
	    m_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

		RS(D3DRS_ALPHABLENDENABLE, FALSE);// �������ŕ`���Ȃ�
		m_pd3dDevice->SetTexture( 0, m_pEarthTexture );
		m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
		
		RS(D3DRS_ALPHABLENDENABLE, TRUE);// �������ŕ`��
		m_pd3dDevice->SetTexture( 0, m_pCloudTexture );
		m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

		RenderText();	// ��ʂ̏�Ԃ�w���v��\������

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
    SAFE_RELEASE( m_pVB );				// ���_�o�b�t�@

    m_pFont->InvalidateDeviceObjects();	// �t�H���g

    return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() �ō쐬�����I�u�W�F�N�g���J������
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pCloudTexture );	// �e�N�X�`��
    SAFE_RELEASE( m_pEarthTexture );	// �e�N�X�`��

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
    SAFE_DELETE( m_pFont );	// �t�H���g

    return S_OK;
}
