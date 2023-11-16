#include "pch.h"
#include "CustomButton.h"


#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>





std::vector<Button *> Button::buttons;

void Button::Mouse(UINT nFlags, CPoint &point)
{
	for (auto &b : buttons)
		if (b->m_bEnabled)
			if (b->m_rectClick.TopLeft().x <= point.x && point.x < b->m_rectClick.BottomRight().x && b->m_rectClick.TopLeft().y <= point.y && point.y < b->m_rectClick.BottomRight().y)
				b->OnMouse(nFlags, true);
			else
				b->OnMouse(nFlags, false);
}

void Button::Keyboard(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	for (auto &b : buttons)
		if (b->m_bEnabled)
			b->OnKeyboard(nChar, nRepCnt, nFlags);
}

void Button::Click(UINT nFlags, CPoint &point)
{
	for (auto &b : buttons)
		if (b->m_bEnabled)
			if (b->m_rectClick.TopLeft().x <= point.x && point.x < b->m_rectClick.BottomRight().x && b->m_rectClick.TopLeft().y <= point.y && point.y < b->m_rectClick.BottomRight().y)
				b->OnClicked(nFlags);
}

void Button::Draw(CDC *pDC)
{
	for (auto &b : buttons)
		if (b->m_bEnabled)
			b->OnDraw(pDC);
}

void Button::Timer(UINT_PTR nIDEvent)
{
	for (auto &b : buttons)
		if (b->m_bEnabled)
			b->OnTimer(nIDEvent);
}

Button::Button(CRect &rect, Callback callback)
: m_rectClick(rect), m_callback(callback)
{
	Button *self[1] = { this };
	group = new ButtonGroup(1, self);
	buttons.push_back(this);
}





ButtonGroup::ButtonGroup(int count, Button **array)
{
	for (int i = 0; i < count; i++) {
		group.push_back(array[i]);
		delete array[i]->group;
		array[i]->group = this;
	}
}

void ButtonGroup::Enable()
{
	for (auto &b : group)
		b->Enable();
}

void ButtonGroup::Disable()
{
	for (auto &b : group)
		b->Disable();
}

void ButtonGroup::ThrowEvent(int nIDEvent)
{
	for (auto &b : group)
		b->OnGroupEvent(nIDEvent);
}

bool ButtonGroup::IsDisabled()
{
	for (auto &b : group)
		if (b->m_bEnabled)
			return false;
	return true;
}

size_t ButtonGroup::Size()
{
	return group.size();
}





void TextButton::OnDraw(CDC *pDC)
{
	CFont font;
	font.CreatePointFont(m_nFontPoint, m_strFontName);
	CFont *oldfont = pDC->SelectObject(&font);
	pDC->SetTextColor(m_colorText);
	pDC->DrawText(m_strText, m_rectClick, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SelectObject(oldfont);
	font.DeleteObject();
}

TextButton::TextButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi)
: Button(rect, callback), m_strText(text), m_strFontName(font), m_nFontPoint((int)(rect.Height() * font_rate * 720 / dpi)) {}

void TextButton::ChangeText(CString &text)
{
	m_strText = text;
}

void TextButton::ChangeTextColor(COLORREF color)
{
	m_colorText = color;
}





void AnimationButton::OnMouse(UINT nFlags, bool isOn)
{
	if (m_nOption & (1 << 0)) {
		if (isOn)
			m_strShownText = _T("¢º ") + m_strText;
		else
			m_strShownText = m_strText;
	}
}

void AnimationButton::OnClicked(UINT nFlags)
{
	if (m_buttonmode == REMAIN) {
		m_bClicked = true;
		if (m_nOption & (1 << 1))
			group->ThrowEvent(1);
		else
			m_callback();
	}
}

