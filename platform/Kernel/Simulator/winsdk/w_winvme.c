/*****************************************************************************
 * Copyright (C) Anyka 2005
 *****************************************************************************
 *   Project:
 *****************************************************************************
 * $Workfile: $
 * $Revision: $
 *     $Date: $
 *****************************************************************************
 * Description:
 *
 *****************************************************************************
 * 15.04.2003    KR36712    add popup menu for termination of application
 *                          MainHandle_WM_CREATE ()
 *                          MainHandle_WM_RBUTTONDOWN ()
 *                          MainHandle_WM_COMMAND ()
 *
 *****************************************************************************
*/

#include <stdio.h>
#include <assert.h>
#include <windows.h>
#include <Windowsx.h>
#include <TCHAR.h>
#include "Gbl_Define.h"
#include "Eng_Debug.h"
#include "Gbl_Global.h"
#include "../res/resource.h"
#include "w_winvme.h"
#include "w_comport.h"
#include "w_vtimer.h"
#include "fwl_lcd.h"
#include "vme.h"
#include "Eng_Graph.h"
#include "Eng_Debug.h"

/*****************************************************************************
 * defines
 *****************************************************************************
*/
#if (USE_COLOR_LCD)
    #if (LCD_TYPE == 0)
        #if (LCD_HORIZONTAL == 1)
            #define PC_LCD_LEFT     56
            #define PC_LCD_TOP      38
        #else
            #define PC_LCD_LEFT     56
            #define PC_LCD_TOP      22
        #endif
    #endif
    #if (LCD_TYPE == 1)
        #if (LCD_HORIZONTAL == 1)
            #define PC_LCD_LEFT     56
            #define PC_LCD_TOP      46
        #else
            #define PC_LCD_LEFT     56
            #define PC_LCD_TOP      24
        #endif
    #endif
    #if (LCD_TYPE == 2)
        #define PC_LCD_LEFT     56
        #define PC_LCD_TOP      18
    #endif
    #if (LCD_TYPE == 3)
        #if (LCD_HORIZONTAL == 1)
           #define PC_LCD_LEFT     138
           #define PC_LCD_TOP      34
        #else
           #define PC_LCD_LEFT     102
           #define PC_LCD_TOP      45
        #endif
    #endif
#else
    #if (LCD_TYPE == 0)
        #define PC_LCD_LEFT     56
        #define PC_LCD_TOP      17
    #endif
    #if (LCD_TYPE == 1)
        #define PC_LCD_LEFT     57
        #define PC_LCD_TOP      34
    #endif
    #if (LCD_TYPE == 2)
        #define PC_LCD_LEFT     75
        #define PC_LCD_TOP      30
    #endif
#endif

#define MAX_MSG_BUFFER  1024
static int m_WindowWidth = 0;
static int m_WindowHeight = 0;
static T_TIMER Touch_Keep_Timer = ERROR_TIMER;
extern T_BOOL vtimer_interrupt_handler_WIN32(T_TIMER timer_id);
extern T_BOOL keypad_interrupt_handler_WIN32(T_U8 MouseState, T_U16 x, T_U16 y);
#ifdef SUPPORT_TSR
extern T_VOID Tsr_interrupt_handle_win32(T_U16 x,T_U16 y,T_DRV_TOUCH_MEG type);
extern T_VOID Touch_Keep(T_TIMER timer_id, T_U32 delay);
#endif
static char szTitle[] = "PC SIM MMI";

static char szWindowClass[] = "PC_SIM";
const char WMVMEOutputFile[] = "winVMEOutput.txt";

// *** windows user messages
#define WM_USER_SCHEDULE_VME        WM_USER+1
#define WM_USER_INIT_VME            WM_USER+2

// debug window
#define     DEBUG_WINDOW_NAME   "Debug Info"
#define     KEY_BACKSPACE       0x08
#define     STRING_LEN          4096
#define     EDIT_SCREEN_X       320
#define     EDIT_SCREEN_Y       240
#define     SCREEN_X            (EDIT_SCREEN_X + 2 + 16 + 2)
#define     SCREEN_Y            (EDIT_SCREEN_Y + 2 + 16 + 2)
#define     WINDOW_XSIZE        (SCREEN_X + 8)
#define     WINDOW_YSIZE        (SCREEN_Y + 8 + 38)
#define     EDITID              1
#define     MAX_EDIT_BUF_SIZE   (0x7FFE)
#define     EDIT_BUF_SIZE       (0x6000)
#define     EDIT_BUF_DEC_SIZE   (0x2000)

