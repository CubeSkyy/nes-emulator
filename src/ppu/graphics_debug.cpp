#define SDL_MAIN_HANDLED
#include <iostream>
#include <memory>
#include <SDL.h>

#include "imgui.h"
#include "graphics_debug.h"
#include "graphics_wrapper.h"
#include "ppu.h"
#include "log/logging.h"


void GraphicsDebug::RenderDebugInfo() const {
    ImGui::SetNextWindowSize(ImVec2(420, 1000), ImGuiCond_Once);
    ImGui::Begin("Debug");

    RenderFlagsView();
    RenderRegisterView();
    ImGui::Separator();
    RenderOAMInfo();
    ImGui::Separator();
    RenderPaletteView();
    RenderPatternTableView();
    RenderDisassemblyView();
    ImGui::End();
}


void GraphicsDebug::RenderOAMInfo() const {
    if (!ImGui::CollapsingHeader("OAM (Sprite Memory)"))
        return;

    // Table header with optimized column widths
    ImGui::BeginTable("OAM", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("00").x);
    ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("255").x);
    ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("239").x);
    ImGui::TableSetupColumn("Tile", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("FF + ").x + 24.0f);
    ImGui::TableSetupColumn("Pri", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Behind").x);
    ImGui::TableSetupColumn("Flags", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("H:F V:F P:3").x);
    ImGui::TableHeadersRow();

    // Get pattern table textures once
    const std::vector<PPU::Pixel> pattern_pixels_0 = ppu_->GetPatternTableSprite(0, 0);
    const std::vector<PPU::Pixel> pattern_pixels_1 = ppu_->GetPatternTableSprite(1, 0);
    SDL_Texture* pattern_texture_0 = gfx_.GetPatternTableTexture(pattern_pixels_0, 0);
    SDL_Texture* pattern_texture_1 = gfx_.GetPatternTableTexture(pattern_pixels_1, 1);

    // Iterate through all 64 sprites
    for (int i = 0; i < 64; i++) {
        const auto& sprite = ppu_->oam_.sprites[i];

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%2d", i);

        ImGui::TableNextColumn();
        ImGui::Text("%3d", sprite.x_);

        ImGui::TableNextColumn();
        ImGui::Text("%3d", sprite.y_);

        ImGui::TableNextColumn();
        // Show tile ID and preview on same line
        ImGui::Text("%02X", sprite.tile_id_);
        ImGui::SameLine();
        // Calculate position in pattern table
        const int tile_x = (sprite.tile_id_ % 16) * 8;
        const int tile_y = (sprite.tile_id_ / 16) * 8;

        // Select the correct pattern table based on sprite_pattern_table_ in 8x8 mode
        // In 8x16 mode, bit 0 of tile number selects the pattern table
        SDL_Texture* tex = ppu_->ctrl_.sprite_size_
                               ? ((sprite.tile_id_ & 1) ? pattern_texture_1 : pattern_texture_0)
                               : (ppu_->ctrl_.sprite_pattern_table_ ? pattern_texture_1 : pattern_texture_0);

        ImGui::Image(tex,
                     ImVec2(24, ppu_->ctrl_.sprite_size_ ? 48 : 24), // 3x scale (8x8 or 8x16)
                     ImVec2(tile_x / 128.0f, tile_y / 128.0f), // UV0 (top-left)
                     ImVec2((tile_x + 8) / 128.0f, // UV1 (bottom-right)
                            (tile_y + (ppu_->ctrl_.sprite_size_ ? 16 : 8)) / 128.0f)
        );

        ImGui::TableNextColumn();
        ImGui::Text("%s", sprite.priority_ ? "Behind" : "Front");

        ImGui::TableNextColumn();
        ImGui::Text("H:%c V:%c P:%d",
                    sprite.flip_h_ ? 'F' : '-', // Horizontal flip
                    sprite.flip_v_ ? 'F' : '-', // Vertical flip
                    sprite.palette_); // Palette
    }
    ImGui::EndTable();
}


