#include "Precompiled.h"
#include "Texture2DDetails.h"

#include "Suora/Assets/Texture2D.h"

namespace Suora
{
	
	void Texture2DDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->IsA<Texture2D>())
		{
			ViewTexture2D(y, obj->As<Texture2D>());
		}
	}

	void Texture2DDetails::ViewTexture2D(float& y, Texture2D* texture)
	{

	}

}