/*****************************************************************************
 * globals
 *****************************************************************************
*/
static HINSTANCE    hApplInstance = NULL;

// *** LCD
static HWND         hwndLCD;
static HDC          hdcLCD;
static HBITMAP      hbitmapSkinBmp;

static DWORD        LCD0Left;
static DWORD        LCD0Top;

static T_BOOL       s_MainLcdOn = AK_FALSE;

static HWND         hWnd_DebugWnd;
volatile static BOOL isUsed;
extern T_VOID akmain(T_VOID);

/*****************************************************************************
 * functions
 *****************************************************************************
*/
static int winvme_SetWindowShape (HWND  hWnd,HGDIOBJ hBitMap)
{

    HDC         hDC;
    HDC         hBmpDC;
    RECT        stRect;
    DWORD       dwX,dwY,dwStartX;
    HRGN        hRgn;
    COLORREF    rgbBack;
    COLORREF    rgbTmp;
    DWORD       Vecx;
    HRGN        Heax;
    DWORD       Veax;

    GetWindowRect(hWnd,&stRect);
    ShowWindow(hWnd,SW_HIDE);
    MoveWindow(hWnd,stRect.left,stRect.top,m_WindowWidth,m_WindowHeight,FALSE);

    hDC = GetDC(hWnd);
    hBmpDC = CreateCompatibleDC(hDC);
    SelectObject(hBmpDC,hBitMap);

//*************** 计算窗口形状 ***************************************
    rgbBack = GetPixel(hBmpDC,0,0);
    hRgn = CreateRectRgn(0,0,0,0);

    dwY = 0;
    while  (1)
    {
        dwX = 0;
        dwStartX = -1;
        if (dwY == 250)
            dwY = 250;
        while (1)
        {
            rgbTmp = GetPixel(hBmpDC, dwX, dwY);
            if  (dwStartX == -1)
            {
                if  (rgbTmp !=  rgbBack)
                    dwStartX = dwX;
            }
            else if  (rgbTmp ==  rgbBack)
            {
                Vecx =  dwY;
                Vecx ++;
                Heax = CreateRectRgn(dwStartX, dwY, dwX,Vecx);
                CombineRgn(hRgn,hRgn,Heax,RGN_OR);
                DeleteObject(Heax);
                dwStartX = -1;
            }
            else if (dwX >=  (DWORD)m_WindowWidth - 1)
            {
                  Vecx =  dwY;
                  Vecx ++;
                  Heax = CreateRectRgn(dwStartX,dwY,dwX,Vecx);
                  CombineRgn(hRgn,hRgn,Heax,RGN_OR);
                  DeleteObject(Heax);
                  dwStartX = -1;
             }
            dwX ++;
            Veax = dwX;
            if (Veax >=  (DWORD)m_WindowWidth)
                break;
        }
        dwY ++;
        Veax = dwY;
        if (Veax >=  (DWORD)m_WindowHeight)
            break;
    }

    SetWindowRgn(hWnd, hRgn,TRUE);
//********************************************************************
    BitBlt(hDC,0,0, m_WindowWidth, m_WindowHeight,hBmpDC,0,0,SRCCOPY);
    DeleteDC(hBmpDC);
    ReleaseDC(hWnd, hDC);
    InvalidateRect(hWnd,NULL,-1);

    return 0;
}

extern T_U8 *lcd_get_disp_buf_addr_large(T_VOID);

static void winvme_RefreshLCD(void)
{
    WORD    i,j;
    BYTE    *pPixelBase;
    BYTE    r,g,b;

    if (s_MainLcdOn)
    {
        pPixelBase  = (BYTE *)lcd_get_disp_buf_addr_large();

        for (j = 0; j < GRAPH_HEIGHT; j++)
        {
            for (i = 0; i < GRAPH_WIDTH; i++)
            {
                r = *pPixelBase++;
                g = *pPixelBase++;
                b = *pPixelBase++;

                SetPixel(hdcLCD, LCD0Left + i, LCD0Top + j, RGB(r, g, b));
            }
        }
    }
    else
    {
        for (j = 0; j < GRAPH_HEIGHT; j++)
        {
            for (i = 0; i < GRAPH_WIDTH; i++)
            {
                SetPixel(hdcLCD, LCD0Left + i, LCD0Top + j, RGB(0, 0, 0));
            }
        }
    }

    return;
}

