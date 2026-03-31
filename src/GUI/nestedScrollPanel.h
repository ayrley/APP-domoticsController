#ifndef __NESTED_SCROLL_PANEL_H
#define __NESTED_SCROLL_PANEL_H

#include <nanogui/vscrollpanel.h>

class NestedScrollPanel : public nanogui::VScrollPanel {
public:
	explicit NestedScrollPanel(nanogui::Widget *parent);

	bool scroll_event(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;
};

#endif