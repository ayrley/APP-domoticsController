#ifndef __MANUAL_CONTROL_PAGE_H
#define __MANUAL_CONTROL_PAGE_H

#include <functional>
#include <vector>

#include "defaultPage.h"

namespace nanogui
{
class Label;
class TextBox;
class Screen;
class Widget;
} // namespace nanogui

class DomeButton;
class IO;

class ManualControlPage : public DefaultPage
{
public:
    explicit ManualControlPage(nanogui::Widget *parent, std::function<void()> onHome);

private:
    void buildIOList();
    void populateForm(IO *io);
    void refreshIOSelection();
    void clearForm();
    void triggerIO(bool state);
    void forceIO(bool state);
    void cycleToNextIO();
    void releaseForcedIO();

    IO *m_selectedIO{nullptr};
    nanogui::Screen *m_nanoScreen{nullptr};

    nanogui::Label *m_statusLabel{nullptr};
    nanogui::Widget *m_ioListPanel{nullptr};
    nanogui::Label *m_locationLabel{nullptr};
    nanogui::TextBox *m_nameBox{nullptr};
    nanogui::TextBox *m_durationBox{nullptr};
    nanogui::Widget *m_stateIndicator{nullptr};
    DomeButton *m_setButton{nullptr};
    DomeButton *m_clearButton{nullptr};
    DomeButton *m_releaseButton{nullptr};

    std::vector<IO *> m_outputIOs;
    std::vector<DomeButton *> m_ioButtons;
    int m_currentIOIndex{-1};
    
    IO *m_forcedIO{nullptr};
    bool m_forcedState{false};
};

#endif