static BOOL winvme_InTouchScrRect(WORD x, WORD y)
{
    RECT lcdRect = {PC_LCD_LEFT, PC_LCD_TOP, MAIN_LCD_WIDTH+PC_LCD_LEFT-1, MAIN_LCD_HEIGHT+PC_LCD_TOP-1};
    #ifdef SUPPORT_TSR
        if (((x >= lcdRect.left) && (x <= lcdRect.right)) && \
                ((y >= lcdRect.top) && (y <= lcdRect.bottom)))
            return TRUE;
        else
            return FALSE;
    #else
        return AK_FALSE;
    #endif
}

static LRESULT MainHandle_WM_CREATE(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    // start and init vme engine
    PostMessage (hWnd, WM_USER_INIT_VME, 0, 0);
    return 0;
}

static LRESULT MainHandle_WM_RBUTTONDOWN(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    HMENU   MainMenu;
    HMENU   PopupMenuSys;
    RECT    WndRect;

    MainMenu        =   LoadMenu (hApplInstance, MAKEINTRESOURCE (IDR_MENU_RBUTTON_POPUP));
    if (NULL != MainMenu)
    {

        PopupMenuSys    =   GetSubMenu (MainMenu, 0);
        if (NULL != PopupMenuSys)
        {

            GetWindowRect (hWnd, &WndRect);

            TrackPopupMenu (
                PopupMenuSys,
                TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                WndRect.left + GET_X_LPARAM(lParam),
                WndRect.top + GET_Y_LPARAM(lParam),
                0,
                hWnd,
                NULL
            );
        }
    }
    return 0;
}

static LRESULT MainHandle_WM_COMMAND(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (LOWORD (wParam))
    {
        case IDM_CLOSE:
            VME_Terminate();
            break;
    }

    return 0;
}