void GraphicsDebug::RenderFlagsView() const {
    static const char* flag_names[] = {"C", "Z", "I", "D", "B", "R", "V", "N"};
    for (int i = 7; i >= 0; --i) {
        if (i != 7) ImGui::SameLine();
        const bool set = (cpu_->P() >> i) & 1;
        ImVec4 color = set ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
        ImGui::TextColored(color, "%s", flag_names[i]);
    }
}

void GraphicsDebug::RenderRegisterView() const {
    ImGui::Text("clocks: %u", bus_->total_cycles_);
    ImGui::Text("PC: %04X", cpu_->PC());
    ImGui::Text("A: %02X (%d)", cpu_->A(), cpu_->A());
    ImGui::SameLine(0.0f, 20.0f);
    ImGui::Text("X: %02X (%d)", cpu_->X(), cpu_->X());
    ImGui::SameLine(0.0f, 20.0f);
    ImGui::Text("Y: %02X (%d)", cpu_->Y(), cpu_->Y());
    ImGui::Text("SP: %02X", cpu_->SP());
}

void GraphicsDebug::RenderDisassemblyView() const {
    if (!ImGui::CollapsingHeader("Disassembly"))
        return;

    if (bus_->cartridge_ != nullptr && !bus_->disassembly_.empty()) {
        const uint16_t pc = cpu_->PC();
        constexpr int lines_before = 13;
        constexpr int lines_after = 13;
        constexpr int lines_total = lines_before + lines_after + 1;

        // Find the iterator for the current PC
        auto it = bus_->disassembly_.find(pc);
        if (it == bus_->disassembly_.end()) return;

        // Walk backward to get the starting iterator
        auto start_it = it;
        for (int i = 0; i < lines_before; ++i) {
            if (start_it == bus_->disassembly_.begin()) break;
            --start_it;
        }

        // Display lines_total lines from start_it
        auto show_it = start_it;
        for (int i = 0; i < lines_total && show_it != bus_->disassembly_.end(); ++i, ++show_it) {
            uint16_t show_pc = show_it->first;
            const std::string& line = show_it->second;
            if (show_pc == pc) {
                auto color = ImVec4(1, 1, 0, 1); // highlight current PC in yellow
                ImGui::TextColored(color, "%s", line.c_str());
            }
            else {
                ImGui::Text("%s", line.c_str());
            }
        }
    }
}


void GraphicsDebug::RenderFpsCounter() {
    static uint32_t last_time = SDL_GetTicks();
    static int frame_count = 0;
    static float fps = 0.0f;
    frame_count++;
    const uint32_t now = SDL_GetTicks();
    if (now - last_time >= 500) {
        // update every 0.5s for stability
        fps = static_cast<float>(frame_count) * 1000.0f / static_cast<float>(now - last_time);
        last_time = now;
        frame_count = 0;
    }
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::Begin("FPS", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoNav);
    ImGui::Text("FPS: %.1f", fps);
    ImGui::End();
}

void GraphicsDebug::RenderPaletteView() const {
    constexpr float kButtonSize = 20.0f;
    constexpr float kPaletteSpacing = 8.0f;
    constexpr ImU32 kHighlightColor = IM_COL32(255, 255, 0, 255);
    constexpr float kHighlightThickness = 3.0f;

    ImGui::Text("Palettes:");
    for (int row = 0; row < 2; ++row) {
        for (int pal = 0; pal < 4; ++pal) {
            const int palette_idx = row * 4 + pal;
            if (pal > 0) ImGui::SameLine(0.0f, kPaletteSpacing);

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const ImVec2 palette_start = ImGui::GetCursorScreenPos();

            // Render 4 color buttons for this palette
            for (int col = 0; col < 4; ++col) {
                const uint8_t color_idx = ppu_->PpuRead(0x3F00 + palette_idx * 4 + col);
                const PPU::Pixel px = PPU::GetPaletteColor(color_idx);

                char btn_id[16];
                snprintf(btn_id, sizeof(btn_id), "##pal%d_%d", palette_idx, col);
                ImGui::ColorButton(btn_id,
                                   ImVec4(px.r_ / 255.0f, px.g_ / 255.0f, px.b_ / 255.0f, 1.0f),
                                   ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
                                   ImVec2(kButtonSize, kButtonSize));

                if (col < 3) ImGui::SameLine();
            }
            ImGui::PopStyleVar();

            // Draw highlight for selected palette
            if (palette_idx == selected_palette_) {
                ImGui::GetWindowDrawList()->AddRect(
                    palette_start,
                    ImVec2(palette_start.x + kButtonSize * 4, palette_start.y + kButtonSize),
                    kHighlightColor, 0.0f, 0, kHighlightThickness);
            }
        }
        ImGui::NewLine();
    }
    ImGui::Dummy(ImVec2(0, 10));
}

