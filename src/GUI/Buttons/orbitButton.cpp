#include "orbitButton.h"

OrbitButton::OrbitButton(nanogui::Widget *parent,
                         const std::string &label,
                         std::function<void()> callback) :
    DomeButton(parent, label, std::move(callback))
{
    set_fixed_size(nanogui::Vector2i(104, 104));
}