#define REFRESH_INTERVAL    2
static int refresh_interval = REFRESH_INTERVAL;
static char refresh_flag = 1;
static LRESULT CALLBACK winvme_MainWndProc(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    PAINTSTRUCT ps;
    HDC         hDC, hBmpDC;
    HBITMAP     hBitmap;
    WORD        x, y;

    x = LOWORD( lParam );
    y = HIWORD( lParam );

    switch (message)
    {
        case WM_CREATE:
            MainHandle_WM_CREATE (hWnd, message, wParam, lParam);
            break;

        case WM_COMMAND:
            MainHandle_WM_COMMAND (hWnd, message, wParam, lParam);
            break;

        case WM_SIZE:
        case WM_MOVE:
        case WM_PAINT:
            hDC = BeginPaint(hWnd, &ps);
            hBmpDC = CreateCompatibleDC( hDC );
            hBitmap = (HBITMAP)SelectObject( hBmpDC, hbitmapSkinBmp );
            BitBlt( hDC, 0, 0, m_WindowWidth, m_WindowHeight, hBmpDC, 0, 0, SRCCOPY );
            DeleteDC( hBmpDC );
            EndPaint(hWnd, &ps);
            winvme_RefreshLCD();
            break;

        case WM_RBUTTONDOWN:
            MainHandle_WM_RBUTTONDOWN (hWnd, message, wParam, lParam);
            break;

        case WM_LBUTTONDOWN:
            if (winvme_InTouchScrRect(x, y))
            {
                #ifdef SUPPORT_TSR
                Tsr_interrupt_handle_win32((T_U16)(x-PC_LCD_LEFT),(T_U16)(y-PC_LCD_TOP),DRV_TOUCH_DOWN);
                Touch_Keep_Timer = vtimer_start(200,AK_TRUE,Touch_Keep);
                #endif
                break;
            }
            else if(keypad_interrupt_handler_WIN32(1, x, y))
                break;

            /* for dragging the window */
            SendMessage(hwndLCD, WM_NCLBUTTONDOWN,HTCAPTION,0);
            break;

        case WM_MOUSEMOVE:
            if (winvme_InTouchScrRect(x, y) && (wParam & MK_LBUTTON))
            {
                #ifdef SUPPORT_TSR
                Tsr_interrupt_handle_win32((T_U16)(x-PC_LCD_LEFT),(T_U16)(y-PC_LCD_TOP),DRV_TOUCH_DOWN);
                vtimer_stop(Touch_Keep_Timer);
                Touch_Keep_Timer = vtimer_start(200,AK_TRUE,Touch_Keep);
                #endif
                break;
            }
            else
                keypad_interrupt_handler_WIN32(2, x, y);
            break;

        case WM_LBUTTONUP:
            Touch_Keep_Timer = vtimer_stop(Touch_Keep_Timer);
            if (winvme_InTouchScrRect(x, y) )
            {
                #ifdef SUPPORT_TSR
                Tsr_interrupt_handle_win32((T_U16)(x-PC_LCD_LEFT),(T_U16)(y-PC_LCD_TOP),DRV_TOUCH_UP);
                #endif
                break;
            }
            else
                keypad_interrupt_handler_WIN32(0, x, y);
            break;

        case WM_TIMER:
            //vtimer_interrupt_handler_WIN32((T_TIMER)wParam);
            refresh_interval++;
            if (refresh_interval >= REFRESH_INTERVAL)
            {
                refresh_interval = 0;
                if (refresh_flag)
                {
                    SendMessage(hwndLCD, WM_PAINT, 0, 0);
                    refresh_flag = 0;
                }
            }
            //vme_engine_HandleTimerQueue (wParam);
            //winvme_StopTimer (wParam);    /* see: timer500handler.c, handle_pub_timer() --ZouMai */
            break;

        case WM_USER_INIT_VME:
            akmain();
            break;

        case WM_USER_SCHEDULE_VME:
            //VME_EvtQueueHandle();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

static LRESULT CALLBACK winvme_WndProcLCD2(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (message)
    {
        case WM_CREATE:
            break;

        case WM_LBUTTONDOWN:
            SendMessage(hwndLCD, WM_NCLBUTTONDOWN,HTCAPTION,0);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        //case WM_PAINT:
            //winvme_RefreshLCD(LCD_S);
            //break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

static LRESULT CALLBACK winvme_DebugWindow(
    HWND    hWnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    switch (message)
    {
        case WM_CREATE:
            // Create the edit control child window
            hWnd_DebugWnd = CreateWindow (TEXT("edit"), NULL,
                WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                        WS_BORDER | ES_LEFT | ES_MULTILINE | ES_READONLY |
                        ES_AUTOVSCROLL,
                        0, 0, SCREEN_X, SCREEN_Y,
                hWnd, (HMENU)EDITID, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
//          SetFont(hWnd_DebugWnd);
            SendMessage(hWnd_DebugWnd, EM_SETLIMITTEXT, MAX_EDIT_BUF_SIZE, 0L);
            return 0;
        case WM_LBUTTONDOWN:
//          SendMessage(hwndLCD[LCD_S], WM_NCLBUTTONDOWN,HTCAPTION,0);
            break;
        case WM_DESTROY:
//          PostQuitMessage(0);
            break;
        case WM_SIZE :
            MoveWindow(hWnd_DebugWnd, 0, 0, LOWORD (lParam), HIWORD (lParam), TRUE);
            return 0 ;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

static ATOM winvme_RegisterClass(
    HINSTANCE   hInstance
)
{
    WNDCLASSEX  wcex;
    ATOM        aTom;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)winvme_MainWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    aTom =  RegisterClassEx(&wcex);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)winvme_WndProcLCD2;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    aTom =  RegisterClassEx(&wcex);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = (WNDPROC)winvme_DebugWindow;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = NULL; // LoadIcon(hInstance, (LPCTSTR)PCDEMO_ICON);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = DEBUG_WINDOW_NAME;
    wcex.hIconSm        = NULL; // LoadIcon(wcex.hInstance, (LPCTSTR)PCDEMO_ICON);

    aTom =  RegisterClassEx(&wcex);

    return aTom;
}

static BOOL winvme_InitInstance(
    HINSTANCE   hInstance,
    int         nCmdShow
)
{
    HWND        hWnd;
    RECT        clientRect;
    BITMAP      stBmp;

#ifdef DEBUG_OUTPUT_TO_WINDOW
    HWND        hDebugWnd;

    hDebugWnd = CreateWindow(DEBUG_WINDOW_NAME, DEBUG_WINDOW_NAME,
                  WS_OVERLAPPEDWINDOW,
                  CW_USEDEFAULT, CW_USEDEFAULT,
                  WINDOW_XSIZE,WINDOW_YSIZE,
                  NULL, NULL, hInstance, NULL);

    if (hDebugWnd == NULL)
        return FALSE;

    ShowWindow(hDebugWnd, nCmdShow);
    UpdateWindow(hDebugWnd);
#endif

    hWnd =   CreateWindow(
                        szWindowClass,
                        szTitle,
                        WS_POPUP,  //| WS_BORDER,
                        CW_USEDEFAULT,
                        0,
                        CW_USEDEFAULT,
                        0,
                        NULL,
                        NULL,
                        hInstance,
                        NULL
                    );
    if (!hWnd)
    {
        return FALSE;
    }
#if (USE_COLOR_LCD)
    #if (LCD_TYPE == 0)
        #if (LCD_HORIZONTAL == 1)
            hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_128X160H));
        #else
            hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_128X160));
        #endif
    #endif
    #if (LCD_TYPE == 1)
        #if (LCD_HORIZONTAL == 1)
            hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_176X220H));
        #else
            hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_176X220));
        #endif
    #endif
    #if (LCD_TYPE == 2)
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_128X128));
    #endif
    #if (LCD_TYPE == 3)
    #if (LCD_HORIZONTAL == 1)
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_240X320H));
    #else
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_CLR_240X320));
    #endif
    #endif
