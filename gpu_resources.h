#pragma once

#include "platform.h"

#include <vulkan/vulkan_core.h>

namespace sren {

// Maximum number of images/render_targets/fbo attachments usable.
static const u8 max_image_outputs = 8;
// Maximum number of layouts in the pipeline.
static const u8 max_descriptor_set_layouts = 8;
// Maximum simultaneous shader stages. Applicable to all different type of pipelines.
static const u8 max_shader_stages = 5;
// Maximum list elements for both descriptor set layout and descriptor sets.
static const u8 max_descriptors_per_set = 16;
static const u8 max_vertex_streams = 16;
static const u8 max_vertex_attributes = 16;

static const u32 submit_header_sentinel = 0xfefeb7ba;
static const u32 max_resource_deletions = 64;

namespace RenderPassOperation {
enum Enum { DontCare, Load, Clear, Count }; // enum Enum
} // namespace RenderPassOperation

class RenderPassOutput {
  public:
    VkFormat color_formats[max_image_outputs];
    VkFormat depth_stencil_format;
    u32 num_color_formats;

    RenderPassOperation::Enum color_operation = RenderPassOperation::DontCare;
    RenderPassOperation::Enum depth_operation = RenderPassOperation::DontCare;
    RenderPassOperation::Enum stencil_operation = RenderPassOperation::DontCare;

    RenderPassOutput &reset();
    RenderPassOutput &color(VkFormat format);
    RenderPassOutput &depth(VkFormat format);
    RenderPassOutput &set_operations(RenderPassOperation::Enum color, RenderPassOperation::Enum depth,
                                     RenderPassOperation::Enum stencil);

}; // struct RenderPassOutput

} // namespace sren