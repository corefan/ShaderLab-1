#ifndef _SHADERLAB_RELIEF_PROG_H_
#define _SHADERLAB_RELIEF_PROG_H_

#include "FilterProgram.h"

namespace sl
{

class ReliefProg : public FilterProgram
{
public:
	ReliefProg(RenderContext* rc, int max_vertex, 
		const std::vector<VertexAttrib>& va_list, RenderBuffer* ib);

}; // ReliefProg

}

#endif // _SHADERLAB_RELIEF_PROG_H_