#else
    #if (LCD_TYPE == 0)
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BW_128X64));
    #endif
    #if (LCD_TYPE == 1)
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BW_128X32));
    #endif
    #if (LCD_TYPE == 2)
        hbitmapSkinBmp = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BW_96X40));
    #endif
#endif
    GetObject(hbitmapSkinBmp,sizeof(BITMAP),&stBmp);
    m_WindowWidth = stBmp.bmWidth;
    m_WindowHeight = stBmp.bmHeight;

    clientRect.left     = ( GetSystemMetrics( SM_CXSCREEN ) - m_WindowWidth ) / 2;
    clientRect.top      = ( GetSystemMetrics( SM_CYSCREEN ) - m_WindowHeight ) / 2;
    clientRect.right    = clientRect.left + m_WindowWidth;
    clientRect.bottom   = clientRect.top + m_WindowHeight;

    SetWindowPos(
        hWnd,
        HWND_TOP,
        clientRect.left,
        clientRect.top,
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top,
        SWP_NOZORDER|SWP_NOACTIVATE
    );

    hwndLCD  = hWnd;
    hdcLCD   = GetDC( hWnd );

    if (hbitmapSkinBmp == NULL)
    {
        MessageBox (NULL, "No skin file!", ERROR, MB_OK | MB_ICONERROR);
        return FALSE;
    }

    winvme_SetWindowShape(hWnd,(HGDIOBJ)hbitmapSkinBmp);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

int APIENTRY WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
)
{
    MSG msg;

    hApplInstance   =   hInstance;

    comport_Init();

    remove(WMVMEOutputFile); //删除输出文件
    /**
    * init display
    */
    LCD0Left = PC_LCD_LEFT;
    LCD0Top  = PC_LCD_TOP;

#if(NO_DISPLAY == 0)
    lcd_turn_off();
#endif

    // Initialize global strings
    winvme_RegisterClass (hInstance);

    // Perform application initialization:
    if (!winvme_InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseDC( hwndLCD, hdcLCD );

    DeleteObject(hbitmapSkinBmp);

    return msg.wParam;
}

/*****************************************************************************
 * interface
 *****************************************************************************
*/
/***
* application
*/
void winvme_ScheduleVMEEngine(void)
{
    PostMessage (hwndLCD, WM_USER_SCHEDULE_VME, 0, 0);
    return;
}

void winvme_CloesAppl(void)
{
    PostMessage(hwndLCD, WM_CLOSE, 0, 0);
}

VOID TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	vtimer_interrupt_handler_WIN32(0);
}

/***
* timer
*/
unsigned int winvme_StartTimer(unsigned int uiTimeOut, unsigned int uiTimerId)
{
    return SetTimer(hwndLCD, uiTimerId, uiTimeOut, (TIMERPROC) TimerCallback);
}

void winvme_StopTimer(unsigned int uiTimerId)
{
    KillTimer(hwndLCD, uiTimerId);
    return;
}

