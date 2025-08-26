#include "graphics_wrapper.h"
#include <SDL.h>
#include <set>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <vector>
#include "ppu.h"


GraphicsWrapper::KeyState GraphicsWrapper::getKey(const SDL_Scancode scancode) {
    static Uint8 prev_state[SDL_NUM_SCANCODES] = {0};
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    const bool pressed = (state[scancode] != 0) && (prev_state[scancode] == 0);
    prev_state[scancode] = state[scancode];
    return KeyState{pressed};
}

uint8_t GraphicsWrapper::getNewController1State() {
    uint8_t curr_state = 0x00;
    curr_state |= getKey(SDL_SCANCODE_X).pressed ?     0b10000000 : 0; // B button
    curr_state |= getKey(SDL_SCANCODE_Z).pressed ?     0b01000000 : 0; // A button
    curr_state |= getKey(SDL_SCANCODE_A).pressed ?     0b00100000 : 0; // SELECT button
    curr_state |= getKey(SDL_SCANCODE_S).pressed ?     0b00010000 : 0; // START button
    curr_state |= getKey(SDL_SCANCODE_UP).pressed ?    0b00001000 : 0; // UP button
    curr_state |= getKey(SDL_SCANCODE_DOWN).pressed ?  0b00000100 : 0; // DOWN button
    curr_state |= getKey(SDL_SCANCODE_LEFT).pressed ?  0b00000010 : 0; // LEFT button
    curr_state |= getKey(SDL_SCANCODE_RIGHT).pressed ? 0b00000001 : 0; // RIGHT button

    return curr_state;
}

GraphicsWrapper::GraphicsWrapper() = default;

GraphicsWrapper::~GraphicsWrapper() {
    Shutdown();
}

bool GraphicsWrapper::Initialize(const std::string& title, const int width, const int height, const int scale) {
    window_width_ = width * scale;
    window_height_ = height * scale;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) return false;
    window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width_,
                               window_height_, SDL_WINDOW_SHOWN);
    if (!window_) return false;
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) return false;
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (!texture_) return false;
    // Create pattern table textures
    pattern_texture_0_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
    pattern_texture_1_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 128, 128);
    if (!pattern_texture_0_ || !pattern_texture_1_) return false;
    // ImGui setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer2_Init(renderer_);
    running_ = true;
    return true;
}

void GraphicsWrapper::Shutdown() const {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (pattern_texture_0_) SDL_DestroyTexture(pattern_texture_0_);
    if (pattern_texture_1_) SDL_DestroyTexture(pattern_texture_1_);
    if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    SDL_Quit();
}

void GraphicsWrapper::BeginFrame() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT) running_ = false;
    }
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void GraphicsWrapper::EndFrame() const {
    ImGui::Render();
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
}

void GraphicsWrapper::UpdateFramebuffer(const std::vector<PPU::Pixel>& framebuffer) const {
    std::vector<uint32_t> rgba_buffer = ConvertPixelsToRgba(framebuffer);
    SDL_UpdateTexture(texture_, nullptr, rgba_buffer.data(), PPU::kWidth * sizeof(uint32_t));
}

bool GraphicsWrapper::ShouldClose() const {
    return !running_;
}

std::vector<uint32_t> GraphicsWrapper::ConvertPixelsToRgba(const std::vector<PPU::Pixel>& pixels) {
    if (pixels.empty()) {
        // Return an empty vector if input is empty
        return std::vector<uint32_t>();
    }

    std::vector<uint32_t> rgba;
    rgba.reserve(pixels.size());

    for (const auto& pixel : pixels) {
        const uint32_t color = (pixel.r_ << 24) | (pixel.g_ << 16) | (pixel.b_ << 8) | 0xFF;
        rgba.push_back(color);
    }
    return rgba;
}

SDL_Texture* GraphicsWrapper::GetPatternTableTexture(const std::vector<PPU::Pixel>& pixels, int table_idx) const {
    const std::vector<uint32_t> pattern_rgba = ConvertPixelsToRgba(pixels);
    SDL_Texture* tex = (table_idx == 0) ? pattern_texture_0_ : pattern_texture_1_;
    SDL_UpdateTexture(tex, nullptr, pattern_rgba.data(), 128 * sizeof(uint32_t));
    return tex;
}

