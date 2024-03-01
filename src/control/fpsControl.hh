#ifndef __FPS_CONTROLLER_H
#define __FPS_CONTROLLER_H


#include "controls.hh"
#include "renderer/camera.hh"
#include "window/event.hh"
#include "window/eventDispatcher.hh"

#include <memory>

namespace Fallment{


class FpsControls:public Controls{
public:
    FpsControls() = default;
    virtual ~FpsControls() = default;

    FpsControls(std::unique_ptr<Camera>&& camera);

public:
    bool onInit() override;
    bool onUpdate() override;
    void onDestory() override;
    void onEvent(const Event& e) override;

private:

    void onMousePosEvent(const Event& e);
    void onMouseScrollEvent(const Event& e);
    void onWindowSizeEvent(const Event& e);

    std::unique_ptr<Camera>  m_camera;
};


}
#endif