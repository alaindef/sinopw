#pragma once
#include "RenderMath.h"
#include "Texture.h"
#include <functional>
#include <string>

class CRenderer
{
    public:
    static void Init(int argc, char *argv[]);
    static void Render();
    static void Reshape(GLsizei,GLsizei);

    static void DrawImage(CTexture* tex, Rect* sourcerect = nullptr, Color colorargb = 0xFFFFFFFF);
    static void StartRendering();
    static void SetRenderFunction(const std::function<void(void)>& f) { mRenderFunction = f;}
    static void InitSetStart(int argc, char *argv[],
        const std::function<void(std::vector<std::string>)> &init_f,
        const std::vector<std::string> &script,
        const std::function<void(void)>& render_f);

    private:
    static CTexture mTexture;
    static std::function<void(void)> mRenderFunction;
};
