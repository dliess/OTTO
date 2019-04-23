#include "board/ui/push2callbacks.h"
#include "Push2LedOfWidget.h"
#include "services/log_manager.hpp"
#include "services/ui_manager.hpp"
#include "core/ui/screen.hpp"

using namespace otto::board::ui;
using OKey = otto::core::ui::Key;


Push2BtnCb::Push2BtnCb(fp::Led::ISetter& rLedSetter) :
    m_rLedSetter(rLedSetter)
{}
void Push2BtnCb::onPressStateChange(const fp::Button::PressState& data, const fp::Widget& w) 
{
    if(fp::Push2Topology::Button::eBtnB != w.id)
    {
        return;
    }
    if(fp::Button::Pressed == data)
    {
        LOGI("btn pressed {}", (int)w.id);
        switch(w.coord.x)
        {
            case 0:
            {
                Application::current().ui_manager->keypress(OKey::arpeggiator);
                break;
            }
            case 1:
            {
                Application::current().ui_manager->keypress(OKey::synth);
                break;
            }
            case 2:
            {
                Application::current().ui_manager->keypress(OKey::envelope);
                break;
            }
            case 3:
            {
                Application::current().ui_manager->keypress(OKey::voices);
                break;
            }
            case 4:
            {
                Application::current().ui_manager->keypress(OKey::fx1);
                break;
            }
            case 5:
            {
                Application::current().ui_manager->keypress(OKey::fx2);
                break;
            }
            case 6:
            {
                Application::current().ui_manager->keypress(OKey::sequencer);
                break;
            }
            case 7:
            {
                Application::current().ui_manager->keypress(OKey::play);
                break;
            }
            default:
            {
                break;
            }
        }
        m_rLedSetter.setLed(Push2::getLedOfButton(w), fp::Led::getRGB(fp::Led::Red));
    }
    else
    {
        LOGI("btn released {}", (int)w.id);
        m_rLedSetter.setLed(Push2::getLedOfButton(w), fp::Led::getRGB(fp::Led::Black));
    }
}

Push2Btn3dCb::Push2Btn3dCb(fp::Led::ISetter& rLedSetter) :
    m_rLedSetter(rLedSetter)
{}
void Push2Btn3dCb::onPressStateChange(const fp::Button3d::StateData& data, const fp::Widget& w) 
{
    const int note = w.coord.x + w.coord.y + 100;
    if(fp::Button3d::Pressed == data.pressState)
    {
        LOGI("btn3d pressed (velocity) {} (id) {}", data.velocity ,(int)w.id);
        Application::current().audio_manager->send_midi_event(core::midi::NoteOnEvent{note});
        m_rLedSetter.setLed(Push2::getLedOfButton3d(w), fp::Led::getRGB(fp::Led::Blue));
    }
    else
    {
        LOGI("btn3d released (velocity) {} (id) {}", data.velocity, (int)w.id);
        Application::current().audio_manager->send_midi_event(core::midi::NoteOffEvent{note});
        m_rLedSetter.setLed(Push2::getLedOfButton3d(w), fp::Led::getRGB(fp::Led::Black));
    }
}
void Push2Btn3dCb::onPositionEvents(const uint32_t& pressure, const fp::Widget& w) 
{
    /*
    std::cout << "btn3d position event: (pressure) "<< pressure << " (id) " << (int)w.id <<  std::endl;
    m_rLedSetter.setLed(Push2::getLedOfButton3d(w), {pressure * 2, pressure * 2, pressure * 2});
    */
}

void Push2EncoderCb::onIncrement(int32_t data, const fp::Widget& w) 
{
    LOGI("onIncrement {}", data);
    switch(w.coord.x)
    {
        case 0:
        {
            Application::current().ui_manager->rotary({core::ui::Rotary::blue, data});
            break;
        }
        case 1:
        {
            Application::current().ui_manager->rotary({core::ui::Rotary::green, data});
            break;
        }
        case 2:
        {
            Application::current().ui_manager->rotary({core::ui::Rotary::yellow, data});
            break;
        }
        case 3:
        {
            Application::current().ui_manager->rotary({core::ui::Rotary::red, data});
            break;
        }
        default:
        {
            break;
        }
    }
}

void Push2EncoderCb::onTouchStateChanged(fp::Encoder::TouchState touchState, const fp::Widget& w) 
{
    LOGI("onTouchStateChanged {}", touchState);
}

void Push2TouchCb::onTouchStateChanged(fp::TouchSurface::TouchState touchState, const fp::Widget& w) 
{
    LOGI("onTouchStateChanged {}", touchState);
}

void Push2TouchCb::onPositionEvents(const fp::TouchSurface::PressData& data, const fp::Widget& w) 
{
    LOGI("onPositionEvents {} {} {}", data.pos.x, data.pos.y, data.pressure);
}
