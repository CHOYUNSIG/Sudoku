#pragma once

#include "pch.h"

#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>

typedef std::function<void()> Callback; // 버튼의 콜백

class Button; // 버튼 가상클래스
struct ButtonGroup; // 버튼 그룹

class TextButton; // 텍스트 기반 버튼
class AnimationButton; // 애니메이션이 있는 버튼
class EdgeButton; // 가장자리가 있는 버튼





/*
*
*	버튼
*
* 설명:
*	버튼 기능을 함
*	Enable 되었을 시에 m_rect 영역의 클릭을 감지해 m_callback을 호출함
* 
* 사용:
*	상속하는 클래스는 다음을 구현해야 함
*
*	OnMouse : 마우스가 버튼 위에 있을 때
*	OnKeyboard : 버튼에 해당하는 단축키가 작동했을 때
*	OnClicked : 버튼이 클릭됐을 때
*	OnDraw : 버튼을 화면에 그릴 때
*	OnTimer : 프레임마다 검사할 항목 설정
*	OnSize : 윈도우 사이즈가 변경되었을 때
*	OnGroupEvent : 버튼 그룹에 이벤트가 발생했을 때
*	Enable : 버튼을 활성화할 때
*	Disable : 버튼을 비활성화 할 때
*
* 주의사항:
*	절대 생성한 버튼의 소멸자를 호출해서는 안 됨
*	생성한 버튼을 활성 상태를 조절해서 재사용하는 방식을 채택해야 함
*
*/

class Button {
	friend struct ButtonGroup;

private:
	static std::vector<Button *> buttons;

	virtual void OnMouse(UINT nFlags, bool isOn) {};
	virtual void OnKeyboard(UINT nChar, UINT nRepCnt, UINT nFlags) {};
	virtual void OnClicked(UINT nFlags) { m_callback(); };
	virtual void OnDraw(CDC *pDC) {};
	virtual void OnTimer(UINT_PTR nIDEvent) {};
	virtual void OnSize(UINT nType, int cx, int cy) {};
	virtual void OnGroupEvent(int nIDEvent) {};

protected:
	bool m_bEnabled = false;
	CRect m_rectClick;
	Callback m_callback;
	ButtonGroup *group = nullptr;

public:
	static void Mouse(UINT nFlags, CPoint &point);
	static void Keyboard(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void Click(UINT nFlags, CPoint &point);
	static void Draw(CDC *pDC);
	static void Timer(UINT_PTR nIDEvent);

	Button(CRect &rect, Callback callback);
	virtual void Enable() { m_bEnabled = true; };
	virtual void Disable() { m_bEnabled = false; };
};





/*
* 
*	버튼 그룹
* 
* 설명:
*	버튼의 그룹을 형성함
*	
* 주의사항:
*	버튼은 한 개의 그룹에만 속할 수 있음
* 
*/

struct ButtonGroup {
	std::vector<Button *> group;

	ButtonGroup(int count, Button** array);
	void Enable();
	void Disable();
	void ThrowEvent(int nIDEvent);
	bool IsDisabled();
	size_t Size();
};





class TextButton : public Button {
protected:
	CString m_strFontName;
	const int m_nFontPoint;
	CString m_strText;
	COLORREF m_colorText = RGB(0, 0, 0);

	void OnDraw(CDC *pDC);

public:
	TextButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi);
	void ChangeText(CString &text);
	void ChangeTextColor(COLORREF color);
};





/*
* 
*	애니메이션 버튼
* 
* 설명:
*	생성과 소멸 시 애니메이션이 있는 버튼
*	버튼의 옵션은 플래그 방식으로 관리
* 
*	옵션 0 : 마우스를 올릴 시 화살표 생성
*	옵션 1 : 버튼을 누르면 버튼 그룹 전체에 소멸 메시지 전달
* 
*/

class AnimationButton : public TextButton {
private:
	enum BUTTONMODE { READY, CREATE, REMAIN, DESTROY };
	BUTTONMODE m_buttonmode = READY;
	const CPoint m_ptAnimationStart;
	const CPoint m_ptAnimationEnd;
	const double m_dAnimationTime;
	clock_t m_clockAnimationInit = clock();
	bool m_bClicked = false;
	CString m_strShownText;
	const int m_nOption;

	void OnMouse(UINT nFlags, bool isOn);
	void OnClicked(UINT nFlags);
	void OnDraw(CDC *pDC);
	void OnTimer(UINT_PTR nIDEvent);
	void OnGroupEvent(int nIDEvent);

	CPoint GetAnimatedPoint(bool forward);
	bool Animating();

public:
	AnimationButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, CPoint &pt, double sec, int option);
	void Enable();
	void Disable();
};





class EdgeButton : public TextButton {
private:
	static const double m_dPopTime;
	clock_t m_clockClick = clock();
	CPen pen;

	void OnClicked(UINT nFlags);
	void OnDraw(CDC *pDC);

protected:
	const int m_nThickness;
	const double m_dPopIntensity;

public:
	EdgeButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, int Thickness, double pop);
};