void GraphicsDebug::RenderPatternTableView() const {
    ImGui::Text("Pattern Tables:");
    const std::vector<PPU::Pixel> pattern_pixels_0 = ppu_->GetPatternTableSprite(0, selected_palette_);
    SDL_Texture* pattern_texture_0 = gfx_.GetPatternTableTexture(pattern_pixels_0, 0);
    ImGui::BeginGroup();
    ImGui::Image(pattern_texture_0, ImVec2(128 * kPatternViewScale, 128 * kPatternViewScale));
    ImGui::EndGroup();
    ImGui::SameLine();
    const std::vector<PPU::Pixel> pattern_pixels_1 = ppu_->GetPatternTableSprite(1, selected_palette_);
    SDL_Texture* pattern_texture_1 = gfx_.GetPatternTableTexture(pattern_pixels_1, 1);
    ImGui::BeginGroup();
    ImGui::Image(pattern_texture_1, ImVec2(128 * kPatternViewScale, 128 * kPatternViewScale));
    ImGui::EndGroup();
}

int main(const int argc, char** argv) {
    if (argc >= 3) {
        std::cout << "Usage: graphics_debug <romfile>\n";
    }

    auto gfx = GraphicsWrapper();
    auto gb = GraphicsDebug(gfx);

    // Load cartridge if ROM provided
    if (argc == 2 && !gb.bus_->LoadCartridge(argv[1])) {
        std::cerr << "Failed to load cartridge: " << argv[1] << std::endl;
    }

    if (!gfx.Initialize("NES Debug", PPU::kWidth, PPU::kHeight, 5)) {
        std::cerr << "Failed to initialize graphics!\n";
        return 1;
    }
    auto startRef = gb.bus_->cpu_;

    while (!gfx.ShouldClose()) {
        const uint32_t frame_start = SDL_GetTicks();
        gfx.BeginFrame();

        gb.bus_->curr_controller_state[0] = GraphicsWrapper::getNewController1State();

        if (GraphicsWrapper::getKey(SDL_SCANCODE_SPACE).pressed)
            gb.run_mode_ = !gb.run_mode_;

        if (gb.run_mode_) {
            do { gb.bus_->Step(); }
            while (!gb.ppu_->frame_complete_);

            const uint32_t frame_time = SDL_GetTicks() - frame_start;

            if (frame_time < GraphicsDebug::kFrameDelay)
                SDL_Delay(GraphicsDebug::kFrameDelay - frame_time);
        }
        else {
            if (GraphicsWrapper::getKey(SDL_SCANCODE_C).pressed) {
                do {
                    gb.bus_->Step();
                }
                while (!gb.cpu_->IsComplete());
                do {
                    gb.bus_->Step();
                }
                while (gb.cpu_->IsComplete());
            }

            if (GraphicsWrapper::getKey(SDL_SCANCODE_F).pressed)
                do { gb.bus_->Step(); }
                while (!gb.ppu_->frame_complete_);

            if (GraphicsWrapper::getKey(SDL_SCANCODE_P).pressed)
                gb.selected_palette_ = (gb.selected_palette_ + 1) % 8;
        }


        gfx.UpdateFramebuffer(gb.ppu_->GetFrameBuffer());

        gb.RenderDebugInfo();
        GraphicsDebug::RenderFpsCounter();

        gfx.EndFrame();

        if (gb.ppu_->frame_complete_)
            gb.ppu_->frame_complete_ = false;
    }
    gfx.Shutdown();
    return 0;
}