/***
* display
*/
void winvme_DisplayOn(void)
{
    s_MainLcdOn = AK_TRUE;
}

void winvme_DisplayOff(void)
{
    s_MainLcdOn = AK_FALSE;
}

void winvme_DisplayUpdate(void)
{
    refresh_interval = 0;
    refresh_flag = 1;
    //SendMessage(hwndLCD, WM_PAINT, 0, 0);
    return;
}
HANDLE hDebugFileHandle = 0;
void init_printf()
{
    hDebugFileHandle = CreateFile(WMVMEOutputFile, GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
}
void Fwl_GetCurrentDirectory(char *CurProjPath)
{
	GetCurrentDirectory(260,CurProjPath);
}
#ifdef DEBUG_SAVE_SBCDATA
T_HANDLE hSaveSBCfile = 0;
T_U32 totalSBClen = 0;
T_U32 totalPCMlen = 0;
T_U32 Saveindex = 0;
T_HANDLE hSavePCMfile = 0;
void Debug_InitSaveSBC()
{							
	char OutputFile[20];
	sprintf(OutputFile,"SBCData%d.sbc",Saveindex);
	if(AK_NULL == hSaveSBCfile)
	{
		hSaveSBCfile = CreateFile(OutputFile, GENERIC_READ|GENERIC_WRITE , \
							FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);		
	}
}
void Debug_DeInitSaveSBC()
{							
	if(AK_NULL != hSaveSBCfile)
	{
		CloseHandle(hSaveSBCfile);		
	}
	hSaveSBCfile = AK_NULL;
}

void Debug_SaveSBC(T_U8* data,T_U16 len)
{
	T_U32 rlen;
	if(hSaveSBCfile)
	{
		totalSBClen += WriteFile(hSaveSBCfile,data, len, &rlen, NULL);
	}
}
void Debug_InitSavePCM()
{							
	char OutputFile[20];
	sprintf(OutputFile,"PCMData%d.pcm",Saveindex);
	if(AK_NULL == hSavePCMfile)
	{
		hSavePCMfile = CreateFile(OutputFile, GENERIC_READ|GENERIC_WRITE , \
							FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);		
	}
}

void Debug_DeInitSavePCM()
{							
	if(AK_NULL != hSavePCMfile)
	{
		CloseHandle(hSavePCMfile);		
	}
	hSavePCMfile = AK_NULL;
}

void Debug_SavePCM(T_U8* data,T_U16 len)
{
	T_U32 rlen;
	if(hSavePCMfile)
	{
		totalPCMlen += WriteFile(hSavePCMfile,data, len, &rlen, NULL);
	}
}

#endif

int printf(const char *fmt, ...)
{
    FILE *  pFile =NULL;
    va_list ap;
    int i, slen, lineIdx;
    int txtRepStart, txtRepEnd, txtSelEnd;
    int str2Pt = 0;
    static int  wasCr = 0; //should be static type.
    static char string[STRING_LEN+4096]; //margin for '\b'
    static char string2[STRING_LEN+4096]; //margin for '\n'->'\r\n'
	char string3[STRING_LEN+4096]; //margin for '\n'->'\r\n'
    static int  prevBSCnt = 0;
	//OVERLAPPED lpOverlapped;
	DWORD filesize = 0;

    //while (isUsed); //EB_Printf can be called multiplely  //KIW
    //pFile = fopen(WMVMEOutputFile,"a+");

    isUsed = TRUE;

    txtRepStart = SendMessage(hWnd_DebugWnd, WM_GETTEXTLENGTH, 0x0, 0x0);
    txtRepEnd = txtRepStart - 1;

    va_start(ap, fmt);
    _vsntprintf(string2, STRING_LEN-1, fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    _vsntprintf(string3, STRING_LEN-1, fmt, ap);
    va_end(ap);
	WriteFile(hDebugFileHandle,string3, strlen(string3), &filesize, NULL);
    string2[STRING_LEN - 1] = '\0';

    //for better look of BS(backspace) char.,
    //the BS in the end of the string will be processed next time.
    for (i = 0; i < prevBSCnt; i++) //process the previous BS char.
    {
        string[i]='\b';
    }
    string[prevBSCnt] = '\0';
    lstrcat(string, string2);
    string2[0] = '\0';

    slen = lstrlen(string);
    for (i = 0; i < slen; i++)
    {
        if (string[slen - i - 1] != '\b')
            break;
    }

    prevBSCnt = i; // These BSs will be processed next time.
    slen = slen - prevBSCnt;

    if (slen == 0)
    {
        isUsed = FALSE;
        if (pFile != NULL)
        {
            fclose(pFile);
        }
        return 0;
    }

    for (i = 0; i < slen; i++)
    {
        if ((string[i] == KEY_BACKSPACE))
        {
            /*
            string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
            string2[str2Pt++] = ' ';txtRepEnd++;
            string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
            wasCr = 0;
            continue;
            */
            if (str2Pt > 0)
            {
                string2[str2Pt--] = KEY_BACKSPACE;txtRepEnd--;
                //string2[str2Pt++] = ' ';txtRepEnd++;
                //string2[str2Pt++] = KEY_BACKSPACE;txtRepEnd++;
                wasCr = 0;
                continue;
            }
        }

        if ((string[i] == '\n'))
        {
            string2[str2Pt++] = '\r';
            txtRepEnd++;
            string2[str2Pt++] = '\n';
            txtRepEnd++;
            wasCr = 0;
            continue;
        }
        if ((string[i] != '\n') && (wasCr == 1))
        {
            string2[str2Pt++] = '\r';
            txtRepEnd++;
            string2[str2Pt++] = '\n';
            txtRepEnd++;
            wasCr=0;
        }
        if (string[i] == '\r')
        {
            wasCr = 1;
            continue;
        }

        if (string[i] == '\b')
        {
            //flush string2
            if (str2Pt > 0)
            {
                string2[--str2Pt] = '\0';
                txtRepEnd--;
                continue;
            }
            //str2Pt==0;
            if(txtRepStart > 0)
            {
                txtRepStart--;
            }
            continue;
        }
        string2[str2Pt++] = string[i];
        txtRepEnd++;
        // if (str2Pt > 256-3)  break; //why needed? 2001.1.26
    }

    string2[str2Pt] = '\0';
    if (str2Pt > 0)
    {
        SendMessage(hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
        SendMessage(hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)string2);
        if (pFile != NULL)
        {
            fprintf(pFile, (const char *)string2);
            fprintf(pFile, "\r");
        }
    }
    else
    {
        if (txtRepStart <= txtRepEnd)
        {
            SendMessage(hWnd_DebugWnd,EM_SETSEL,txtRepStart,txtRepEnd+1);
            SendMessage(hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
        }
    }

    //If edit buffer is over EDIT_BUF_SIZE,
    //the size of buffer must be decreased by EDIT_BUF_DEC_SIZE.
    if (txtRepEnd > EDIT_BUF_SIZE)
    {
        lineIdx=SendMessage(hWnd_DebugWnd,EM_LINEFROMCHAR,EDIT_BUF_DEC_SIZE,0x0);
        //lineIdx=SendMessage(hWnd_DebugWnd,EM_LINEFROMCHAR,txtRepEnd-txtRepStart+1,0x0); //for debug
        txtSelEnd=SendMessage(hWnd_DebugWnd,EM_LINEINDEX,lineIdx,0x0)-1;
        SendMessage(hWnd_DebugWnd,EM_SETSEL,0,txtSelEnd+1);

        SendMessage(hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
        //SendMessage(hWnd_DebugWnd,WM_CLEAR,0,0); //WM_CLEAR doesn't work? Why?
        //SendMessage(hWnd_DebugWnd,WM_CUT,0,0); //WM_CUT doesn't work? Why?

        //make the end of the text shown.
        txtRepEnd=SendMessage(hWnd_DebugWnd,WM_GETTEXTLENGTH,0x0,0x0)-1;
        SendMessage(hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2);
        SendMessage(hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)" ");
        SendMessage(hWnd_DebugWnd,EM_SETSEL,txtRepEnd+1,txtRepEnd+2);
        SendMessage(hWnd_DebugWnd,EM_REPLACESEL,0,(LPARAM)"");
    }

    isUsed = FALSE;
    if (pFile != NULL)
    {
        fclose(pFile);
    }
    return 0;
}

int putch(char ch)
{
    printf("%c", ch);

    return 0;
}

int puts(const char *s)
{
    printf("%s", s);

    return 0;
}






