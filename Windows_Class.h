#pragma once
#pragma once
#include <Windows.h>
#include <iostream>
#include <thread>
#include "PaintingFunction.h"

//这是一款Windows API的窗口类封装
//特性：支持多态，支持动态回调函数，支持自定义回调函数，包含多种已有控件


extern HWND MainWin;
extern HINSTANCE hInst;
extern HDC mHDC;
extern WNDCLASS wc;
extern HFONT hFont3;

struct StructOfXWYH
{
    int x;
    int y;
    int w;
    int h;
};


//标准普通窗口
class BaseWin { 
protected:

    DWORD dwExStyle, dwStyle;
    D2D1::ColorF WinColor= D2D1::ColorF(D2D1::ColorF::White);
    UINT ResourceName=0;
    string ResourceType;
    int ProcMode = Internal;
    int PaintMode = DefaultPaint;

public:
    HWND mHwnd = NULL;
    LPCWSTR ClassName, WinName;
    StructOfXWYH rect;

    enum PaintModeEnum {
        DefaultPaint,
        ColorPaint,
        PicturePaint,
        CostomPaint,
    };

    enum ProcModeEnum {
        Internal,
        External,
    };


    struct CrtParam {
        BaseWin* mThis;
    }mCrtParam = { this };

    BaseWin(DWORD mdwExStyle, LPCWSTR mClassName, LPCWSTR mWinName, DWORD mdwStyle, StructOfXWYH mRect) {
        WinProc = &BaseWin::DefaultWinProc;
        dwExStyle = mdwExStyle;
        ClassName = mClassName;
        WinName = mWinName;
        dwStyle = mdwStyle;
        rect = mRect;
    }


    BaseWin(DWORD mdwExStyle, LPCWSTR mClassName, LPCWSTR mWinName, DWORD mdwStyle, StructOfXWYH mCdnt,LRESULT(BaseWin::* CostomProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)) {
        WinProc = CostomProc;
        dwExStyle = mdwExStyle;
        ClassName = mClassName;
        WinName = mWinName;
        dwStyle = mdwStyle;
        rect = mCdnt;
    }

    BaseWin(DWORD mdwExStyle, LPCWSTR mClassName, LPCWSTR mWinName, DWORD mdwStyle, StructOfXWYH mCdnt, LRESULT(* CostomProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)) {
        ProcMode = External;
        ExternalWinProc = CostomProc;
        dwExStyle = mdwExStyle;
        ClassName = mClassName;
        WinName = mWinName;
        dwStyle = mdwStyle;
        rect = mCdnt;
    }

    virtual ~BaseWin() = default;


    void SetProc(LRESULT(BaseWin::* CostomProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)) {
        ProcMode = Internal;
        WinProc = CostomProc;
    }

    void SetProc(LRESULT(* CostomProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)) {
        ProcMode = External;
        ExternalWinProc = CostomProc;
    }

    void RegWin() {
        wc.lpfnWndProc = StaticWinProc;
        wc.lpszClassName = ClassName;
        RegisterClass(&wc);
    }

    void CrtWin(HWND hWnd, int n) {
        mHwnd = CreateWindowEx(dwExStyle, ClassName, WinName, dwStyle, rect.x, rect.y, rect.w, rect.h, hWnd, (HMENU)(n++), hInst, &mCrtParam);
    }

    LRESULT(BaseWin::* WinProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)= nullptr;

    LRESULT(* ExternalWinProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam)= nullptr;

    static LRESULT CALLBACK StaticWinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        CrtParam* pThis = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            pThis = reinterpret_cast<CrtParam*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        }
        else {
            pThis = reinterpret_cast<CrtParam*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        if (pThis->mThis->ProcMode == External) {
            return pThis->mThis->ExternalWinProc(hwnd, msg, wParam, lParam);
        }
        else {
            return ((pThis->mThis)->*(pThis->mThis->WinProc))(hwnd, msg, wParam, lParam);
        }
    }

    virtual LRESULT CALLBACK DefaultWinProc(HWND hwnd, UINT msgID, WPARAM wParam, LPARAM lParam)
    {
        PAINTSTRUCT ps;
        switch (msgID) {
        case WM_PAINT:
            mHDC = BeginPaint(mHwnd, &ps);
            CreateThreatPaintWin();
            EndPaint(mHwnd, &ps);
            break;
        case WM_ERASEBKGND:
            return TRUE;
        default:
            return DefWindowProc(hwnd, msgID, wParam, lParam);
        }
        return 0;
    }

    void ShowWin() {
        ShowWindow(mHwnd, SW_SHOW);
    }

    void HideWin() {
        ShowWindow(mHwnd, SW_HIDE);
    }

    void (*CostomPaintWin)()=nullptr;

    void SetColorPaint(D2D1::ColorF mColor) {
        PaintMode = ColorPaint;
        WinColor = mColor;
    }

    void SetPicturePaint(UINT mResourceName, char* mResourceType) {
        PaintMode = PicturePaint;
        ResourceName = mResourceName;
        ResourceType = mResourceType;
    }
    
    void SetCostomPaint(void(*mCostomPaintWin)()) {
        PaintMode = CostomPaint;
        CostomPaintWin = mCostomPaintWin;
    }

    virtual void PaintWin() {
        if (PaintMode == DefaultPaint) {

        }
        if (PaintMode == ColorPaint) {
            DrawColor(mHwnd, rect.x, rect.y, rect.w, rect.h, WinColor);
        }
        if (PaintMode == PicturePaint) {
            DrawPic(mHwnd, rect.w, rect.h, ResourceName, ResourceType);
        }
        if (PaintMode == CostomPaint) {
            CostomPaintWin();
        }
    }

    virtual void CreateThreatPaintWin() {

        thread thr(&BaseWin::PaintWin, this);
        thr.detach();
    }

};

