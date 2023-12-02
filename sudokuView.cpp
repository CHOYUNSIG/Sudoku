
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

#include <algorithm>
#include <string>
#include <functional>
#include <direct.h>
#include <thread>

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
	ON_WM_KEYUP()
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

void CsudokuView::AfterCreateWindow()
{	
	OnSoundVolumeClicked(0);
	OnScreenSizeClicked(0);
	OnLanguageClicked(0);
	group_init->Enable();
}

// CsudokuView 그리기

void CsudokuView::OnDraw(CDC *pDC)
{
	if (m_bAfterCreateWindow) {
		m_bAfterCreateWindow = false;
		AfterCreateWindow();
	}

	CsudokuDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CDC memdc;
	memdc.CreateCompatibleDC(pDC);
	memdc.SetBkMode(TRANSPARENT);
	
	CRect ClientRect;
	GetClientRect(ClientRect);

	int width = ClientRect.Width();
	int height = ClientRect.Height();
	int dpi = GetDpiForWindow(GetSafeHwnd());
	
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC, width, height);
	memdc.SelectObject(bitmap);
	for (int i = 0; i < height; i++)
		memdc.FillSolidRect(CRect(0, i, width, i + 1), RGB(255 - i * 15 / height, 245, 240 + i * 15 / height));

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
	CFont font, *oldfont;
	CPen pen, *oldpen;
	CString string;

	if (m_mode == GAME) {
		// 게임판 뒷배경
		memdc.FillSolidRect(CRect(
			width / 20,
			height / 20,
			width / 20 + height * 9 / 10,
			height * 19 / 20
		), RGB(255, 255, 255));
		// 동일한 숫자의 셀 색칠
		if (m_ingame == ON) {
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
		{
			font.CreatePointFont(height * 108 / dpi, font_name);
			oldfont = memdc.SelectObject(&font);
			memdc.DrawText(_T(" Sudoku™"), CRect(0, height / 10, width, height * 3 / 10), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
		}
		// 랭킹판
		if (m_menu == START || m_menu == NEW_GAME) {
			// 랭킹판 - 등수
			CString rank[5] = { CString("1st"), CString("2nd"), CString("3rd"), CString("4th"), CString("5th") };
			size_t max_ranked = max(max(ranking[0]->size(), ranking[1]->size()), ranking[2]->size());
			font.CreatePointFont((int)(height * 32 / dpi), font_name);
			oldfont = memdc.SelectObject(&font);
			for (int i = 0; i < max_ranked; i++) {
				memdc.DrawText(rank[i], CRect(
					width / 2,
					height * (6 + i) / 12,
					width * 6 / 10,
					height * (7 + i) / 12
				), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			// 랭킹판 - 난이도 별 기록
			CString diff[3] = { CString("쉬움"), CString("보통"), CString("어려움") };
			for (int d = 0; d < 3; d++) {
				if (ranking[d]->empty())
					continue;
				font.CreatePointFont(height * 32 / dpi, font_name);
				oldfont = memdc.SelectObject(&font);
				memdc.DrawText(diff[d], CRect(
					width * (6 + d) / 10,
					height * 5 / 12,
					width * (7 + d) / 10,
					height * 6 / 12
				), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
				memdc.SelectObject(oldfont);
				font.DeleteObject();
				for (int i = 0; i < ranking[d]->size(); i++) {
					font.CreatePointFont(height * 24 / dpi, _T("consolas"));
					oldfont = memdc.SelectObject(&font);
					string.Format(_T("%.2fs"), ranking[d]->at(i));
					memdc.DrawText(string, CRect(
						width * (6 + d) / 10,
						height * (6 + i) / 12,
						width * (7 + d) / 10,
						height * (7 + i) / 12
					), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
					memdc.SelectObject(oldfont);
					font.DeleteObject();
				}
			}
		}
	}
	else if (m_mode == LOADING) {
		// 로딩
		font.CreatePointFont(height * 72 / dpi, font_name);
		oldfont = memdc.SelectObject(&font);
		string = CString("로딩중");
		for (int i = 0; i < (clock() - m_clockRequested) * 2 / CLOCKS_PER_SEC % 4; i++)
			string += CString(".");
		memdc.DrawText(string, CRect(0, 0, width, height), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		memdc.SelectObject(oldfont);
		font.DeleteObject();
	}
	else if (m_mode == GAME) {
		// 카운트다운
		if (m_ingame == READY) {
			font.CreatePointFont(height * 72 / dpi, font_name);
			oldfont = memdc.SelectObject(&font);
			memdc.DrawText(_T("준비!"), CRect(width / 20, height / 20, width / 20 + height * 9 / 10, height / 2), DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			font.CreatePointFont(height * 72 / dpi, _T("consolas"));
			oldfont = memdc.SelectObject(&font);
			string.Format(_T("%.2fs"), 3 - (double)(clock() - m_clockGenerated) / CLOCKS_PER_SEC);
			memdc.DrawText(string, CRect(width / 20, height / 2, width / 20 + height * 9 / 10, height * 19 / 20), DT_SINGLELINE | DT_CENTER | DT_TOP);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
		}
		// 게임판 테두리
		if (m_ingame == READY || m_ingame == PAUSE) {
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
		// 게임판
		if (m_ingame == ON || m_ingame == DONE) {
			// 게임판 - 메모 숫자
			font.CreatePointFont(height * 24 / dpi, _T("굴림"));
			oldfont = memdc.SelectObject(&font);
			for (int i = 0; i < 9; i++)
				for (int j = 0; j < 9; j++) {
					if (m_map->GetValue(i, j) == 0) {
						int memo = m_map->GetMemo(i, j);
						for (int k = 0; k < 9; k++)
							if (memo & (1 << k)) {
								string.Format(_T("%d"), k + 1);
								if (k + 1 == m_map->GetValue(m_nSelRow, m_nSelCol))
									memdc.SetTextColor(RGB(0, 0, 0));
								else
									memdc.SetTextColor(RGB(180, 180, 180));
								memdc.DrawText(string, CRect(
									width / 20 + height * j / 10 + height * (k % 3) / 30,
									height * (i * 2 + 1) / 20 + height * (k / 3) / 30,
									width / 20 + height * j / 10 + height * (k % 3 + 1) / 30,
									height * (i * 2 + 1) / 20 + height * (k / 3 + 1) / 30
								), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
							}
					}
				}
			memdc.SetTextColor(RGB(0, 0, 0));
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			// 게임판 - 가는 선
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
			// 게임판 - 굵은 선
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
			// 게임판 - 선택된 셀
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
		}
		// 경과 시간
		if (!m_bEditMode && (m_ingame == ON || m_ingame == PAUSE)) {
			double time = 0.0;
			if (m_ingame == ON)
				time = GetTime();
			else if (m_ingame == PAUSE)
				time = m_dPausedTime;
			font.CreatePointFont(height * 36 / dpi, _T("consolas"));
			oldfont = memdc.SelectObject(&font);
			string.Format(_T("%.2fs"), time);
			memdc.DrawText(string, CRect(
				0,
				height / 9,
				width * 21 / 40 + height * 9 / 20,
				height * 2 / 9
			), DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
		}
		// 게임 결과
		if (m_ingame == DONE) {
			// 게임 결과 - 난이도
			font.CreatePointFont(height * 36 / dpi, font_name);
			oldfont = memdc.SelectObject(&font);
			switch (m_diff) {
			case EASY:
				string = CString("쉬움");
				break;
			case MEDIUM:
				string = CString("보통");
				break;
			case HARD:
				string = CString("어려움");
				break;
			case USER:
				string = CString("사용자 정의");
				break;
			}
			memdc.DrawText(string, CRect(
				width * 21 / 40 + height * 9 / 20 - width,
				0,
				width * 21 / 40 + height * 9 / 20 + width,
				height / 2
			), DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
			memdc.SelectObject(oldfont);
			font.DeleteObject();
			// 게임 결과 - 완료 시간
			font.CreatePointFont(height * 72 / dpi, _T("consolas"));
			oldfont = memdc.SelectObject(&font);
			string.Format(_T("%.2fs"), m_dFinishedTime);
			memdc.DrawText(string, CRect(
				width * 21 / 40 + height * 9 / 20 - width,
				height / 2,
				width * 21 / 40 + height * 9 / 20 + width,
				height
			), DT_SINGLELINE | DT_CENTER | DT_TOP);
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
	
	// 화면 주사율 조정
	SetTimer(0, 500 / FPS, NULL);

	// DPI 조정
	TextButton::m_nDPI = GetDpiForWindow(GetSafeHwnd());
	
	// 폰트 로딩
	{
		char font_path[256] = { 0, };
		if (_getcwd(font_path, sizeof(font_path)) != nullptr) {
			strcat_s(font_path, "\\res\\MaruBuri-Regular.ttf");
			AddFontResource(CString(font_path));
			font_name = CString("마루 부리 중간");
		}
		else {
			font_name = CString("Arial");
		}
	}

	// 설정값 복원
	{
		CStdioFile preset;
		preset.Open(_T("settings.dat"), CFile::modeRead | CFile::typeText);
		char buffer[1024] = { 0, };
		preset.Read(buffer, sizeof(buffer));
		preset.Close();

		char *context = nullptr;
		char *line = strtok_s(buffer, ":", &context);
		int *pre_setting_value[3] = {
			&m_nSoundVolume,
			&m_nScreenRatio,
			&m_nLanguage
		};

		for (int i = 0; i < 3; i++) {
			line = strtok_s(NULL, "\n", &context);
			*pre_setting_value[i] = atoi(line);
			line = strtok_s(NULL, ":", &context);
		}
	}

	// 랭킹 복원
	{
		CStdioFile rank;
		rank.Open(_T("ranking.dat"), CFile::modeRead | CFile::typeText);
		char buffer[1024] = { 0, };
		rank.Read(buffer, sizeof(buffer));
		rank.Close();

		char *context = nullptr;
		char *line = strtok_s(buffer, ":", &context);

		for (int d = 0; d < 3; d++) {
			ranking[d] = new std::vector<double>;
			for (int i = 0; i < 5; i++) {
				line = strtok_s(NULL, "\n", &context);
				double sec = atof(line);
				if (sec > 0)
					ranking[d]->push_back(sec);
				line = strtok_s(NULL, ":", &context);
			}
		}
	}

	/*
	* 
	* 버튼 생성 단계
	* : 게임에서 사용할 버튼은 여기서 전부 선언합니다.
	* 
	*/
	
	// 메뉴
	{
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
			Button *button_init[4] = { nullptr, };
			Callback cb[4] = {
				[=]() { OnNewgameClicked(); },
				[=]() { OnContinueClicked(); },
				[=]() { OnSettingsClicked(); },
				[=]() { OnExitClicked(); }
			};
			CString string[4] = {
				CString("새 게임"),
				CString("이어하기"),
				CString("설정"),
				CString("종료")
			};
			for (int i = 0; i < 4; i++)
				button_init[i] = new AnimationButton(menu_rect[i][0][0], menu_rect[i][0][1], cb[i], string[i], font_name, 0.5, menu_sp[i][0], 0.3 + 0.05 * i, 0b11);
			group_init = new ButtonGroup(4, button_init);
		}

		// 메뉴 - NEWGAME
		{
			Button *button_newgame[5] = { nullptr, };
			Callback cb[5] = {
				[=]() { OnDifficultyClicked(EASY); },
				[=]() { OnDifficultyClicked(MEDIUM); },
				[=]() { OnDifficultyClicked(HARD); },
				[=]() { OnUserClicked(); },
				[=]() { OnBackNewGameClicked(); }
			};
			CString string[5] = {
				CString("쉬움"),
				CString("보통"),
				CString("어려움"),
				CString("사용자 정의"),
				CString("뒤로")
			};
			for (int i = 0; i < 5; i++)
				button_newgame[i] = new AnimationButton(menu_rect[i][0][0], menu_rect[i][0][1], cb[i], string[i], font_name, 0.5, menu_sp[i][0], 0.3 + 0.05 * i, 0b11);
			group_newgame = new ButtonGroup(5, button_newgame);
		}

		// 메뉴 - SETTINGS
		{
			Button *button_settings[13] = { nullptr, };

			// 메뉴 - SETTINGS - 항목
			{
				button_settings[0] = new AnimationButton(menu_rect[0][0][0], menu_rect[0][0][1], [=]() {}, CString("음량"), font_name, 0.5, menu_sp[0][0], 0.3, 0b00);
				button_settings[1] = new AnimationButton(menu_rect[1][0][0], menu_rect[1][0][1], [=]() {}, CString("화면 크기"), font_name, 0.5, menu_sp[1][0], 0.35, 0b00);
				button_settings[2] = new AnimationButton(menu_rect[2][0][0], menu_rect[2][0][1], [=]() {}, CString("언어"), font_name, 0.5, menu_sp[2][0], 0.4, 0b00);
			}

			// 메뉴 - SETTINGS - 뒤로
			{
				button_settings[3] = new AnimationButton(menu_rect[3][0][0], menu_rect[3][0][1], [=]() { OnBackSettingsClicked(); }, CString("뒤로"), font_name, 0.5, menu_sp[3][0], 0.45, 0b11);
			}

			// 메뉴 - SETTINGS - 설정값
			{
				button_settings[4] = new AnimationButton(menu_rect[0][1][0], menu_rect[0][1][1], []() {}, CString(""), font_name, 0.5, menu_sp[0][1], 0.3, 0b00);
				button_settings[5] = new AnimationButton(menu_rect[1][1][0], menu_rect[1][1][1], []() {}, CString(""), font_name, 0.5, menu_sp[1][1], 0.35, 0b00);
				button_settings[6] = new AnimationButton(menu_rect[2][1][0], menu_rect[2][1][1], []() {}, CString(""), font_name, 0.5, menu_sp[2][1], 0.4, 0b00);
			}

			// 메뉴 - SETTINGS - 설정값 조정
			{
				std::function<void(int)> cb[3] = {
					[=](int inc) { OnSoundVolumeClicked(inc); },
					[=](int inc) { OnScreenSizeClicked(inc); },
					[=](int inc) { OnLanguageClicked(inc); }
				};
				for (int i = 0; i < 3; i++) {
					button_settings[7 + i * 2] = new AnimationButton(menu_rect[i][1][0],
						[=](int width, int height) {
							CPoint rect = menu_rect[i][1][0](width, height);
							return CPoint(rect.x + height / 10, rect.y + height / 10);
						}, [=]() { cb[i](-1); }, CString("◀"), font_name, 0.5, menu_sp[i][1], 0.3 + 0.05 * i, 0b00);
					button_settings[8 + i * 2] = new AnimationButton(
						[=](int width, int height) {
							CPoint rect = menu_rect[i][1][1](width, height);
							return CPoint(rect.x - height / 10, rect.y - height / 10);
						}, menu_rect[i][1][1], [=]() { cb[i](1); }, CString("▶"), font_name, 0.5, menu_sp[i][1], 0.3 + 0.05 * i, 0b00);
				}
			}

			group_settings = new ButtonGroup(13, button_settings);
		}
	}

	// 게임
	{
		// 게임 - 숫자키
		{
			Button *button_number[9] = { nullptr, };
			for (int i = 0; i < 9; i++) {
				CString a;
				a.Format(_T("%d"), i + 1);
				Corner tl = [=](int width, int height) {
					return CPoint(
						width * 21 / 40 + height * 9 / 20 + height * (2 * (i % 3) - 3) / 18 + 3,
						height * (3 + i / 3) / 9 + 3
					);
					};
				Corner br = [=](int width, int height) {
					return CPoint(
						width * 21 / 40 + height * 9 / 20 + height * (2 * (i % 3) - 1) / 18 - 3,
						height * (4 + i / 3) / 9 - 3
					);
					};
				button_number[i] = new EdgeButton(tl, br, [=]() { OnNumberKeyClicked(i + 1); }, a, font_name, 0.5, 1, 0.1);
			}
			group_numberkey = new ButtonGroup(9, button_number);
		}

		// 게임 - 일시정지
		{
			Corner tl = [=](int width, int height) {
				return CPoint(
					width * 21 / 40 + height * 9 / 20 + height / 9,
					height / 9
				);
				};
			Corner br = [=](int width, int height) {
				return CPoint(
					width * 21 / 40 + height * 9 / 20 + height * 2 / 9,
					height * 2 / 9
				);
				};
			Button *button_pause = new EdgeButton(tl, br, [=]() { OnPauseClicked(); }, CString("∥"), CString("consolas"), 0.5, 3, 0.1);
			group_pause = new ButtonGroup(1, &button_pause);
		}

		// 게임 - 일시정지 시 메뉴
		{

			Button *button_onpause[3] = { nullptr, };
			Corner tl[3], br[3], sp[3];
			for (int i = 0; i < 3; i++) {
				tl[i] = [=](int width, int height) {
					return CPoint(
						width / 20 + height * 9 / 20 - width * 3 / 20,
						height * (i * 2 + 7) / 20
					);
					};
				br[i] = [=](int width, int height) {
					return CPoint(
						width / 20 + height * 9 / 20 + width * 3 / 20,
						height * (i * 2 + 9) / 20
					);
					};
				sp[i] = [=](int width, int height) {
					return CPoint(
						-width / 2,
						height * (i * 2 + 7) / 20
					);
					};
			}
			button_onpause[0] = new AnimationButton(tl[0], br[0], [=]() { OnKeepGoingClicked(); }, CString("계속하기"), font_name, 0.5, sp[0], 0.3, 0b11);
			button_onpause[1] = new AnimationButton(tl[1], br[1], [=]() { OnSaveClicked(); }, CString("저장"), font_name, 0.5, sp[1], 0.35, 0b01);
			button_onpause[2] = new AnimationButton(tl[2], br[2], [=]() { OnGiveUpClicked(); }, CString("나가기"), font_name, 0.5, sp[2], 0.4, 0b11);
			group_onpause = new ButtonGroup(3, button_onpause);
		}

		// 게임 - 하단 메뉴
		{
			Button *button_toolbar[3] = { nullptr, };
			Corner tl[3], br[3];
			for (int i = 0; i < 3; i++) {
				tl[i] = [=](int width, int height) {
					return CPoint(
						width * 21 / 40 + height * 9 / 20 + height * (i * 3 - 4) / 18,
						height * 7 / 9
					);
					};
				br[i] = [=](int width, int height) {
					return CPoint(
						width * 21 / 40 + height * 9 / 20 + height * (i * 3 - 2) / 18,
						height * 8 / 9
					);
					};
			}
			button_toolbar[0] = new EdgeButton(tl[0], br[0], [=]() { OnEraseClicked(); }, CString("지우기"), font_name, 0.3, 3, 0.1);
			button_toolbar[1] = new EdgeButton(tl[1], br[1], [=]() { OnMemoClicked(); }, CString("메모"), font_name, 0.3, 3, 0.1);
			button_toolbar[2] = new EdgeButton(tl[2], br[2], [=]() { OnHintClicked(); }, CString("힌트"), font_name, 0.3, 3, 0.1);
			group_toolbar = new ButtonGroup(3, button_toolbar);
		}

		// 게임 - 격자판
		{
			Button *button_sudoku[81] = { nullptr, };
			for (int i = 0; i < 9 * 9; i++) {
				Corner tl = [=](int width, int height) {
					return CPoint(
						width / 20 + height * (i % 9) / 10,
						height * (2 * (i / 9) + 1) / 20
					);
					};
				Corner br = [=](int width, int height) {
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
	}

	return 0;
}


void CsudokuView::OnNewgameClicked() {
	group_newgame->Enable();
	m_menu = NEW_GAME;
}

void CsudokuView::OnDifficultyClicked(DIFF diff) {
	m_diff = diff;
	m_bMemo = false;
	m_bEditMode = false;
	m_bHintUsed = false;
	m_dAdditionalTime = 0.0;
	m_clockRequested = clock();

	int blank = 0;
	switch (diff) {
	case EASY:
		blank = 20;
		break;
	case MEDIUM:
		blank = 40;
		break;
	case HARD:
		blank = 100;
		break;
	}
	
	std::thread([&]() {
		m_mutex.lock();
		m_map = new SudokuMap(blank);
		m_mutex.unlock();
	}).detach();
	while (m_mutex.try_lock()) m_mutex.unlock();
	m_mode = LOADING;
}

void CsudokuView::OnContinueClicked() {
	CFileDialog dlg(TRUE, NULL, NULL, 0, _T("Sudoku Save File (*.sdk)|*.sdk|"), this);
	if (dlg.DoModal() == IDOK) {
		CStdioFile savefile;
		savefile.Open(dlg.GetPathName(), CFile::modeRead | CFile::typeText);
		char buffer[2048] = { 0, };
		savefile.Read(buffer, sizeof(buffer));
		savefile.Close();

		char *context = nullptr;
		char *line = strtok_s(buffer, ":", &context);

		int map[9][9] = { 0, };
		int bit[9][9] = { 0, };
		for (int i = 0; i < 81; i++) {
			line = strtok_s(NULL, "\n", &context);
			map[i / 9][i % 9] = std::atoi(line);
			line = strtok_s(NULL, ":", &context);
		}
		for (int i = 0; i < 81; i++) {
			line = strtok_s(NULL, "\n", &context);
			bit[i / 9][i % 9] = std::atoi(line);
			line = strtok_s(NULL, ":", &context);
		}
		line = strtok_s(NULL, "\n", &context);
		m_diff = (DIFF)std::atoi(line);
		line = strtok_s(NULL, ":", &context);
		line = strtok_s(NULL, "\n", &context);
		m_dAdditionalTime = std::atof(line);
		line = strtok_s(NULL, ":", &context);
		line = strtok_s(NULL, "\n", &context);
		m_bHintUsed = (bool)std::atoi(line);
		line = strtok_s(NULL, ":", &context);
		
		m_bMemo = false;
		m_bEditMode = false;
		m_map = new SudokuMap(map, bit);
		m_mode = LOADING;
	}
	else
		group_init->Enable();
}

double CsudokuView::GetTime() const
{
	return (double)(clock() - m_clockStarted) / CLOCKS_PER_SEC + m_dAdditionalTime;
}

void CsudokuView::OnSettingsClicked() {
	
	m_menu = SETTINGS;
	group_settings->Enable();
}

void CsudokuView::OnUserClicked() {
	m_bMemo = false;
	m_bEditMode = true;
	m_bHintUsed = false;
	m_map = new SudokuMap;
	m_nSelRow = m_nSelCol = 4;
	m_diff = USER;
	m_ingame = ON;
	m_mode = GAME;
	group_sudoku->Enable();
	group_numberkey->Enable();
	group_pause->Enable();
	group_toolbar->Enable();
}

void CsudokuView::OnBackNewGameClicked() {
	group_init->Enable();
	m_menu = START;
}

void CsudokuView::OnSoundVolumeClicked(int inc)
{
	m_nSoundVolume = max(min(m_nSoundVolume + inc * 10, 100), 0);

	CString a;
	a.Format(_T("%d%%"), m_nSoundVolume);
	((TextButton *)(group_settings->group[4]))->ChangeText(a);

	COLORREF l = RGB(0, 0, 0), r = RGB(0, 0, 0);
	if (m_nSoundVolume == 0)
		l = RGB(200, 200, 200);
	if (m_nSoundVolume == 100)
		r = RGB(200, 200, 200);
	((TextButton *)(group_settings->group[7]))->ChangeTextColor(l);
	((TextButton *)(group_settings->group[8]))->ChangeTextColor(r);
}

void CsudokuView::OnScreenSizeClicked(int inc)
{
	m_nScreenRatio = max(min(m_nScreenRatio + inc, SCREEN_RATIO_COUNT - 1), 0);

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

	COLORREF l = RGB(0, 0, 0), r = RGB(0, 0, 0);
	if (m_nScreenRatio == 0)
		l = RGB(200, 200, 200);
	if (m_nScreenRatio == SCREEN_RATIO_COUNT - 1)
		r = RGB(200, 200, 200);
	((TextButton *)(group_settings->group[9]))->ChangeTextColor(l);
	((TextButton *)(group_settings->group[10]))->ChangeTextColor(r);
}

void CsudokuView::OnLanguageClicked(int inc)
{
	m_nLanguage = (m_nLanguage + LANG_COUNT + inc) % LANG_COUNT;
}

void CsudokuView::OnBackSettingsClicked() {
	CString sound, screen, lang;
	sound.Format(_T("sound: %d\n"), m_nSoundVolume);
	screen.Format(_T("screen: %d\n"), m_nScreenRatio);
	lang.Format(_T("lang: %d\n"), m_nLanguage);
	CString st = sound + screen + lang;
	char buffer[1024] = { 0, };
	for (int i = 0; i < st.GetLength(); i++)
		buffer[i] = (char)st[i];

	CStdioFile preset;
	preset.Open(_T("settings.dat"), CFile::modeWrite | CFile::typeText);
	preset.Write(buffer, sizeof(char) * st.GetLength());
	preset.Close();

	m_menu = START;
	group_init->Enable();
}

void CsudokuView::OnNumberKeyClicked(int num) {
	if (m_bMemo)
		m_map->ToggleMemo(num, m_nSelRow, m_nSelCol);
	else
		m_map->SetValue(num, m_nSelRow, m_nSelCol);
}

void CsudokuView::OnEraseClicked()
{
	m_map->SetValue(0, m_nSelRow, m_nSelCol);
}

void CsudokuView::OnMemoClicked()
{
	m_bMemo = !m_bMemo;
	if (m_bMemo)
		((TextButton *)(group_toolbar->group[1]))->ChangeTextColor(RGB(255, 0, 0));
	else
		((TextButton *)(group_toolbar->group[1]))->ChangeTextColor(RGB(0, 0, 0));
}

void CsudokuView::OnPauseClicked()
{
	if (m_ingame == ON) {
		m_dPausedTime = GetTime();
		m_clockPaused = clock();
		m_ingame = PAUSE;
		group_onpause->Enable();
		for (int i = 0; i < 81; i++)
			((TextButton *)group_sudoku->group[(size_t)i])->ChangeText(CString(""));
	}
}

void CsudokuView::OnKeepGoingClicked()
{
	m_dAdditionalTime -= (double)(clock() - m_clockPaused) / CLOCKS_PER_SEC;
	m_ingame = ON;
}

void CsudokuView::OnSaveClicked()
{
	CFileDialog dlg(FALSE, _T("sdk"), NULL, 0, _T("Sudoku Save File (*.sdk)|*.sdk|"), this);
	if (dlg.DoModal() == IDOK) {
		CString st, a;
		for (int i = 0; i < 81; i++) {
			int indel = 0;
			if ((m_bEditMode && m_map->GetValue(i / 9, i % 9) > 0) || !m_map->Editable(i / 9, i % 9))
				indel = 10;
			a.Format(_T("cell%d: %d\n"), i, m_map->GetValue(i / 9, i % 9) + indel);
			st += a;
		}
		for (int i = 0; i < 81; i++) {
			a.Format(_T("memo%d: %d\n"), i, m_map->GetMemo(i / 9, i % 9));
			st += a;
		}
		a.Format(_T("diff: %d\n"), m_diff);
		st += a;
		if (m_bEditMode) a.Format(_T("time: %.2f\n"), 0.0);
		else a.Format(_T("time: %.2f\n"), m_dPausedTime);
		st += a;
		if (m_bEditMode) a.Format(_T("hint: %d\n"), 0);
		else a.Format(_T("hint: %d\n"), (int)m_bHintUsed);
		st += a;

		char buffer[2048] = { 0, };
		for (int i = 0; i < st.GetLength(); i++)
			buffer[i] = (char)st[i];

		CStdioFile savefile;
		savefile.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		savefile.Write(buffer, sizeof(char) * st.GetLength());
		savefile.Close();
	}
}

void CsudokuView::OnHintClicked()
{
	if (m_ingame == ON && ((!m_bHintUsed && AfxMessageBox(_T("힌트 기능을 사용하면 기록을 세울 수 없습니다.\n힌트를 사용하시겠습니까?"), MB_OKCANCEL, MB_ICONASTERISK) == IDOK) || m_bHintUsed)) {
		m_bHintUsed = true;
		switch (m_map->Hint()) {
		case 1:
			AfxMessageBox(_T("현재 스도쿠 판에 오답이 있습니다."), MB_OK, MB_ICONSTOP);
			break;
		case 2:
			AfxMessageBox(_T("해답 도출 도중 모순이 발견되었습니다."), MB_OK, MB_ICONSTOP);
			break;
		case 3:
			AfxMessageBox(_T("유일한 정답을 발견할 수 없습니다."), MB_OK, MB_ICONSTOP);
			break;
		default:
			break;
		}
	}
}

void CsudokuView::OnGiveUpClicked()
{
	group_numberkey->Disable();
	group_pause->Disable();
	group_toolbar->Disable();
	group_sudoku->Disable();
	m_mode = INIT;
	m_menu = START;
	group_init->Enable();
	delete m_map;
	m_map = nullptr;
}

void CsudokuView::OnSudokuMapClicked(int i)
{
	m_nSelRow = i / 9;
	m_nSelCol = i % 9;
}

void CsudokuView::OnExitClicked()
{
	AfxGetApp()->m_pMainWnd->PostMessage(WM_CLOSE);
}

void CsudokuView::OnDoneClicked()
{
	group_sudoku->Disable();
	group_init->Enable();
	m_mode = INIT;
	m_menu = START;
	delete m_map;
	m_map = nullptr;
}


void CsudokuView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(0);

	char font_path[256] = { 0, };
	if (_getcwd(font_path, sizeof(font_path)) != nullptr) {
		strcat_s(font_path, "\\res\\MaruBuri-Regular.ttf");
		RemoveFontResource(CString(font_path));
	}
}


void CsudokuView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Button::Timer(nIDEvent);

	if (nIDEvent == 0) {
		if (m_mode == LOADING) {
			// 스도쿠 문제 로딩 완료
			if (m_mutex.try_lock()) {
				m_mutex.unlock();
				m_nSelRow = m_nSelCol = 4;
				m_clockGenerated = clock();
				m_ingame = READY;
				m_mode = GAME;
				group_numberkey->Enable();
				group_pause->Enable();
				group_toolbar->Enable();
			}
		}
		else if (m_mode == GAME) {
			if (m_ingame == READY) {
				// 준비 시간
				if (clock() - m_clockGenerated >= 3 * CLOCKS_PER_SEC) {
					m_ingame = ON;
					m_clockStarted = clock();
					group_sudoku->Enable();
				}
			}
			if (m_ingame == ON) {
				// Button과 SudokuMap 간의 데이터 교환
				for (int i = 0; i < 9; i++)
					for (int j = 0; j < 9; j++)
						if (m_map->GetValue(i, j) > 0) {
							CString a;
							a.Format(_T("%d"), m_map->GetValue(i, j));
							((TextButton *)group_sudoku->group[(size_t)i * 9 + j])->ChangeText(a);
							COLORREF color;
							if (m_map->Contradict(i, j))
								color = RGB(255, 0, 0);
							else if (m_map->Editable(i, j))
								color = RGB(100, 100, 255);
							else
								color = RGB(50, 50, 50);
							((TextButton *)group_sudoku->group[(size_t)i * 9 + j])->ChangeTextColor(color);
						}
						else
							((TextButton *)group_sudoku->group[(size_t)i * 9 + j])->ChangeText(CString(""));
				// 스도쿠 완료
				if (!m_bEditMode && m_map->Done()) {
					m_dFinishedTime = GetTime();
					group_numberkey->Disable();
					group_pause->Disable();
					group_toolbar->Disable();

					if (m_diff != USER && !m_bHintUsed) {
						ranking[m_diff]->push_back(m_dFinishedTime);
						std::sort(ranking[m_diff]->begin(), ranking[m_diff]->end());
						while (ranking[m_diff]->size() > 5)
							ranking[m_diff]->pop_back();

						CString st;
						CString diff[3] = { CString("easy"), CString("medium"), CString("hard") };
						for (int d = 0; d < 3; d++) {
							for (int i = 0; i < ranking[d]->size(); i++) {
								CString a, b;
								a.Format(_T("%d"), i + 1);
								b.Format(_T("%.2f\n"), ranking[d]->at(i));
								st += diff[d] + a + CString(": ") + b;
							}
							for (int i = (int)ranking[d]->size(); i < 5; i++) {
								CString a;
								a.Format(_T("%d"), i + 1);
								st += diff[d] + a + CString(": ") + CString("-1\n");
							}
						}

						char buffer[1024] = { 0, };
						for (int i = 0; i < st.GetLength(); i++)
							buffer[i] = (char)st[i];

						CStdioFile rank;
						rank.Open(_T("ranking.dat"), CFile::modeWrite | CFile::typeText);
						rank.Write(buffer, sizeof(char) * st.GetLength());
						rank.Close();
					}

					m_ingame = DONE;
					group_done->Enable();
				}
			}
		}

		Invalidate(TRUE);
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
		// 셀에 숫자 입력
		if (m_ingame == ON) {
			if ('1' <= nChar && nChar <= '9')
				OnNumberKeyClicked(nChar - '0');
			else if (VK_NUMPAD1 <= nChar && nChar <= VK_NUMPAD9)
				OnNumberKeyClicked(nChar - VK_NUMPAD0);
			else if (nChar == VK_DELETE || nChar == VK_BACK || nChar == '0' || nChar == VK_NUMPAD0)
				OnEraseClicked();
		}
		// 선택된 셀 이동
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
		// 메모 기능 켜기
		if (m_ingame == ON) {
			if (nChar == VK_CONTROL) {
				m_bMemo = true;
				((TextButton *)(group_toolbar->group[1]))->ChangeTextColor(RGB(255, 0, 0));
			}
		}
		// 일시정지
		if (nChar == VK_ESCAPE) {
			if (m_ingame == ON)
				OnPauseClicked();
			else if (m_ingame == PAUSE)
				Button::Click(0, CPoint(SCREEN_RATIO[m_nScreenRatio][0] / 20 + SCREEN_RATIO[m_nScreenRatio][1] * 9 / 20, SCREEN_RATIO[m_nScreenRatio][1] * 4 / 10));
		}
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CsudokuView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (m_mode == GAME) {
		// 메모 기능 끄기
		if (m_ingame == ON) {
			if (nChar == VK_CONTROL) {
				((TextButton *)(group_toolbar->group[1]))->ChangeTextColor(RGB(0, 0, 0));
				m_bMemo = false;
			}
		}
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
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