void AnimationButton::OnDraw(CDC *pDC)
{
	CFont font, *oldfont;
	font.CreatePointFont(m_nFontPoint, m_strFontName);
	oldfont = pDC->SelectObject(&font);
	int dx = m_ptAnimationEnd.x - m_ptAnimationStart.x;
	int dy = m_ptAnimationEnd.y - m_ptAnimationStart.y;
	double time = (double)(clock() - m_clockAnimationInit) / CLOCKS_PER_SEC;
	if (m_buttonmode == CREATE) {
		CPoint pt = GetAnimatedPoint(true);
		pDC->DrawText(m_strText, CRect(pt.x, pt.y, pt.x + m_rectClick.Width(), pt.y + m_rectClick.Height()), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	else if (m_buttonmode == REMAIN) {
		pDC->DrawText(m_strShownText, m_rectClick, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	else if (m_buttonmode == DESTROY) {
		CPoint pt = GetAnimatedPoint(false);
		pDC->DrawText(m_strText, CRect(pt.x, pt.y, pt.x + m_rectClick.Width(), pt.y + m_rectClick.Height()), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	oldfont = pDC->SelectObject(&font);
}

void AnimationButton::OnTimer(UINT_PTR nIDEvent)
{
	if (m_buttonmode == CREATE) {
		if (!Animating())
			m_buttonmode = REMAIN;
	}
	else if (m_buttonmode == DESTROY) {
		if (!Animating())
			Disable();
	}
}

void AnimationButton::OnGroupEvent(int nIDEvent)
{
	if (nIDEvent == 0) {
		if (m_bClicked && group->IsDisabled())
			m_callback();
	}
	else if (nIDEvent == 1) {
		m_clockAnimationInit = clock();
		m_buttonmode = DESTROY;
	}
}

CPoint AnimationButton::GetAnimatedPoint(bool forward)
{
	int dx = m_ptAnimationEnd.x - m_ptAnimationStart.x;
	int dy = m_ptAnimationEnd.y - m_ptAnimationStart.y;
	double time = (double)(clock() - m_clockAnimationInit) / CLOCKS_PER_SEC;
	if (!Animating()) {
		if (forward)
			return m_ptAnimationEnd;
		else
			return m_ptAnimationStart;
	}
	if (forward) {
		double rate = time / m_dAnimationTime;
		rate = -rate * (rate - 2);
		return CPoint(
			(int)(m_ptAnimationStart.x + dx * rate),
			(int)(m_ptAnimationStart.y + dy * rate)
		);
	}
	else {
		double rate = time / m_dAnimationTime;
		rate = rate * rate;
		return CPoint(
			(int)(m_ptAnimationEnd.x - dx * rate),
			(int)(m_ptAnimationEnd.y - dy * rate)
		);
	}
}

bool AnimationButton::Animating()
{
	return (clock() - m_clockAnimationInit) < m_dAnimationTime * CLOCKS_PER_SEC;
}

AnimationButton::AnimationButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, CPoint &pt, double sec, int option)
: TextButton(rect, callback, text, font, font_rate, dpi), m_ptAnimationStart(pt), m_ptAnimationEnd(rect.TopLeft()), m_dAnimationTime(sec), m_nOption(option) {
	m_strShownText = m_strText;
}

void AnimationButton::Enable()
{
	m_bClicked = false;
	m_bEnabled = true;
	m_clockAnimationInit = clock();
	m_buttonmode = CREATE;
}

void AnimationButton::Disable()
{
	m_bEnabled = false;
	m_buttonmode = READY;
	group->ThrowEvent(0);
}





const double EdgeButton::m_dPopTime = 0.2;

void EdgeButton::OnClicked(UINT nFlags)
{
	m_clockClick = clock();
	m_callback();
}

void EdgeButton::OnDraw(CDC *pDC)
{
	CPen *oldpen = pDC->SelectObject(&pen);
	CFont font, *oldfont;
	CPoint points[5];
	double time = (double)(clock() - m_clockClick) / CLOCKS_PER_SEC;
	if (time > m_dPopTime) {
		points[0] = m_rectClick.TopLeft();
		points[1] = CPoint(m_rectClick.BottomRight().x, m_rectClick.TopLeft().y);
		points[2] = m_rectClick.BottomRight();
		points[3] = CPoint(m_rectClick.TopLeft().x, m_rectClick.BottomRight().y);
		font.CreatePointFont(m_nFontPoint, m_strFontName);
	}
	else {
		double rate = time / m_dPopTime; rate = -4 * rate * (rate - 1);
		int dx = (int)((m_rectClick.Width() * m_dPopIntensity) * rate);
		int dy = (int)((m_rectClick.Height() * m_dPopIntensity) * rate);
		points[0] = CPoint(m_rectClick.TopLeft().x + dx, m_rectClick.TopLeft().y + dy);
		points[1] = CPoint(m_rectClick.BottomRight().x - dx, m_rectClick.TopLeft().y + dy);
		points[2] = CPoint(m_rectClick.BottomRight().x - dx, m_rectClick.BottomRight().y - dy);
		points[3] = CPoint(m_rectClick.TopLeft().x + dx, m_rectClick.BottomRight().y - dy);
		font.CreatePointFont((int)(m_nFontPoint * (1 - m_dPopIntensity * rate)), m_strFontName);
	}
	pDC->Polygon(points, 4);
	oldfont = pDC->SelectObject(&font);
	pDC->DrawText(m_strText, m_rectClick, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SelectObject(oldpen);
	font.DeleteObject();
}

EdgeButton::EdgeButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, int thickness, double pop)
: TextButton(rect, callback, text, font, font_rate, dpi), m_nThickness(thickness), m_dPopIntensity(pop)
{
	pen.CreatePen(PS_SOLID, m_nThickness, RGB(0, 0, 0));
}