class TagWin :public BaseWin {
public:

    TagWin(DWORD mdwExStyle, LPCWSTR mClassName, LPCWSTR mWinName, DWORD mdwStyle, StructOfXWYH mCdnt) :BaseWin(mdwExStyle, mClassName, mWinName, mdwStyle, mCdnt) {
        WinProc = &TagWin::DefaultWinProc;
        dwExStyle = mdwExStyle;
        ClassName = mClassName;
        WinName = mWinName;
        dwStyle = mdwStyle;
        rect = mCdnt;
    }

    TagWin(DWORD mdwExStyle, LPCWSTR mClassName, LPCWSTR mWinName, DWORD mdwStyle, StructOfXWYH mCdnt, LPCWSTR mTitle,int mTextBegin,int mTextEnd) :BaseWin(mdwExStyle, mClassName, mWinName, mdwStyle, mCdnt) {
        WinProc = &TagWin::DefaultWinProc;
        dwExStyle = mdwExStyle;
        ClassName = mClassName;
        WinName = mWinName;
        dwStyle = mdwStyle;
        rect = mCdnt;
        Title = mTitle;
        TextBegin = mTextBegin;
        TextEnd = mTextEnd;
    }

    LRESULT(TagWin::* WinProc)(HWND hWnd, UINT msgID, WPARAM wParam, LPARAM lParam);

    void PaintWin(){
        HDC hdc = GetDC(this->mHwnd);
        if (PaintMode == DefaultPaint) {

        }
        if (PaintMode == ColorPaint) {
            DrawColor(mHwnd,rect.w, rect.h, WinColor);
        }
        if (PaintMode == PicturePaint) {
            DrawPic(mHwnd, rect.w, rect.h, ResourceName, ResourceType);
        }
        if (PaintMode == CostomPaint) {
            CostomPaintWin();
        }
        SelectObject(hdc, Font);
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 2, 6, Title, wcslen(Title));
    }

    void CreateThreatPaintWin() {
        thread thr(&TagWin::PaintWin, this);
        thr.detach();
    }

    void SetFont(HFONT mFont) {
        Font = mFont;
    }

    void SetTextLong(int mTextBegin,int mTextEnd) {
        TextBegin = mTextBegin;
        TextEnd = mTextEnd;
    }

private:
    HFONT Font = hFont3;
    LPCWSTR Title = L"";
    int TextBegin=0;
    int TextEnd = 0;
};

class ButtonWin : public BaseWin {
    void (*ClickEvent)() = nullptr;

    LRESULT CALLBACK DefaultWinProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        static int CursorIsIn = 0;
        PAINTSTRUCT ps;
        TRACKMOUSEEVENT tme;
        thread tt1;
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.dwHoverTime = HOVER_DEFAULT;
        tme.hwndTrack = hwnd;
        switch (Message)
        {
        case WM_MOUSEMOVE:
            TrackMouseEvent(&tme);
            if (CursorIsIn == 0) {
                InvalidateRect(hwnd, NULL, TRUE);
            }
            CursorIsIn = 1;
            break;
        case WM_MOUSELEAVE:
            CursorIsIn = 0;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case WM_PAINT:
            FreeMyResource(IDR_MP31, "MP3", "data//01//Click.mp3");
            if (CursorIsIn == 0) {
                mHDC = BeginPaint(hwnd, &ps);
                //
                EndPaint(hwnd, &ps);
            }
            else {
                mHDC = BeginPaint(hwnd, &ps);
                //
                EndPaint(hwnd, &ps);
            }
            break;
        case WM_LBUTTONDOWN:
            //鼠标点击
            break;
        default:
            return DefWindowProc(hwnd, Message, wParam, lParam);   //让系统处理消息，这条语句一定要加上
        }
        return 0;
    }
};