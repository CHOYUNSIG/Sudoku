
// sudokuView.cpp: CsudokuView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "sudoku.h"
#endif

#include "MainFrm.h"
#include "sudokuDoc.h"
#include "sudokuView.h"
#include "SudokuMap.h"
#include "CustomButton.h"
#include <functional>
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CsudokuView

IMPLEMENT_DYNCREATE(CsudokuView, CView)

BEGIN_MESSAGE_MAP(CsudokuView, CView)
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
END_MESSAGE_MAP()




// CsudokuView 생성/소멸

CsudokuView::CsudokuView() noexcept
{
	// TODO: 여기에 생성 코드를 추가합니다.
}

CsudokuView::~CsudokuView()
{
}

BOOL CsudokuView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.
	return CView::PreCreateWindow(cs);
}

// CsudokuView 그리기

void CsudokuView::OnDraw(CDC *pDC)
{
	CsudokuDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CDC memdc;
	memdc.CreateCompatibleDC(pDC);
	memdc.SetBkMode(TRANSPARENT);
	
	CRect ClientRect;
	GetClientRect(ClientRect);
	width = ClientRect.Width();
	height = ClientRect.Height();
	
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, width, height);
	memdc.SelectObject(bitmap);
	memdc.FillSolidRect(ClientRect, RGB(255, 255, 255));


	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	CFont font, *oldfont;
	CPen pen, *oldpen;
	CString string;

	if (m_mode == GAME) {
		if (m_ingame == ON) {
			// 동일한 숫자의 셀 색칠
			if (m_map->GetValue(m_nSelRow, m_nSelCol) != 0)
				for (int i = 0; i < 9; i++)
					for (int j = 0; j < 9; j++)
						if (!(i == m_nSelRow && j == m_nSelCol) && m_map->GetValue(i, j) == m_map->GetValue(m_nSelRow, m_nSelCol))
							memdc.FillSolidRect(CRect(
								width / 20 + height * j / 10,
								height * (2 * i + 1) / 20,
								width / 20 + height * (j + 1) / 10,
								height * (2 * i + 3) / 20
							), RGB(220, 220, 220));
		}
	}

	// 버튼
	Button::Draw(&memdc); 

	if (m_mode == INIT) {
		// 제목
		font.CreatePointFont((int)(height * 1.5 * 72 / GetDpiForWindow(GetSafeHwnd())), font_name);
		oldfont = memdc.SelectObject(&font);
		memdc.DrawText(_T(" Sudoku™"), CRect(0, height / 10, width, height * 3 / 10), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		memdc.SelectObject(oldfont);
		font.DeleteObject();
		// 랭킹판
		if (m_menu == START) {
			
		}
	}
	else if (m_mode == LOADING) {
		// 로딩
		font.CreatePointFont((int)(height * 72 / GetDpiForWindow(GetSafeHwnd())), font_name);
		oldfont = memdc.SelectObject(&font);
		memdc.DrawText(_T("Loading..."), CRect(0, 0, width, height), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		memdc.SelectObject(oldfont);
		font.DeleteObject();
	}
	else if (m_mode == GAME) {
		if (m_ingame == READY) {
			// 카운트다운 글자
			font.CreatePointFont(height * 72 / GetDpiForWindow(GetSafeHwnd()), font_name);
			oldfont = memdc.SelectObject(&font);
			memdc.DrawText(_T("준비!"), CRect(width / 20, height / 20, width / 20 + height * 9 / 10, height / 2), DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			font.CreatePointFont(height * 72 / GetDpiForWindow(GetSafeHwnd()), _T("consolas"));
			oldfont = memdc.SelectObject(&font);
			string.Format(_T("%.2fs"), 3 - (double)(clock() - m_clockGenerated) / CLOCKS_PER_SEC);
			memdc.DrawText(string, CRect(width / 20, height / 2, width / 20 + height * 9 / 10, height * 19 / 20), DT_SINGLELINE | DT_CENTER | DT_TOP);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			// 게임판 테두리
			CPen pen, *oldpen;
			pen.CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
			oldpen = memdc.SelectObject(&pen);
			memdc.MoveTo(CPoint(width / 20, height / 20));
			memdc.LineTo(CPoint(width / 20 + height * 9 / 10, height / 20));
			memdc.MoveTo(CPoint(width / 20, height / 20));
			memdc.LineTo(CPoint(width / 20, height * 19 / 20));
			memdc.MoveTo(CPoint(width / 20, height * 19 / 20));
			memdc.LineTo(CPoint(width / 20 + height * 9 / 10, height * 19 / 20));
			memdc.MoveTo(CPoint(width / 20 + height * 9 / 10, height / 20));
			memdc.LineTo(CPoint(width / 20 + height * 9 / 10, height * 19 / 20));
			memdc.SelectObject(oldpen);
			pen.DeleteObject();
		}
		else if (m_ingame == ON || m_ingame == DONE) {
			// 게임판 가는 선
			pen.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			oldpen = memdc.SelectObject(&pen);
			for (int i = 0; i < 10; i++) {
				memdc.MoveTo(CPoint(width / 20, height * (2 * i + 1) / 20));
				memdc.LineTo(CPoint(width / 20 + height * 9 / 10, height * (2 * i + 1) / 20));
				memdc.MoveTo(CPoint(width / 20 + height * i / 10, height / 20));
				memdc.LineTo(CPoint(width / 20 + height * i / 10, height * 19 / 20));
			}
			memdc.SelectObject(oldpen);
			pen.DeleteObject();
			// 게임판 굵은 선
			pen.CreatePen(PS_SOLID, 3, RGB(0, 0, 0));
			oldpen = memdc.SelectObject(&pen);
			for (int i = 0; i < 4; i++) {
				memdc.MoveTo(CPoint(width / 20, height * (6 * i + 1) / 20));
				memdc.LineTo(CPoint(width / 20 + height * 9 / 10, height * (6 * i + 1) / 20));
				memdc.MoveTo(CPoint(width / 20 + height * i * 3 / 10, height / 20));
				memdc.LineTo(CPoint(width / 20 + height * i * 3 / 10, height * 19 / 20));
			}
			memdc.SelectObject(oldpen);
			pen.DeleteObject();
			// 게임판 선택된 셀
			pen.CreatePen(PS_SOLID, 5, RGB(255, 100, 100));
			oldpen = memdc.SelectObject(&pen);
			memdc.MoveTo(CPoint(width / 20 + height * m_nSelCol / 10, height * (2 * m_nSelRow + 1) / 20));
			memdc.LineTo(CPoint(width / 20 + height * m_nSelCol / 10, height * (2 * m_nSelRow + 3) / 20));
			memdc.MoveTo(CPoint(width / 20 + height * (m_nSelCol + 1) / 10, height * (2 * m_nSelRow + 1) / 20));
			memdc.LineTo(CPoint(width / 20 + height * (m_nSelCol + 1) / 10, height * (2 * m_nSelRow + 3) / 20));
			memdc.MoveTo(CPoint(width / 20 + height * m_nSelCol / 10, height * (2 * m_nSelRow + 1) / 20));
			memdc.LineTo(CPoint(width / 20 + height * (m_nSelCol + 1) / 10, height * (2 * m_nSelRow + 1) / 20));
			memdc.MoveTo(CPoint(width / 20 + height * m_nSelCol / 10, height * (2 * m_nSelRow + 3) / 20));
			memdc.LineTo(CPoint(width / 20 + height * (m_nSelCol + 1) / 10, height * (2 * m_nSelRow + 3) / 20));
			memdc.SelectObject(oldpen);
			pen.DeleteObject();
			// 경과 시간
			if (m_ingame == ON) {
				font.CreatePointFont(height * 36 / GetDpiForWindow(GetSafeHwnd()), _T("consolas"));
				oldfont = memdc.SelectObject(&font);
				string.Format(_T("%.2fs"), (double)(clock() - m_clockStarted) / CLOCKS_PER_SEC);
				memdc.DrawText(string, CRect(
					0,
					height / 9,
					width * 21 / 40 + height * 9 / 20 + height / 18,
					height * 2 / 9
				), DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
			}
			else {
				font.CreatePointFont(height * 72 / GetDpiForWindow(GetSafeHwnd()), _T("consolas"));
				oldfont = memdc.SelectObject(&font);
				string.Format(_T("%.2fs"), (double)(m_clockEnded - m_clockStarted) / CLOCKS_PER_SEC);
				memdc.DrawText(string, CRect(
					width * 21 / 40 + height * 9 / 20 - width,
					0,
					width * 21 / 40 + height * 9 / 20 + width,
					height
				), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
			memdc.SelectObject(oldfont);
			font.DeleteObject();
		}
	}

	pDC->BitBlt(0, 0, ClientRect.Width(), ClientRect.Height(), &memdc, 0, 0, SRCCOPY);
	memdc.DeleteDC();
}


// CsudokuView 인쇄

BOOL CsudokuView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CsudokuView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CsudokuView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CsudokuView 진단

#ifdef _DEBUG
void CsudokuView::AssertValid() const
{
	CView::AssertValid();
}

void CsudokuView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CsudokuDoc* CsudokuView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CsudokuDoc)));
	return (CsudokuDoc*)m_pDocument;
}
#endif //_DEBUG


// CsudokuView 메시지 처리기


int CsudokuView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	SetTimer(0, 500 / FPS, NULL);

	if (_getcwd(font_path, sizeof(font_path)) != nullptr) {
		strcat_s(font_path, "\\res\\MaruBuri-Regular.ttf");
		AddFontResource(CString(font_path));
	}

	TextButton::m_nDPI = GetDpiForWindow(GetSafeHwnd());
	font_name = CString("마루 부리 중간");

	/*
	* 
	* 버튼 생성 단계
	* : 게임에서 사용할 버튼은 여기서 전부 선언합니다.
	* 
	*/
	
	Corner menu_rect[5][2][2];
	Corner menu_sp[5][2];
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 2; j++) {
			menu_rect[i][j][0] = [=](int width, int height) { return CPoint(width * (1 + j * 5) / 10, height * 2 / 5 + height * i / 10); };
			menu_rect[i][j][1] = [=](int width, int height) { return CPoint(width * (4 + j * 5) / 10, height * 2 / 5 + height * (i + 1) / 10); };
			menu_sp[i][j] = [=](int width, int height) { return CPoint(width * (j * 15 - 4) / 10, height * 2 / 5 + height * i / 10); };
		}
	}

	// 메뉴 - INIT
	{
		Button *button_init[4];
		button_init[0] = new AnimationButton(menu_rect[0][0][0], menu_rect[0][0][1], [=]() { OnNewgameClicked(); }, CString("새 게임"), font_name, 0.5, menu_sp[0][0], 0.3, 0b11);
		button_init[1] = new AnimationButton(menu_rect[1][0][0], menu_rect[1][0][1], [=]() { OnContinueClicked(); }, CString("이어하기"), font_name, 0.5, menu_sp[1][0], 0.35, 0b11);
		button_init[2] = new AnimationButton(menu_rect[2][0][0], menu_rect[2][0][1], [=]() { OnSettingsClicked(); }, CString("설정"), font_name, 0.5, menu_sp[2][0], 0.4, 0b11);
		button_init[3] = new AnimationButton(menu_rect[3][0][0], menu_rect[3][0][1], [=]() { OnExitClicked(); }, CString("종료"), font_name, 0.5, menu_sp[3][0], 0.45, 0b11);
		group_init = new ButtonGroup(4, button_init);
	}

	// 메뉴 - NEWGAME
	{
		Button *button_newgame[5];
		button_newgame[0] = new AnimationButton(menu_rect[0][0][0], menu_rect[0][0][1], [=]() { OnNewGameStart(EASY); }, CString("쉬움"), font_name, 0.5, menu_sp[0][0], 0.3, 0b11);
		button_newgame[1] = new AnimationButton(menu_rect[1][0][0], menu_rect[1][0][1], [=]() { OnNewGameStart(MEDIUM); }, CString("보통"), font_name, 0.5, menu_sp[1][0], 0.35, 0b11);
		button_newgame[2] = new AnimationButton(menu_rect[2][0][0], menu_rect[2][0][1], [=]() { OnNewGameStart(HARD); }, CString("어려움"), font_name, 0.5, menu_sp[2][0], 0.4, 0b11);
		button_newgame[3] = new AnimationButton(menu_rect[3][0][0], menu_rect[3][0][1], [=]() { OnUserClicked(); }, CString("사용자 정의"), font_name, 0.5, menu_sp[3][0], 0.45, 0b11);
		button_newgame[4] = new AnimationButton(menu_rect[4][0][0], menu_rect[4][0][1], [=]() { OnBackNewGameClicked(); }, CString("뒤로"), font_name, 0.5, menu_sp[4][0], 0.5, 0b11);
		group_newgame = new ButtonGroup(5, button_newgame);
	}

	// 메뉴 - SETTINGS
	{
		Button *button_settings[13];
		button_settings[0] = new AnimationButton(menu_rect[0][0][0], menu_rect[0][0][1], [=]() {}, CString("음량"), font_name, 0.5, menu_sp[0][0], 0.3, 0b00);
		button_settings[1] = new AnimationButton(menu_rect[1][0][0], menu_rect[1][0][1], [=]() {}, CString("화면 크기"), font_name, 0.5, menu_sp[1][0], 0.35, 0b00);
		button_settings[2] = new AnimationButton(menu_rect[2][0][0], menu_rect[2][0][1], [=]() {}, CString("언어"), font_name, 0.5, menu_sp[2][0], 0.4, 0b00);
		button_settings[3] = new AnimationButton(menu_rect[3][0][0], menu_rect[3][0][1], [=]() { OnBackSettingsClicked(); }, CString("뒤로"), font_name, 0.5, menu_sp[3][0], 0.45, 0b11);

		button_settings[4] = new AnimationButton(menu_rect[0][1][0], menu_rect[0][1][1], [=]() {}, CString("슬라이더"), font_name, 0.5, menu_sp[0][1], 0.3, 0b00);
		button_settings[5] = new AnimationButton(menu_rect[1][1][0], menu_rect[1][1][1], [=]() {}, CString("좌우버튼"), font_name, 0.5, menu_sp[1][1], 0.35, 0b00);
		button_settings[6] = new AnimationButton(menu_rect[2][1][0], menu_rect[2][1][1], [=]() {}, CString("좌우버튼"), font_name, 0.5, menu_sp[2][1], 0.4, 0b00);

		std::function<void(bool)> cb[3] = {
			[=](bool inc) { OnSoundVolumeClicked(inc); },
			[=](bool inc) { OnScreenSizeClicked(inc); },
			[=](bool inc) { OnLanguageClicked(inc); }
		};

		for (int i = 0; i < 3; i++) {
			button_settings[7 + i * 2] = new AnimationButton(
				[=](int width, int height) { 
					CPoint rect = menu_rect[i][1][0](width, height);
					return CPoint(rect.x - height / 10, rect.y);
				},
				[=](int width, int height) {
					CPoint rect = menu_rect[i][1][0](width, height);
					return CPoint(rect.x, rect.y + height / 10);
				},
				[=]() { cb[i](false); }, CString("◀"), font_name, 0.5, menu_sp[i][1], 0.3, 0b00);
			button_settings[8 + i * 2] = new AnimationButton(
				[=](int width, int height) {
					CPoint rect = menu_rect[i][1][1](width, height);
					return CPoint(rect.x, rect.y - height / 10);
				},
				[=](int width, int height) {
					CPoint rect = menu_rect[i][1][1](width, height);
					return CPoint(rect.x + height / 10, rect.y);
				},
				[=]() { cb[i](true); }, CString("▶"), font_name, 0.5, menu_sp[i][1], 0.3, 0b00);
		}

		group_settings = new ButtonGroup(13, button_settings);
	}

	// 게임 - Number Key
	{
		Button *button_number[9];
		for (int i = 0; i < 9; i++) {
			CString a;
			a.Format(_T("%d"), i + 1);
			Corner tl = [=](int weight, int height) {
				return CPoint(
					width * 21 / 40 + height * 9 / 20 + height * (2 * (i % 3) - 3) / 18 + 3,
					height * (3 + i / 3) / 9 + 3
				);
				};
			Corner br = [=](int weight, int height) {
				return CPoint(
					width * 21 / 40 + height * 9 / 20 + height * (2 * (i % 3) - 1) / 18 - 3,
					height * (4 + i / 3) / 9 - 3
				);
				};
			button_number[i] = new EdgeButton(tl, br, [=]() { OnNumberKeyClicked(i + 1); }, a, font_name, 0.5, 3, 0.1);
		}
		group_numberkey = new ButtonGroup(9, button_number);
	}

	// 게임 - 격자판
	{
		Button *button_sudoku[81];
		for (int i = 0; i < 9 * 9; i++) {
			Corner tl = [=](int weight, int height) {
				return CPoint(
					width / 20 + height * (i % 9) / 10,
					height * (2 * (i / 9) + 1) / 20
				);
				};
			Corner br = [=](int weight, int height) {
				return CPoint(
					width / 20 + height * (i % 9 + 1) / 10,
					height * (2 * (i / 9) + 3) / 20
				);
				};
			button_sudoku[i] = new TextButton(tl, br, [=]() { OnSudokuMapClicked(i); }, CString(""), CString("굴림"), 0.7);
		}
		group_sudoku = new ButtonGroup(81, button_sudoku);
	}

	// 게임 - 완료
	{
		Corner tl = [](int width, int height) {
			return CPoint(
				width * 21 / 40 + height * 9 / 20 - height / 6,
				height * 7 / 9
			);
			};
		Corner br = [](int width, int height) {
			return CPoint(
				width * 21 / 40 + height * 9 / 20 + height / 6,
				height * 7 / 9 + height / 10
			);
			};
		Corner sp = [](int width, int height) {
			return CPoint(width * 3 / 2, height * 7 / 9);
			};
		Button *button_done = new AnimationButton(tl, br, [=]() { OnDoneClicked(); }, CString("완료"), font_name, 0.5, sp, 0.4, 0b11);
		group_done = new ButtonGroup(1, &button_done);
	}

	group_init->Enable();

	return 0;
}


