
// sudokuView.h: CsudokuView 클래스의 인터페이스
//

#pragma once

#include "CustomButton.h"
#include "SudokuMap.h"

const int FPS = 60;
enum MODE {INIT, GAME};
enum MENU {START, NEW_GAME, CONTINUE, SETTINGS};
enum INGAME {READY, ON, PAUSE, SAVE, DONE};
enum DIFF {EASY, MEDIUM, HARD, USER};

class CsudokuView : public CView
{
protected: // serialization에서만 만들어집니다.
	CsudokuView() noexcept;
	DECLARE_DYNCREATE(CsudokuView)

// 특성입니다.
public:
	CsudokuDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현입니다.
public:
	virtual ~CsudokuView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	int width = 852, height = 480;
	char font_path[256] = { 0, };
	CString font_name;

	MODE m_mode = INIT;
	MENU m_menu = START;
	INGAME m_ingame = READY;

	SudokuMap *map = nullptr;
	int m_nSelRow;
	int m_nSelCol;
	clock_t m_clockGenerated;
	clock_t m_clockStarted;
	clock_t m_clockEnded;

	ButtonGroup *group_init;
	ButtonGroup *group_newgame;
	ButtonGroup *group_settings;
	ButtonGroup *group_numberkey;
	ButtonGroup *group_sudoku;
	ButtonGroup *group_done;

	void OnNewgameClicked();
	void OnContinueClicked();
	void OnSettingsClicked();
	void OnExitClicked();
	void OnNewGameStart(DIFF diff);
	void OnUserClicked();
	void OnBackNewGameClicked();
	void OnBackSettingsClicked();
	void OnNumberKeyClicked(int num);
	void OnSudokuMapClicked(int index);
	void OnDoneClicked();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // sudokuView.cpp의 디버그 버전
inline CsudokuDoc* CsudokuView::GetDocument() const
   { return reinterpret_cast<CsudokuDoc*>(m_pDocument); }
#endif

