#include "gpu_resources.h"

namespace sren {

// RenderPassOutput
RenderPassOutput &RenderPassOutput::reset() {
    num_color_formats = 0;
    for (u32 i = 0; i < max_image_outputs; ++i) {
        color_formats[i] = VK_FORMAT_UNDEFINED;
    }
    depth_stencil_format = VK_FORMAT_UNDEFINED;
    color_operation = depth_operation = stencil_operation = RenderPassOperation::DontCare;
    return *this;
}

RenderPassOutput &RenderPassOutput::color(VkFormat format) {
    color_formats[num_color_formats++] = format;
    return *this;
}

RenderPassOutput &RenderPassOutput::depth(VkFormat format) {
    depth_stencil_format = format;
    return *this;
}

RenderPassOutput &RenderPassOutput::set_operations(RenderPassOperation::Enum color_,
                                                   RenderPassOperation::Enum depth_,
                                                   RenderPassOperation::Enum stencil_) {
    color_operation = color_;
    depth_operation = depth_;
    stencil_operation = stencil_;

    return *this;
}

} // namespace sren