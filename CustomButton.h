#pragma once

#include "pch.h"

#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>

class CustomButton;
class ButtonGroup;
typedef std::function<void()> Callback;





/*

커스텀 버튼

사용:
	상속하는 클래스는 다음을 구현해야 함

	OnMouse : 마우스가 버튼 위에 있을 때
	OnKeyboard : 버튼에 해당하는 단축키가 작동했을 때
	OnClicked : 버튼이 클릭됐을 때
	OnDraw : 버튼을 화면에 그릴 때
	OnGroupEvent : 버튼 그룹에 이벤트가 발생했을 때
	Enable : 버튼을 활성화할 때
	Disable : 버튼을 비활성화 할 때

주의사항:
	절대 생성한 버튼의 소멸자를 호출해서는 안 됨
	생성한 버튼을 활성 상태를 조절해서 재사용하는 방식을 채택해야 함

*/
class CustomButton {
	friend class ButtonGroup;

private:
	static std::vector<CustomButton *> buttons;

	virtual void OnMouse(UINT nFlags, bool isOn) {};
	virtual void OnKeyboard(UINT nChar, UINT nRepCnt, UINT nFlags) {};
	virtual void OnClicked(UINT nFlags) { m_callback(); };
	virtual void OnDraw(CDC *pDC) {};
	virtual void OnGroupEvent(int nIDEvent) {};

	ButtonGroup *group = nullptr;

protected:
	bool m_bEnabled = false;
	CRect m_rect;
	Callback m_callback;
	void ThrowGroupEvent(int nIdEvent);

public:
	static void Mouse(UINT nFlags, CPoint &point);
	static void Keyboard(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void Click(UINT nFlags, CPoint &point);
	static void Draw(CDC *pDC);

	CustomButton(CRect &rect, Callback callback);
	virtual void Enable() { m_bEnabled = true; };
	virtual void Disable() { m_bEnabled = false; };
};





class ButtonGroup {
private:
	std::vector<CustomButton *> group;

public:
	ButtonGroup(int count, CustomButton** array);
	void Enable();
	void Disable();
	void ThrowEvent(int nIDEvent);
};





class TextButton : public CustomButton {
protected:
	CString m_strFontName;
	const int m_nFontPoint;
	CString m_strText;
	COLORREF m_colorText;

	void OnDraw(CDC *pDC);

public:
	TextButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi);
	void ChangeText(CString &text);
	void ChangeTextColor(COLORREF color);
};





class AnimationMenuButton : public TextButton {
private:
	enum BUTTONMODE { READY, CREATE, REMAIN, DESTROY };
	BUTTONMODE m_buttonmode = READY;
	bool m_bClicked = false;
	const CPoint m_ptAnimationStart;
	const double m_dAnimationTime;
	clock_t m_clockCreate = clock();
	clock_t m_clockDestroy = clock();
	CString m_strShownText;

	void OnMouse(UINT nFlags, bool isOn);
	void OnClicked(UINT nFlags);
	void OnDraw(CDC *pDC);
	void OnGroupEvent(int nIDEvent);

public:
	AnimationMenuButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, CPoint &pt, double sec);
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

