#include "board/ui/glfw_ui_manager.hpp"

#include <chrono>
#include <thread>

#include "core/ui/vector_graphics.hpp"

#include "services/log_manager.hpp"
#include "services/ui_manager.hpp"

#define NANOVG_GL3_IMPLEMENTATION
#define OTTO_NVG_CREATE nvgCreateGL3
#define OTTO_NVG_DELETE nvgDeleteGL3

#include "board/ui/keys.hpp"
#include "board/ui/push2callbacks.h"

// C APIs. Include last
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <nanovg_gl.h>

#include <stdlib.h> // calloc

//#define __FRONTPANEL_SIMULATION__
#ifdef __FRONTPANEL_SIMULATION__
#include "FpSimulation.h"
#include "Push2Topology.h"
#else
#include "Push2Device.h"
#include "UsbMidiInputPortListProvider.h"
#include "UsbMidiOutputPortListProvider.h"
#include "UsbMidiPortNotifier.h"
#endif
namespace otto::glfw {

  Window::Window(int width, int height, const std::string& name)
  {
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    _glfw_win = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!_glfw_win) {
      throw util::exception("Failed to create GLFW window {}", name);
    }
    glfwSetWindowUserPointer(_glfw_win, this);

#if false
    glfwSetKeyboardCallback(_glfw_win, [](GLFWwindow* window, int key, int scancode, int action,
                                          int mods, const char* str, int) {
      if (auto* win = get_for(window); win) {
        if (win->key_callback) {
          win->key_callback(board::ui::Action{action}, board::ui::Modifiers{mods},
                            board::ui::Key{key});
        }
        if (win->char_callback && strlen(str) == 1) {
          win->char_callback(str[0]);
        }
      }
    });
#else
    glfwSetKeyCallback(_glfw_win, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
      if (auto* win = get_for(window); win && win->key_callback) {
        win->key_callback(board::ui::Action{action}, board::ui::Modifiers{mods},
                          board::ui::Key{key});
      }
    });

    glfwSetCharCallback(_glfw_win, [](GLFWwindow* window, unsigned ch) {
      if (auto* win = get_for(window); win && win->char_callback) {
        win->char_callback((char) ch);
      }
    });
#endif

    make_current();
    gl3wInit();
  }

  Window::~Window() noexcept {}

  GLFWwindow* Window::unwrap()
  {
    return _glfw_win;
  }

  Window::operator GLFWwindow*()
  {
    return _glfw_win;
  }

  Window* Window::get_for(GLFWwindow* glfw_win)
  {
    return static_cast<Window*>(glfwGetWindowUserPointer(glfw_win));
  }

  void Window::make_current()
  {
    glfwMakeContextCurrent(_glfw_win);
  }

  void Window::swap_buffers()
  {
    glfwSwapBuffers(_glfw_win);
  }

  void Window::set_window_aspect_ration(int x, int y)
  {
    glfwSetWindowAspectRatio(_glfw_win, x, y);
  }

  void Window::set_window_size_limits(int min_x, int min_y, int max_x, int max_y)
  {
    glfwSetWindowSizeLimits(_glfw_win, min_x, min_y, max_x, max_y);
  }

  bool Window::should_close()
  {
    return glfwWindowShouldClose(_glfw_win);
  }

  vg::Point Window::cursor_pos()
  {
    double x, y;
    glfwGetCursorPos(_glfw_win, &x, &y);
    return vg::Point{(float)x, (float)y};
  }

  std::pair<int, int> Window::window_size()
  {
    int x, y;
    glfwGetWindowSize(_glfw_win, &x, &y);
    return {x, y};
  }

  std::pair<int, int> Window::framebuffer_size()
  {
    int x, y;
    glfwGetFramebufferSize(_glfw_win, &x, &y);
    return {x, y};
  }


  NVGWindow::NVGWindow(int width, int height, const std::string& name)
    : Window(width, height, name),
      _vg(OTTO_NVG_CREATE(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG)),
      _canvas(_vg, width, height)
  {}

  NVGWindow::~NVGWindow() noexcept
  {
    OTTO_NVG_DELETE(_vg);
  }

  vg::Canvas& NVGWindow::canvas()
  {
    return _canvas;
  }

  void NVGWindow::begin_frame()
  {
    make_current();
    auto [winWidth, winHeight] = window_size();
    auto [fbWidth, fbHeight] = framebuffer_size();

    _canvas.setSize(winWidth, winHeight);

    // Update and render
    glViewport(0, 0, fbWidth, fbHeight);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    _canvas.clearColor(vg::Colours::Black);
    _canvas.begineFrame(winWidth, winHeight);
  }

  void NVGWindow::end_frame()
  {
    _canvas.endFrame();
    glEnable(GL_DEPTH_TEST);
    swap_buffers();
  }

} // namespace otto::glfw

