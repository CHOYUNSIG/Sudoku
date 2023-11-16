#pragma once

#include "pch.h"

#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>

typedef std::function<void()> Callback; // ��ư�� �ݹ�

class Button; // ��ư ����Ŭ����
struct ButtonGroup; // ��ư �׷�

class TextButton; // �ؽ�Ʈ ��� ��ư
class AnimationButton; // �ִϸ��̼��� �ִ� ��ư
class EdgeButton; // �����ڸ��� �ִ� ��ư





/*
*
*	��ư
*
* ����:
*	��ư ����� ��
*	Enable �Ǿ��� �ÿ� m_rect ������ Ŭ���� ������ m_callback�� ȣ����
* 
* ���:
*	����ϴ� Ŭ������ ������ �����ؾ� ��
*
*	OnMouse : ���콺�� ��ư ���� ���� ��
*	OnKeyboard : ��ư�� �ش��ϴ� ����Ű�� �۵����� ��
*	OnClicked : ��ư�� Ŭ������ ��
*	OnDraw : ��ư�� ȭ�鿡 �׸� ��
*	OnTimer : �����Ӹ��� �˻��� �׸� ����
*	OnSize : ������ ����� ����Ǿ��� ��
*	OnGroupEvent : ��ư �׷쿡 �̺�Ʈ�� �߻����� ��
*	Enable : ��ư�� Ȱ��ȭ�� ��
*	Disable : ��ư�� ��Ȱ��ȭ �� ��
*
* ���ǻ���:
*	���� ������ ��ư�� �Ҹ��ڸ� ȣ���ؼ��� �� ��
*	������ ��ư�� Ȱ�� ���¸� �����ؼ� �����ϴ� ����� ä���ؾ� ��
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
*	��ư �׷�
* 
* ����:
*	��ư�� �׷��� ������
*	
* ���ǻ���:
*	��ư�� �� ���� �׷쿡�� ���� �� ����
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
*	�ִϸ��̼� ��ư
* 
* ����:
*	������ �Ҹ� �� �ִϸ��̼��� �ִ� ��ư
*	��ư�� �ɼ��� �÷��� ������� ����
* 
*	�ɼ� 0 : ���콺�� �ø� �� ȭ��ǥ ����
*	�ɼ� 1 : ��ư�� ������ ��ư �׷� ��ü�� �Ҹ� �޽��� ����
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