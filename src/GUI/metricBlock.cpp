#include "metricBlock.h"

#include "heatProgressBar.h"

#include <nanogui/nanogui.h>

nanogui::Label *createMetricBlock(nanogui::Widget *parent,
                                  const std::string &title,
                                  const std::string &initialValue,
                                  nanogui::ProgressBar **barOut)
{
    nanogui::Widget *panel = new nanogui::Widget(parent);
    panel->set_layout(new nanogui::GroupLayout());

    nanogui::Label *titleLabel = new nanogui::Label(panel, title, "sans-bold");
    titleLabel->set_font_size(18);

    HeatProgressBar *bar = new HeatProgressBar(panel);
    bar->set_fixed_size(nanogui::Vector2i(380, 16));
    bar->set_value(0.0f);

    nanogui::Label *valueLabel = new nanogui::Label(panel, initialValue, "sans-bold");
    valueLabel->set_font_size(24);

    if (barOut != nullptr) {
        *barOut = bar;
    }

    return valueLabel;
}
