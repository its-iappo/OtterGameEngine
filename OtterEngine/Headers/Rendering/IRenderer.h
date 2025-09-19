#pragma once

namespace OtterEngine {

    class IRenderer {
    public:
        virtual ~IRenderer() = default;
        virtual void Init() = 0;
        virtual void Clear() = 0;
        virtual void DrawFrame() = 0;
    };

}
