#include "pch.h"
#include "CustomButton.h"


#include <vector>
#include <ctime>
#include <cstdarg>
#include <functional>





std::vector<CustomButton *> CustomButton::buttons;

void CustomButton::ThrowGroupEvent(int nIDEvent) {
	if (group != nullptr)
		group->ThrowEvent(nIDEvent);
	else
		OnGroupEvent(nIDEvent);
}

void CustomButton::Mouse(UINT nFlags, CPoint &point) {
	for (auto &b : buttons)
		if (b->m_bEnabled)
			if (b->m_rect.TopLeft().x <= point.x && point.x < b->m_rect.BottomRight().x && b->m_rect.TopLeft().y <= point.y && point.y < b->m_rect.BottomRight().y)
				b->OnMouse(nFlags, true);
			else
				b->OnMouse(nFlags, false);
}

void CustomButton::Keyboard(UINT nChar, UINT nRepCnt, UINT nFlags) {
	for (auto &b : buttons)
		if (b->m_bEnabled)
			b->OnKeyboard(nChar, nRepCnt, nFlags);
}

void CustomButton::Click(UINT nFlags, CPoint &point) {
	for (auto &b : buttons)
		if (b->m_bEnabled)
			if (b->m_rect.TopLeft().x <= point.x && point.x < b->m_rect.BottomRight().x && b->m_rect.TopLeft().y <= point.y && point.y < b->m_rect.BottomRight().y)
				b->OnClicked(nFlags);
}

void CustomButton::Draw(CDC *pDC) {
	for (auto &b : buttons)
		if (b->m_bEnabled)
			b->OnDraw(pDC);
}

CustomButton::CustomButton(CRect &rect, Callback callback)
: m_rect(rect), m_callback(callback) {
	buttons.push_back(this);
}





ButtonGroup::ButtonGroup(int count, CustomButton **array) {
	for (int i = 0; i < count; i++) {
		group.push_back(array[i]);
		array[i]->group = this;
	}
}

void ButtonGroup::Enable() {
	for (auto &b : group)
		b->Enable();
}

void ButtonGroup::Disable() {
	for (auto &b : group)
		b->Disable();
}

void ButtonGroup::ThrowEvent(int nIDEvent) {
	for (auto &b : group)
		b->OnGroupEvent(nIDEvent);
}





void TextButton::OnDraw(CDC *pDC) {
	CFont font;
	font.CreatePointFont(m_nFontPoint, m_strFontName);
	CFont *oldfont = pDC->SelectObject(&font);
	pDC->SetTextColor(m_colorText);
	pDC->DrawText(m_strText, m_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SetTextColor(RGB(0, 0, 0));
	pDC->SelectObject(oldfont);
	font.DeleteObject();
}

TextButton::TextButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi)
: CustomButton(rect, callback), m_strText(text), m_strFontName(font), m_nFontPoint(rect.Height() * font_rate * 720 / dpi) {}

void TextButton::ChangeText(CString &text) {
	m_strText = text;
}

void TextButton::ChangeTextColor(COLORREF color) {
	m_colorText = color;
}





void AnimationMenuButton::OnMouse(UINT nFlags, bool isOn) {
	if (isOn)
		m_strShownText = _T("¢º ") + m_strText;
	else
		m_strShownText = m_strText;
}

void AnimationMenuButton::OnClicked(UINT nFlags) {
	if (m_buttonmode == REMAIN) {
		ThrowGroupEvent(0);
		m_bClicked = true;
	}
}

