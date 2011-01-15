
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2011 Francois Beaune
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_RENDERER_KERNEL_RENDERING_PROGRESSIVE_SAMPLEGENERATOR_H
#define APPLESEED_RENDERER_KERNEL_RENDERING_PROGRESSIVE_SAMPLEGENERATOR_H

// appleseed.renderer headers.
#include "renderer/global/global.h"
#include "renderer/kernel/rendering/progressive/sample.h"

// Standard headers.
#include <vector>

// Forward declarations.
namespace foundation    { class LightingConditions; }
namespace foundation    { class MersenneTwister; }
namespace renderer      { class Frame; }
namespace renderer      { class ISampleRenderer; }
namespace renderer      { class ProgressiveFrameBuffer; }

namespace renderer
{

class SampleGenerator
  : public foundation::NonCopyable
{
  public:
    // Constructor.
    SampleGenerator(
        Frame&                              frame,
        ISampleRenderer*                    sample_renderer,
        const size_t                        generator_index,
        const size_t                        generator_count);

    // Generate @sample_count samples and store them in @framebuffer.
    void generate_samples(
        const size_t                        sample_count,
        ProgressiveFrameBuffer&             framebuffer);

  private:
    Frame&                                  m_frame;
    ISampleRenderer*                        m_sample_renderer;
    const foundation::LightingConditions&   m_lighting_conditions;
    const size_t                            m_stride;
    size_t                                  m_sequence_index;
    foundation::MersenneTwister             m_rng;
    std::vector<Sample>                     m_samples;

    void generate_sample(Sample& sample);
};

}       // namespace renderer

#endif  // !APPLESEED_RENDERER_KERNEL_RENDERING_PROGRESSIVE_SAMPLEGENERATOR_H
