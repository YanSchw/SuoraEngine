#pragma once

namespace Suora 
{

	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;

		virtual void Init() = 0;
		virtual void SwapBuffers() = 0;
		virtual void MakeCurrent() = 0;

		static Scope<GraphicsContext> Create(void* window);
	};

}