#pragma once

#include "FpButton.h"
#include "FpButton3d.h"
#include "FpEncoder.h"
#include "FpTouchSurface.h"
#include "FpLed.h"

namespace otto::board::ui {

class Push2BtnCb : public fp::Button::CallbackIf
{
public:
    Push2BtnCb(fp::Led::ISetter& rLedSetter);
    void onPressStateChange(const fp::Button::PressState& data, const fp::Widget& w) override;
private:
    fp::Led::ISetter& m_rLedSetter;
};

class Push2Btn3dCb : public fp::Button3d::CallbackIf
{
public:
    Push2Btn3dCb(fp::Led::ISetter& rLedSetter);
    void onPressStateChange(const fp::Button3d::StateData& data, const fp::Widget& w) override;
    void onPositionEvents(const uint32_t& pressure, const fp::Widget& w) override;
private:
    fp::Led::ISetter& m_rLedSetter;
};

class Push2EncoderCb : public fp::Encoder::CallbackIf
{
public:
    void onIncrement(int32_t data, const fp::Widget& w) override;
    void onTouchStateChanged(fp::Encoder::TouchState touchState, const fp::Widget& w) override;
};

class Push2TouchCb : public fp::TouchSurface::CallbackIf
{
public:
    void onTouchStateChanged(fp::TouchSurface::TouchState touchState, const fp::Widget& w) override;
    void onPositionEvents(const fp::TouchSurface::PressData& data, const fp::Widget& w) override;
};

}// namespace otto::board::ui