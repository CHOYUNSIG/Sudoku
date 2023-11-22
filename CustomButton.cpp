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

void Button::Size(UINT nType, int cx, int cy)
{
	for (auto &b : buttons)
		b->OnSize(nType, cx, cy);
}

Button::Button(Corner tl, Corner br, Callback callback)
: m_topleft(tl), m_bottomright(br), m_callback(callback)
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

size_t ButtonGroup::Count()
{
	return group.size();
}





int TextButton::m_nDPI = 96;

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

void TextButton::OnSize(UINT nType, int cx, int cy)
{
	Button::OnSize(nType, cx, cy);
	m_nFontPoint = (int)(m_rectClick.Height() * m_dFontRate * 720 / m_nDPI);
}

TextButton::TextButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate)
: Button(tl, br, callback), m_strText(text), m_strFontName(font), m_dFontRate(font_rate) {}

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
	m_bOn = m_nOption & (1 << 0) && isOn;
}

void AnimationButton::OnClicked(UINT nFlags)
{
	if (m_buttonmode == REMAIN) {
		if (m_nOption & (1 << 1)) {
			m_bClicked = true;
			group->ThrowEvent(1);
		}
		else
			m_callback();
	}
}

void AnimationButton::OnDraw(CDC *pDC)
{
	CFont font, *oldfont;
	font.CreatePointFont(m_nFontPoint, m_strFontName);
	oldfont = pDC->SelectObject(&font);
	if (m_buttonmode == CREATE) {
		CPoint pt = GetAnimatedPoint(true);
		pDC->DrawText(m_strText, CRect(pt.x, pt.y, pt.x + m_rectClick.Width(), pt.y + m_rectClick.Height()), DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	else if (m_buttonmode == REMAIN) {
		CString a = m_strText;
		if (m_bOn)
			a = _T("¢º ") + a;
		pDC->DrawText(a, m_rectClick, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
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

void AnimationButton::OnSize(UINT nType, int cx, int cy)
{
	TextButton::OnSize(nType, cx, cy);
	m_ptAnimationStart = m_start(cx, cy);
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
	int dx = m_rectClick.TopLeft().x - m_ptAnimationStart.x;
	int dy = m_rectClick.TopLeft().y - m_ptAnimationStart.y;
	double time = (double)(clock() - m_clockAnimationInit) / CLOCKS_PER_SEC;
	if (!Animating()) {
		if (forward)
			return m_rectClick.TopLeft();
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
			(int)(m_rectClick.TopLeft().x - dx * rate),
			(int)(m_rectClick.TopLeft().y - dy * rate)
		);
	}
}

bool AnimationButton::Animating()
{
	return (clock() - m_clockAnimationInit) < m_dAnimationTime * CLOCKS_PER_SEC;
}

AnimationButton::AnimationButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate, Corner &start, double sec, int option)
: TextButton(tl, br, callback, text, font, font_rate), m_start(start), m_dAnimationTime(sec), m_nOption(option) {}

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
	CPen *oldpen = pDC->SelectObject(&m_penEdge);
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

EdgeButton::EdgeButton(Corner tl, Corner br, Callback callback, CString &text, CString &font, double font_rate, int thickness, double pop)
: TextButton(tl, br, callback, text, font, font_rate), m_nThickness(thickness), m_dPopIntensity(pop)
{
	m_penEdge.CreatePen(PS_SOLID, m_nThickness, RGB(0, 0, 0));
}

