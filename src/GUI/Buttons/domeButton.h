#ifndef __DOME_BUTTON_H
#define __DOME_BUTTON_H

#include <functional>
#include <string>

#include <nanogui/widget.h>

class DomeButton : public nanogui::Widget {
public:
	explicit DomeButton(nanogui::Widget *parent, const std::string &label,
	                      std::function<void()> callback = nullptr);

	nanogui::Vector2i preferred_size_impl(NVGcontext *ctx) const override;
	bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
	                        int modifiers) override;
	bool mouse_enter_event(const nanogui::Vector2i &p, bool enter) override;
	void draw(NVGcontext *ctx) override;

	void set_label(const std::string &label) { m_label = label; }
	void set_callback(std::function<void()> callback) { m_callback = std::move(callback); }
	void set_home_style(bool enabled) { m_homeStyle = enabled; }

private:
	std::string           m_label;
	std::function<void()> m_callback;
	bool                  m_hovered{false};
	bool                  m_pressed{false};
	bool                  m_homeStyle{false};
};

#endif
