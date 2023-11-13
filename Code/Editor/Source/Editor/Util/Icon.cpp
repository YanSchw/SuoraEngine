#include "Icon.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/Texture2D.h"

namespace Suora
{

	const Icon Icon::Actor                     = Icon(SuoraID("ad168979-55cd-408e-afd2-a24cabf26922"));
	const Icon Icon::ArrowDown                 = Icon(SuoraID("8742cec8-9ee5-4645-b036-577146904b41"));
	const Icon Icon::ArrowRight                = Icon(SuoraID("970c3d0e-c5b0-4a2e-a548-661d9b00d977"));
	const Icon Icon::ArrowUp                   = Icon(SuoraID("98548865-889f-4705-abe7-37a83fdd652d"));
	const Icon Icon::Box                       = Icon(SuoraID("269931d5-7e60-4934-a89a-26b7993ae0f3"));
	const Icon Icon::Bug                       = Icon(SuoraID("cfb089b4-bd21-4ee6-93a3-a6334e15430e"));
	const Icon Icon::Camera                    = Icon(SuoraID("8952ef88-cbd0-41ab-9d3c-d4c4b39a30f9"));
	const Icon Icon::Capsule                   = Icon(SuoraID("b7221496-4fc6-4e08-9f23-655d5edfe820"));
	const Icon Icon::Character                 = Icon(SuoraID("0660326f-a8a2-4314-bc96-9b61bdbddae3"));
	const Icon Icon::Cogwheel                  = Icon(SuoraID("3e254a4e-cc83-4254-a462-73739fce6d61"));
	const Icon Icon::Continue                  = Icon(SuoraID("073b49ee-bfe3-472a-9b15-78b864268b0e"));
	const Icon Icon::Critical                  = Icon(SuoraID("7df5a6ba-75a5-4974-87d7-047e58cce35e"));
	const Icon Icon::Decal                     = Icon(SuoraID("9d81a066-2336-42f5-bf35-7bb1c4c65d66"));
	const Icon Icon::Trashbin                  = Icon(SuoraID("fe478bcb-d2be-4a9c-9dd3-ab79e5f3b0e4"));
	const Icon Icon::Sun                       = Icon(SuoraID("64738d74-08a9-4383-8659-620808d5269a"));
	const Icon Icon::Error                     = Icon(SuoraID("7224f801-1370-4f82-ac37-0cecf2cc35b9"));
	const Icon Icon::Folder                    = Icon(SuoraID("8c5f9517-4cab-4320-ab7a-db4e00b57ad0"));
	const Icon Icon::Globe                     = Icon(SuoraID("24294e57-bae7-4ff7-a0f2-73f9741069da"));
	const Icon Icon::Grid                      = Icon(SuoraID("634893f5-8fec-4ef9-88b5-14b6a0aea623"));
	const Icon Icon::Image                     = Icon(SuoraID("26ed62e2-e39b-4028-aca2-7b116c74541f"));
	const Icon Icon::World                     = Icon(SuoraID("3578494c-3c74-4aa5-8d34-4d28959a21f5"));
	const Icon Icon::Info                      = Icon(SuoraID("b3758660-1e0a-4a6a-9223-1a25966bdefd"));
	const Icon Icon::MaterialSphere            = Icon(SuoraID("10db4794-ec22-46fc-8be9-6cd8e5245349"));
	const Icon Icon::Mesh                      = Icon(SuoraID("b14b065c-a2c0-4dc9-9272-ab0415ada141"));
	const Icon Icon::Pause                     = Icon(SuoraID("af9d611d-dfc9-4bc5-a994-5f63b443842c"));
	const Icon Icon::Play                      = Icon(SuoraID("c503c57f-100f-4e0d-8f27-38259b174ba8"));
	const Icon Icon::LightBulb                 = Icon(SuoraID("f789d2bf-dcda-4e30-b2d9-3db979b7c6da"));
	const Icon Icon::Effect                    = Icon(SuoraID("0001ed8d-ef3a-4375-be4b-d03cfc4febfd"));
	const Icon Icon::Rename                    = Icon(SuoraID("76a3d617-de80-49b1-bc53-694e033387c9"));
	const Icon Icon::FloppyDisk                = Icon(SuoraID("617d1c03-b141-4d71-9d73-993188ff011f"));
	const Icon Icon::Sphere                    = Icon(SuoraID("7e43f48b-3dc8-4eab-b91a-b4e2e7999190"));
	const Icon Icon::Stop                      = Icon(SuoraID("bf120f8b-0aca-4e28-993e-ba1e9ce4e693"));
	const Icon Icon::Rotate                    = Icon(SuoraID("883caf6f-1080-4758-854f-713a68ef2602"));
	const Icon Icon::Scale                     = Icon(SuoraID("b9df5e5c-349b-4c41-baca-746c98d0478a"));
	const Icon Icon::Select                    = Icon(SuoraID("38654b71-4b30-43ce-b2bf-c5a516257e93"));
	const Icon Icon::Translate                 = Icon(SuoraID("544a3477-4ec1-4a9c-acdc-1201321b26d3"));
	const Icon Icon::TextBubble                = Icon(SuoraID("b17ddd50-8a50-4143-ac3d-028d3366495b"));
	const Icon Icon::Undo                      = Icon(SuoraID("f11389f9-2062-497a-a24c-c27c3c7b93f9"));
	const Icon Icon::NotVisisble               = Icon(SuoraID("8dd405ad-59d9-4a13-8a2b-b610310bf600"));
	const Icon Icon::Visisble                  = Icon(SuoraID("b63772a8-57cf-47ef-8c6f-1e3f6dbd61e6"));
	const Icon Icon::Warning                   = Icon(SuoraID("cf027e56-ead0-4e61-ac31-b3d9125a03d9"));


	Icon::Icon(const SuoraID& id)
	{
		m_Texture = AssetManager::GetAsset<Texture2D>(id);
	}

	Texture* Icon::GetTexture() const
	{
		return m_Texture->GetTexture();
	}

	Icon::operator Texture*() const
	{
		return GetTexture();
	}

}