void CsudokuView::OnNewgameClicked() {
	group_newgame->Enable();
	m_menu = NEW_GAME;
}

void CsudokuView::OnNewGameStart(DIFF diff) {
	m_mode = LOADING;
	CDC* pDC = GetDC();
	OnDraw(pDC);
	pDC->DeleteDC();

	if (diff == EASY) m_map = new SudokuMap(20);
	else if (diff == MEDIUM) m_map = new SudokuMap(40);
	else if (diff == HARD) m_map = new SudokuMap(100);

	m_ingame = READY;
	m_nSelRow = 4;
	m_nSelCol = 4;
	m_clockGenerated = clock();
	m_mode = GAME;
	group_numberkey->Enable();
}

void CsudokuView::OnContinueClicked() {
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Sudoku Save File (*.sdk)|*.sdk|"), this);
	m_menu = CONTINUE;
	if (dlg.DoModal() == IDOK) {
		
	}
	else {
		group_init->Enable();
		m_menu = START;
	}
}

void CsudokuView::OnSettingsClicked() {
	
	group_settings->Enable();
	m_menu = SETTINGS;
}

void CsudokuView::OnUserClicked() {

}

void CsudokuView::OnBackNewGameClicked() {
	group_init->Enable();
	m_menu = START;
}

void CsudokuView::OnSoundVolumeClicked(bool inc)
{
	if (inc)
		m_nSoundVolume = min(m_nSoundVolume + 10, 100);
	else
		m_nSoundVolume = max(m_nSoundVolume - 10, 0);

	CString a;
	a.Format(_T("%d%%"), m_nSoundVolume);
	((TextButton *)(group_settings->group[4]))->ChangeText(a);
}

