#ifndef __DOME_CONFIRM_DIALOG_H
#define __DOME_CONFIRM_DIALOG_H

#include <functional>
#include <string>

#include <nanogui/window.h>

class DomeConfirmDialog : public nanogui::Window
{
public:
    explicit DomeConfirmDialog(nanogui::Widget *parent,
                               const std::string &headline,
                               const std::string &context,
                               const std::string &message,
                               const std::string &confirmLabel = "Confirm",
                               const std::string &cancelLabel = "Cancel",
                               std::function<void()> onConfirm = nullptr,
                               std::function<void()> onCancel = nullptr);

    void draw(NVGcontext *ctx) override;
};

#endif
