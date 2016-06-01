#include "GaussianBlurHoriProg.h"
#include "../render/RenderShader.h"
#include "../parser/GaussianBlurHori.h"

// for UNIFORM_INT1
#include <render/render.h>

namespace sl
{

GaussianBlurHoriProg::GaussianBlurHoriProg(RenderContext* rc, int max_vertex, 
										   const std::vector<VertexAttrib>& va_list, RenderBuffer* ib)
	: FilterProgram(rc, max_vertex)
	, m_tex_width_val(0)
{
	Init(va_list, ib, new parser::GaussianBlurHori());

	m_tex_width_id = m_shader->AddUniform("u_tex_width", UNIFORM_FLOAT1);
}

void GaussianBlurHoriProg::SetTexWidth(float width)
{
	if (width != m_tex_width_val) {
		m_tex_width_val = width;
		m_shader->SetUniform(m_tex_width_id, UNIFORM_FLOAT1, &width);
	}
}

}