void CsudokuView::OnScreenSizeClicked(bool inc)
{
	if (inc)
		m_nScreenRatio = (m_nScreenRatio + 1) % SCREEN_RATIO_COUNT;
	else
		m_nScreenRatio = (m_nScreenRatio + SCREEN_RATIO_COUNT - 1) % SCREEN_RATIO_COUNT;

	CRect fullrect, clientrect;
	AfxGetMainWnd()->GetWindowRect(&fullrect);
	GetWindowRect(&clientrect);
	AfxGetMainWnd()->MoveWindow(CRect(
		fullrect.TopLeft().x,
		fullrect.TopLeft().y,
		fullrect.TopLeft().x + SCREEN_RATIO[m_nScreenRatio][0] + fullrect.Width() - clientrect.Width(),
		fullrect.TopLeft().y + SCREEN_RATIO[m_nScreenRatio][1] + fullrect.Height() - clientrect.Height()
	));

	CString a;
	a.Format(_T("%d×%d"), SCREEN_RATIO[m_nScreenRatio][0], SCREEN_RATIO[m_nScreenRatio][1]);
	((TextButton *)(group_settings->group[5]))->ChangeText(a);
}

void CsudokuView::OnLanguageClicked(bool inc)
{
	if (inc)
		m_nLanguage = (m_nLanguage + 1) % LANG_COUNT;
	else
		m_nLanguage = (m_nLanguage + LANG_COUNT - 1) % LANG_COUNT;
}