void AnimationMenuButton::OnDraw(CDC *pDC) {
	int dx = m_rect.TopLeft().x - m_ptAnimationStart.x;
	int dy = m_rect.TopLeft().y - m_ptAnimationStart.y;
	CFont font, *oldfont;
	font.CreatePointFont(m_nFontPoint, m_strFontName);
	oldfont = pDC->SelectObject(&font);
	if (m_buttonmode == CREATE) {
		double time = (double)(clock() - m_clockCreate) / CLOCKS_PER_SEC;
		double rate = time / m_dAnimationTime; rate = -rate * (rate - 2);
		CRect new_rect(
			(int)(m_ptAnimationStart.x + dx * rate),
			(int)(m_ptAnimationStart.y + dy * rate),
			(int)(m_ptAnimationStart.x + m_rect.Width() + dx * rate),
			(int)(m_ptAnimationStart.y + m_rect.Height() + dy * rate)
		);
		if (time < m_dAnimationTime)
			pDC->DrawText(m_strText, new_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		else {
			pDC->DrawText(m_strText, m_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			m_buttonmode = REMAIN;
		}
	}
	else if (m_buttonmode == REMAIN) {
		pDC->DrawText(m_strShownText, m_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	else if (m_buttonmode == DESTROY) {
		double time = (double)(clock() - m_clockDestroy) / CLOCKS_PER_SEC;
		double rate = time / m_dAnimationTime; rate = rate * rate;
		CRect new_rect(
			(int)(m_rect.TopLeft().x - dx * rate),
			(int)(m_rect.TopLeft().y - dy * rate),
			(int)(m_rect.BottomRight().x - dx * rate),
			(int)(m_rect.BottomRight().y - dy * rate)
		);
		if (time < m_dAnimationTime)
			pDC->DrawText(m_strText, new_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		else {
			Disable();
			if (m_bClicked) {
				m_callback();
				m_bClicked = false;
			}
		}
	}
	oldfont = pDC->SelectObject(&font);
}

void AnimationMenuButton::OnGroupEvent(int nIDEvent) {
	if (nIDEvent == 0) {
		m_clockDestroy = clock();
		m_buttonmode = DESTROY;
	}
}

AnimationMenuButton::AnimationMenuButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, CPoint &pt, double sec)
: TextButton(rect, callback, text, font, font_rate, dpi), m_ptAnimationStart(pt), m_dAnimationTime(sec) {
	m_strShownText = m_strText;
}

void AnimationMenuButton::Enable() {
	m_bEnabled = true;
	m_clockCreate = clock();
	m_buttonmode = CREATE;
}

void AnimationMenuButton::Disable() {
	m_bEnabled = false;
	m_buttonmode = READY;
}





const double EdgeButton::m_dPopTime = 0.2;

void EdgeButton::OnClicked(UINT nFlags) {
	m_clockClick = clock();
	m_callback();
}

void EdgeButton::OnDraw(CDC *pDC) {
	CPen *oldpen = pDC->SelectObject(&pen);
	CFont font, *oldfont;
	CPoint points[5];
	double time = (double)(clock() - m_clockClick) / CLOCKS_PER_SEC;
	if (time > m_dPopTime) {
		points[0] = m_rect.TopLeft();
		points[1] = CPoint(m_rect.BottomRight().x, m_rect.TopLeft().y);
		points[2] = m_rect.BottomRight();
		points[3] = CPoint(m_rect.TopLeft().x, m_rect.BottomRight().y);
		font.CreatePointFont(m_nFontPoint, m_strFontName);
	}
	else {
		double rate = time / m_dPopTime; rate = -4 * rate * (rate - 1);
		int dx = (int)((m_rect.Width() * m_dPopIntensity) * rate);
		int dy = (int)((m_rect.Height() * m_dPopIntensity) * rate);
		points[0] = CPoint(m_rect.TopLeft().x + dx, m_rect.TopLeft().y + dy);
		points[1] = CPoint(m_rect.BottomRight().x - dx, m_rect.TopLeft().y + dy);
		points[2] = CPoint(m_rect.BottomRight().x - dx, m_rect.BottomRight().y - dy);
		points[3] = CPoint(m_rect.TopLeft().x + dx, m_rect.BottomRight().y - dy);
		font.CreatePointFont(m_nFontPoint * (1 - m_dPopIntensity * rate), m_strFontName);
	}
	pDC->Polygon(points, 4);
	oldfont = pDC->SelectObject(&font);
	pDC->DrawText(m_strText, m_rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SelectObject(oldpen);
	font.DeleteObject();
}

EdgeButton::EdgeButton(CRect &rect, Callback callback, CString &text, CString &font, double font_rate, int dpi, int thickness, double pop)
: TextButton(rect, callback, text, font, font_rate, dpi), m_nThickness(thickness), m_dPopIntensity(pop) {
	pen.CreatePen(PS_SOLID, m_nThickness, RGB(0, 0, 0));
}

