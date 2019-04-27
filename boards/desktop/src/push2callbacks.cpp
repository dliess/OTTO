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
    OKey key;
    switch(w.id)
    {
        case fp::Push2Topology::Button::eBtnB:
        {
            switch(w.coord.x)
            {
                case 0: key = OKey::arpeggiator; break;
                case 1: key = OKey::synth;       break;
                case 2: key = OKey::envelope;    break;
                case 3: key = OKey::voices;      break;
                case 4: key = OKey::fx1;         break;
                case 5: key = OKey::fx2;         break;
                case 6: key = OKey::sequencer;   break;
                case 7: key = OKey::play;        break;
                default:                         return;
            }
            if(fp::Button::Pressed == data)
            {
                m_rLedSetter.setLed(Push2::getLedOfButton(w), fp::Led::getRGB(fp::Led::Red));
            }
            else
            {
                m_rLedSetter.setLed(Push2::getLedOfButton(w), fp::Led::getRGB(fp::Led::Black));
            }
            break;
        }
        case fp::Push2Topology::Button::eBtnShift:
        {
            std::cout << "Setting key to Shift" << std::endl;
            key = OKey::shift;
            break;
        }
        default:
        {
            return;
        }
    }
    if(fp::Button::Pressed == data)
    {
        if(key == OKey::shift) std::cout << "Shift pressed" << std::endl;
        Application::current().ui_manager->keypress(key);
    }
    else
    {
        if(key == OKey::shift) std::cout << "Shift released" << std::endl;
        Application::current().ui_manager->keyrelease(key);
    }            
}

Push2Btn3dCb::Push2Btn3dCb(fp::Led::ISetter& rLedSetter) :
    m_rLedSetter(rLedSetter)
{}
void Push2Btn3dCb::onPressStateChange(const fp::Button3d::StateData& data, const fp::Widget& w) 
{
    const int note = w.coord.x + w.coord.y + 60;
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