void CsudokuView::OnBackSettingsClicked() {
	group_init->Enable();
	m_menu = START;
}

void CsudokuView::OnNumberKeyClicked(int num) {	
	m_map->SetValue(num, m_nSelRow, m_nSelCol);
}

void CsudokuView::OnSudokuMapClicked(int i) {
	m_nSelRow = i / 9;
	m_nSelCol = i % 9;
}

void CsudokuView::OnExitClicked() {
	AfxGetApp()->m_pMainWnd->PostMessage(WM_CLOSE);
}

void CsudokuView::OnDoneClicked() {
	group_sudoku->Disable();
	group_init->Enable();
	m_mode = INIT;
	m_menu = START;
}


void CsudokuView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(0);
	RemoveFontResource(CString(font_path));
}


void CsudokuView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Button::Timer(nIDEvent);

	if (nIDEvent == 0) {
		Invalidate(TRUE);

		if (m_mode == GAME) {
			if (m_ingame == READY) {
				// 준비시간
				if (clock() - m_clockGenerated >= 3 * CLOCKS_PER_SEC) {
					m_ingame = ON;
					m_clockStarted = clock();
					group_sudoku->Enable();
				}
			}
			if (m_ingame == ON) {
				// 숫자와 버튼 간의 데이터 교환
				for (int i = 0; i < 9; i++)
					for (int j = 0; j < 9; j++)
						if (m_map->GetValue(i, j) > 0) {
							CString a;
							a.Format(_T("%d"), m_map->GetValue(i, j));
							((TextButton *)group_sudoku->group[i * 9 + j])->ChangeText(a);
							COLORREF color;
							if (m_map->Contradict(i, j))
								color = RGB(255, 0, 0);
							else if (m_map->Editable(i, j))
								color = RGB(100, 100, 255);
							else
								color = RGB(50, 50, 50);
							((TextButton *)group_sudoku->group[i * 9 + j])->ChangeTextColor(color);
						}
						else
							((TextButton *)group_sudoku->group[i * 9 + j])->ChangeText(CString(""));
				// 완료 여부
				if (m_map->Done()) {
					group_numberkey->Disable();
					m_clockEnded = clock();
					m_ingame = DONE;
					group_done->Enable();
				}
			}
		}
	}

	CView::OnTimer(nIDEvent);
}

void CsudokuView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Button::Mouse(nFlags, point);

	CView::OnMouseMove(nFlags, point);
}


void CsudokuView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Button::Click(nFlags, point);

	CView::OnLButtonDown(nFlags, point);
}


void CsudokuView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Button::Keyboard(nChar, nRepCnt, nFlags);

	if (m_mode == GAME) {
		if (m_ingame == ON) {
			if ('1' <= nChar && nChar <= '9')
				OnNumberKeyClicked(nChar - '0');
			else if (nChar == VK_DELETE || nChar == VK_BACK || nChar == '0')
				OnNumberKeyClicked(0);
		}
		if (m_ingame == ON || m_ingame == DONE) {
			if (nChar == VK_UP)
				m_nSelRow = max(0, m_nSelRow - 1);
			else if (nChar == VK_DOWN)
				m_nSelRow = min(8, m_nSelRow + 1);
			else if (nChar == VK_LEFT)
				m_nSelCol = max(0, m_nSelCol - 1);
			else if (nChar == VK_RIGHT)
				m_nSelCol = min(8, m_nSelCol + 1);
		}
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CsudokuView::OnEraseBkgnd(CDC *pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	return FALSE;
}


void CsudokuView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	Button::Size(nType, cx, cy);
}
