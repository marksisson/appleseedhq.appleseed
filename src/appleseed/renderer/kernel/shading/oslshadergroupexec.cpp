
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2014-2015 Esteban Tovagliari, The appleseedhq Organization
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

// Interface header.
#include "oslshadergroupexec.h"

// appleseed.renderer headers.
#include "renderer/kernel/shading/closures.h"
#include "renderer/kernel/shading/shadingpoint.h"
#include "renderer/kernel/shading/shadingray.h"
#include "renderer/modeling/shadergroup/shadergroup.h"

// Standard headers.
#include <cassert>

using namespace foundation;

namespace renderer
{

//
// OSLShaderGroupExec class implementation.
//

OSLShaderGroupExec::OSLShaderGroupExec(OSL::ShadingSystem& shading_system)
  : m_osl_shading_system(shading_system)
  , m_osl_thread_info(shading_system.create_thread_info())
  , m_osl_shading_context(shading_system.get_context(m_osl_thread_info))
{
}

OSLShaderGroupExec::~OSLShaderGroupExec()
{
    if (m_osl_shading_context)
        m_osl_shading_system.release_context(m_osl_shading_context);

    if (m_osl_thread_info)
        m_osl_shading_system.destroy_thread_info(m_osl_thread_info);
}

void OSLShaderGroupExec::execute_shading(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point) const
{
    do_execute(
        shader_group,
        shading_point,
        shading_point.get_ray().m_flags);
}

void OSLShaderGroupExec::execute_transparency(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point,
    Alpha&                          alpha,
    float*                          holdout) const
{
    do_execute(
        shader_group,
        shading_point,
        VisibilityFlags::TransparencyRay);

    process_transparency_tree(shading_point.get_osl_shader_globals().Ci, alpha);

    if (holdout)
        *holdout = process_holdout_tree(shading_point.get_osl_shader_globals().Ci);
}

void OSLShaderGroupExec::execute_shadow(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point,
    Alpha&                          alpha) const
{
    do_execute(
        shader_group,
        shading_point,
        VisibilityFlags::ShadowRay);

    process_transparency_tree(shading_point.get_osl_shader_globals().Ci, alpha);
}

void OSLShaderGroupExec::execute_emission(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point) const
{
    do_execute(
        shader_group,
        shading_point,
        VisibilityFlags::LightRay);
}

void OSLShaderGroupExec::execute_bump(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point,
    const double                    s) const
{
    do_execute(
        shader_group,
        shading_point,
        VisibilityFlags::CameraRay);

    CompositeSurfaceClosure c(
        0,
        shading_point.get_shading_basis(),
        shading_point.get_osl_shader_globals().Ci);

    if (c.get_num_closures() > 0)
    {
        const size_t index = c.choose_closure(s);
        shading_point.set_shading_basis(
            c.get_closure_shading_basis(index));
    }
}

Color3f OSLShaderGroupExec::execute_background(
    const ShaderGroup&              shader_group,
    const Vector3d&                 outgoing) const
{
    assert(m_osl_shading_context);
    assert(m_osl_thread_info);

    OSL::ShaderGlobals sg;
    memset(&sg, 0, sizeof(OSL::ShaderGlobals));
    sg.I = Vector3f(outgoing);
    sg.renderer = m_osl_shading_system.renderer();
    sg.raytype = VisibilityFlags::CameraRay;

    m_osl_shading_system.execute(
        *m_osl_shading_context,
        *shader_group.shader_group_ref(),
        sg);

    return process_background_tree(sg.Ci);
}

void OSLShaderGroupExec::do_execute(
    const ShaderGroup&              shader_group,
    const ShadingPoint&             shading_point,
    const VisibilityFlags::Type     ray_flags) const
{
    assert(m_osl_shading_context);
    assert(m_osl_thread_info);

    shading_point.initialize_osl_shader_globals(
        shader_group,
        ray_flags,
        m_osl_shading_system.renderer());

    m_osl_shading_system.execute(
        *m_osl_shading_context,
        *shader_group.shader_group_ref(),
        shading_point.get_osl_shader_globals());
}

}   // namespace renderer
