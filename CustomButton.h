#pragma once

#include "pch.h"

#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>

typedef std::function<void()> Callback; // ��ư�� �ݹ�
typedef std::function<CPoint(int, int)> Corner; // ��ư ������ ������ ��� �Լ�

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
*	1. ���� ������ ��ư�� �Ҹ��ڸ� ȣ���ؼ��� �� ��
*	������ ��ư�� Ȱ�� ���¸� �����ؼ� �����ϴ� ����� ä���ؾ� ��
*	2. �ݵ�� ��ư�� �����ϸ� WM_SIZE �޽����� �߻������� ��
*
*/

class Button {
	friend struct ButtonGroup;

private:
	static std::vector<Button *> buttons;

protected:
	const Corner m_topleft;
	const Corner m_bottomright;
	const Callback m_callback;

	virtual void OnMouse(UINT nFlags, bool isOn) {};
	virtual void OnKeyboard(UINT nChar, UINT nRepCnt, UINT nFlags) {};
	virtual void OnClicked(UINT nFlags) { m_callback(); };
	virtual void OnDraw(CDC *pDC) {};
	virtual void OnTimer(UINT_PTR nIDEvent) {};
	virtual void OnSize(UINT nType, int cx, int cy) { m_rectClick = CRect(m_topleft(cx, cy), m_bottomright(cx, cy)); };
	virtual void OnGroupEvent(int nIDEvent) {};

	bool m_bEnabled = false;
	CRect m_rectClick;
	ButtonGroup *group = nullptr;

public:
	static void Mouse(UINT nFlags, CPoint &point);
	static void Keyboard(UINT nChar, UINT nRepCnt, UINT nFlags);
	static void Click(UINT nFlags, CPoint &point);
	static void Draw(CDC *pDC);
	static void Timer(UINT_PTR nIDEvent);
	static void Size(UINT nType, int cx, int cy);

	Button(Corner tl, Corner br, Callback callback);
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
	size_t Count();
};





/*
* 
* �ؽ�Ʈ ��ư
* 
* ����:
*	����� �ؽ�Ʈ�� �ִ� ��ư
* 
* ���ǻ���:
*	�ݵ�� ��� ���� DPI ���� �������־�� ��
*	static int m_nDPI
* 
*/
class TextButton : public Button {
protected:
	const double m_dFontRate;
	const CString m_strFontName;
	
	int m_nFontPoint;
	CString m_strText;
	COLORREF m_colorText = RGB(0, 0, 0);

	void OnDraw(CDC *pDC);
	virtual void OnSize(UINT nType, int cx, int cy);

public:
	static int m_nDPI;

	TextButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate);
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

	const Corner m_start;
	const double m_dAnimationTime;
	const int m_nOption;
	
	BUTTONMODE m_buttonmode = READY;
	CPoint m_ptAnimationStart;
	clock_t m_clockAnimationInit = clock();
	bool m_bClicked = false;
	CString m_strShownText;

	void OnMouse(UINT nFlags, bool isOn);
	void OnClicked(UINT nFlags);
	void OnDraw(CDC *pDC);
	void OnTimer(UINT_PTR nIDEvent);
	void OnSize(UINT nType, int cx, int cy);
	void OnGroupEvent(int nIDEvent);

	CPoint GetAnimatedPoint(bool forward);
	bool Animating();

public:
	AnimationButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate, Corner &start, double sec, int option);
	void Enable();
	void Disable();
};





class EdgeButton : public TextButton {
private:
	static const double m_dPopTime;
	const int m_nThickness;
	const double m_dPopIntensity;

	clock_t m_clockClick = clock();
	CPen m_penEdge;

	void OnClicked(UINT nFlags);
	void OnDraw(CDC *pDC);

public:
	EdgeButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate, int Thickness, double pop);
};