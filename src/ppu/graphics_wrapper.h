#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include <SDL.h>

#include "ppu.h"

// Forward declarations for SDL types
struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

// GraphicsWrapper: Abstracts SDL2 and ImGui setup for the PPU
class GraphicsWrapper {
public:
    struct KeyState {
        bool pressed;
    };
    GraphicsWrapper();
    ~GraphicsWrapper();

    static KeyState getKey(SDL_Scancode scancode);

    static uint8_t getNewController1State();

    // Initialize SDL2, ImGui, and create window/renderer/texture
    bool Initialize(const std::string& title, int width, int height, int scale = 2);
    // Shutdown SDL2 and ImGui
    void Shutdown() const;

    // Begin a new frame (handles ImGui and SDL events)
    void BeginFrame();
    // End the frame (renders everything to the screen)
    void EndFrame() const;

    // Update the NES framebuffer (palette indices)
    void UpdateFramebuffer(const std::vector<PPU::Pixel>& framebuffer) const;


    // Check if the window should close
    [[nodiscard]] bool ShouldClose() const;

    // Converts a vector of PPU::Pixel (NES colors) to a vector of 32-bit RGBA values
    static std::vector<uint32_t> ConvertPixelsToRgba(const std::vector<PPU::Pixel>& pixels);

    // Returns an SDL_Texture* for the given pattern table index (0 or 1), generating and updating it as needed
    [[nodiscard]] SDL_Texture* GetPatternTableTexture(const std::vector<PPU::Pixel>& pixels, int table_idx) const;

private:
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
    SDL_Texture* pattern_texture_0_ = nullptr;
    SDL_Texture* pattern_texture_1_ = nullptr;
    bool running_ = true;
    int window_width_ = 0;
    int window_height_ = 0;
    int scale_ = 2;
};
