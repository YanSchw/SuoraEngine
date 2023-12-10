#pragma once
#include "Suora/Common/StringUtils.h"
#include "Suora/Common/SuoraID.h"

namespace Suora
{
	class Texture;
	class Texture2D;

	struct Icon
	{
		Icon() = delete;
		explicit Icon(const SuoraID& id);

		Texture* GetTexture() const;

		operator Texture*() const;

		// ICONS
		static const Icon Actor;
		static const Icon ArrowDown;
		static const Icon ArrowRight;
		static const Icon ArrowUp;
		static const Icon Box;
		static const Icon Bug;
		static const Icon Camera;
		static const Icon Capsule;
		static const Icon Character;
		static const Icon Cogwheel;
		static const Icon Continue;
		static const Icon Controller;
		static const Icon Critical;
		static const Icon Decal;
		static const Icon Trashbin;
		static const Icon Sun;
		static const Icon Error;
		static const Icon Folder;
		static const Icon Globe;
		static const Icon Grid;
		static const Icon Image;
		static const Icon World;
		static const Icon Info;
		static const Icon MaterialSphere;
		static const Icon Mesh;
		static const Icon Pause;
		static const Icon Play;
		static const Icon LightBulb;
		static const Icon Effect;
		static const Icon Rename;
		static const Icon FloppyDisk;
		static const Icon Sphere;
		static const Icon Stop;
		static const Icon Rotate;
		static const Icon Scale;
		static const Icon Select;
		static const Icon Translate;
		static const Icon TextBubble;
		static const Icon Undo;
		static const Icon NotVisisble;
		static const Icon Visisble;
		static const Icon Warning;


	private:
		Texture2D* m_Texture = nullptr;
	};

}