namespace otto::services {

  using namespace core::ui;

  static void error_callback(int error, const char* description)
  {
    LOG_F(ERROR, "GLFW UI: {}", description);
  }

  void GLFWUIManager::main_ui_loop()
  {
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      LOG_F(ERROR, "Failed to init GLFW.");
    }
    gsl::final_act terminate_glfw(glfwTerminate);
    gsl::final_act exit_application(
      [] { Application::current().exit(Application::ErrorCode::ui_closed); });

    glfw::NVGWindow main_win(vg::width, vg::height, "OTTO");

    main_win.set_window_aspect_ration(4, 3);
    main_win.set_window_size_limits(320, 240, GLFW_DONT_CARE, GLFW_DONT_CARE);

    main_win.key_callback = board::ui::handle_keyevent;

    vg::initUtils(main_win.canvas());

    glfwSetTime(0);

    double t, spent;

#ifdef __FRONTPANEL_SIMULATION__
  fp::Simulation<fp::Push2Topology> push2Device("127.0.0.1:50051");
#else
    midi::InputPortListProvider  inputPortListProvider;
    if(!inputPortListProvider.init())
    {
        return;
    }
    midi::OutputPortListProvider outputPortListProvider;
    if(!outputPortListProvider.init())
    {
        return;
    }
    midi::PortNotifier<midi::InputPortListProvider>   inputPortNotifier(inputPortListProvider);
    midi::PortNotifier<midi::OutputPortListProvider>  outputPortNotifier(outputPortListProvider); 

    Push2::Push2Device push2Device(inputPortNotifier, outputPortNotifier);
    //push2.registerCB();

    inputPortNotifier.update();
    outputPortNotifier.update();
    push2Device.init(1000);
#endif
    board::ui::Push2EncoderCb encoderCb;
    push2Device.registerCB(encoderCb, fp::Widget(fp::Push2Topology::Encoder::eEncoder, fp::IdxAll));

    push2Device.setLed(fp::Widget(fp::Push2Topology::Led::eLedT, 0), fp::Led::getRGB(fp::Led::Blue));
    push2Device.setLed(fp::Widget(fp::Push2Topology::Led::eLedT, 1), fp::Led::getRGB(fp::Led::Green));
    push2Device.setLed(fp::Widget(fp::Push2Topology::Led::eLedT, 2), {0xFB, 0xB8, 0x0B});
    push2Device.setLed(fp::Widget(fp::Push2Topology::Led::eLedT, 3), fp::Led::getRGB(fp::Led::Red));

    board::ui::Push2BtnCb buttonCb(push2Device);
    push2Device.registerCB(buttonCb, fp::Widget(fp::Push2Topology::Button::eBtnB, fp::IdxAll));

    char* buffer = static_cast<char*>(calloc(4, vg::width * vg::height));
    //char* flippedBuffer = static_cast<char*>(calloc(4, vg::width * vg::height));

    const auto displayId = fp::Push2Topology::Display::Id::eDisplay;
    auto pRenderMedium = push2Device.getRenderMedium(fp::Widget(displayId));
    pRenderMedium->setFrameBufRendering(false);
    while (!main_win.should_close() && Application::current().running()) {
      float scale;

      t = glfwGetTime();

#ifdef __FRONTPANEL_SIMULATION__
      push2Device.updateInputs();
#endif
      //glScalef(1,-1,1);

      auto [winWidth, winHeight] = main_win.window_size();
      // Calculate pixel ration for hi-dpi devices.
       
      main_win.begin_frame();
      scale =
        std::min((float) winWidth / (float) vg::width, ((float) winHeight) / (float) vg::height);
      //main_win.canvas().scale(scale, scale);
      //main_win.canvas().transform(1.0, 0.0, 0.0, 0.0, -1.0, vg::height);
      main_win.canvas().transform(0.8,   0.0, 0.0,
                                  -0.8,  0.0, vg::height * 1.06);
      draw_frame(main_win.canvas());
      main_win.end_frame();

      glfwPollEvents();
      flush_events();

      glReadPixels(0, 0, vg::width, vg::height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
      const fp::gfx::Rectangle rect( fp::gfx::Coord(100, 0),
                                     fp::gfx::Size2D(vg::width, vg::height) );
      pRenderMedium->streamToSubWindow(rect, reinterpret_cast<fp::ColorRGB*>(buffer));
      pRenderMedium->flushFrameBuffer();

      spent = glfwGetTime() - t;

      std::this_thread::sleep_for(std::chrono::milliseconds(int(1000 / 60 - spent * 1000)));
    }
    free(buffer);
  }
} // namespace otto::services

// kak: other_file=../include/board/ui/glfw_ui_manager.hpp
