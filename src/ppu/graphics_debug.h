#include <cstdint>
#include <memory>

#include "bus.h"
#include "cpu.h"
#include "graphics_wrapper.h"
#include "ppu.h"

class GraphicsDebug {
public:
    explicit GraphicsDebug(GraphicsWrapper& gfx) : gfx_(gfx) {
        cpu_ = std::make_shared<CPU>();
        ppu_ = std::make_shared<PPU>();
        bus_ = std::make_shared<Bus>(cpu_.get(), ppu_.get());
        cpu_->Bus(bus_.get());  // Set bus pointer in CPU after bus is created
    }

    bool run_mode_ = false;
    uint8_t selected_palette_ = 0;
    static constexpr int kTargetFps = 60;
    static constexpr uint32_t kFrameDelay = 1000 / kTargetFps; // 16ms
    static constexpr float kPatternViewScale = 1.5f;

    std::shared_ptr<CPU> cpu_;
    std::shared_ptr<PPU> ppu_;
    std::shared_ptr<Bus> bus_;
    GraphicsWrapper& gfx_;

    void RenderDebugInfo() const;
    void RenderOAMInfo() const;
    void RenderFlagsView() const;
    void RenderRegisterView() const;
    static void RenderFpsCounter();
    void RenderDisassemblyView() const;
    void RenderPaletteView() const;
    void RenderPatternTableView() const;
};
