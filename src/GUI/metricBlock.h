#ifndef __METRIC_BLOCK_H
#define __METRIC_BLOCK_H

#include <string>

namespace nanogui
{
class Label;
class ProgressBar;
class Widget;
} // namespace nanogui

nanogui::Label *createMetricBlock(nanogui::Widget *parent,
                                  const std::string &title,
                                  const std::string &initialValue,
                                  nanogui::ProgressBar **barOut);

#endif
