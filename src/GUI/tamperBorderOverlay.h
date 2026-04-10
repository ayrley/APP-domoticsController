#ifndef __TAMPER_BORDER_OVERLAY_H
#define __TAMPER_BORDER_OVERLAY_H

#include <chrono>

#include <nanogui/widget.h>

class TamperBorderOverlay : public nanogui::Widget
{
public:
    TamperBorderOverlay(nanogui::Widget *parent, const bool *detectedFlag);

    void draw(NVGcontext *ctx) override;

private:
    const bool *m_detectedFlag;
    std::chrono::steady_clock::time_point m_startTime;
};

#endif