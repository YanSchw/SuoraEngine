#include "Precompiled.h"
#include "NodeDetails.h"

#include "Suora/Assets/Texture2D.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/Editor/Panels/Major/NodeClassEditor.h"

namespace Suora
{
	static void DragFloat2(Vec2* vec, float x, float y, float width, float height)
	{
		float singleWidth = width / 2.0f;
		EditorUI::DragFloat(&vec->x, x + singleWidth * 0.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->y, x + singleWidth * 1.0f, y, singleWidth, height);
	}
	static void DragFloat3(Vec3* vec, float x, float y, float width, float height)
	{
		float singleWidth = width / 3.0f;
		EditorUI::DragFloat(&vec->x, x + singleWidth * 0.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->y, x + singleWidth * 1.0f, y, singleWidth, height);
		EditorUI::DragFloat(&vec->z, x + singleWidth * 2.0f, y, singleWidth, height);
	}

	void NodeDetails::ViewObject(Object* obj, float& y)
	{
		if (obj->GetNativeClass().Inherits(Node::StaticClass()))
		{
			ViewNode(y, obj->As<Node>());
		}
	}

	void NodeDetails::ViewNode(float& y, Node* node)
	{

		if (!node)
		{
			EditorUI::Text("Select a Node to view its Details.", Font::Instance, 0, 0, GetWidth(), GetHeight(), 36, Vec2(0, 0), Color(1));
			return;
		}

		bool NodeEnabled = node->m_Enabled;

		y -= 40;
		EditorUI::Button("", 0.0f, y, 40.0f, 40.0f, ShutterPanelParams());
		EditorUI::DrawTexturedRect(EditorUI::GetClassIcon(node->GetClass())->GetTexture(), 5.0f, y + 5.0f, 30.0f, 30.0f, 0.0f, Color(1));
		EditorUI::Button("", 40.0f, y, GetDetailWidth() - 80.0f, 40.0f, ShutterPanelParams());
		EditorUI::TextField(&node->m_Name, 45.0f, y + 7.5f, (GetDetailWidth() - 40.0f) / 2.0f, 25.0f);
		node->SetName(node->GetName());
		EditorUI::Button("", GetDetailWidth() - 40.0f, y, 40.0f, 40.0f, ShutterPanelParams());
		EditorUI::Checkbox(&node->m_Enabled, GetDetailWidth() - 40.0f + 7.5f, y + 7.5f, 25.0f, 25.0f);
		y -= 10;
		EditorUI::Text("=======================================================================", Font::Instance, 0.0f, y + 5.0f, GetDetailWidth(), 5.0f, 18.0f, Vec2(0.0f), Color(0.65f));

		if (NodeEnabled != node->m_Enabled)
		{
			node->SetEnabled(node->m_Enabled);
		}

		bool skipFirstDerivative = false;
		if (Node3D* node3D = node->As<Node3D>())
		{
			skipFirstDerivative = true;
			y -= 35;
			static bool readTransform = true;
			static std::function<void(String)> disableReadTransform = [](String s) { readTransform = false; };
			if (readTransform) // Doesn't work with writing values in the TextBox!
			{
				Transform_Pos = node3D->GetLocalPosition();
				Transform_Rot = node3D->GetEulerRotation();
				Transform_Scale = node3D->GetLocalScale();
				if (Transform_Rot.x == -0.0f || Transform_Rot.x == 0.0f) Transform_Rot.x = 0.0f; if (Transform_Rot.x == -180.0f) Transform_Rot.x = 180.0f;
				if (Transform_Rot.y == -0.0f || Transform_Rot.y == 0.0f) Transform_Rot.y = 0.0f; if (Transform_Rot.y == -180.0f) Transform_Rot.y = 180.0f;
				if (Transform_Rot.z == -0.0f || Transform_Rot.z == 0.0f) Transform_Rot.z = 0.0f; if (Transform_Rot.z == -180.0f) Transform_Rot.z = 180.0f;
			}
			readTransform = true;
			if (EditorUI::CategoryShutter(0, "Node3D", 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				y -= 34;
				DrawLabel("Position", y, 35.0f);
				//static Vec3 position;
				GetDetailsPanel()->DrawVec3Control(&Transform_Pos, GetDetailWidth() * GetSeperator(), y + 4.5f, GetDetailWidth() - GetDetailWidth() * GetSeperator() - 35.0f, 25, 0.0f, disableReadTransform);
				node3D->SetLocalPosition(Transform_Pos);
				y -= 34;
				DrawLabel("Rotation", y, 35.0f);
				if (!GetMajorTab()->IsA<NodeClassEditor>()
					|| (GetMajorTab()->IsA<NodeClassEditor>() && GetMajorTab()->As<NodeClassEditor>()->m_CurrentPlayState == PlayState::Editor)
					|| true)
				{
					//static Vec3 rot;
					GetDetailsPanel()->DrawVec3Control(&Transform_Rot, GetDetailWidth() * GetSeperator(), y + 4.5f, GetDetailWidth() - GetDetailWidth() * GetSeperator() - 35.0f, 25, 0.0f, disableReadTransform);
					node3D->SetEulerRotation(Transform_Rot);
				}
				y -= 34;
				DrawLabel("Scale", y, 35.0f);
				//static Vec3 scale;
				GetDetailsPanel()->DrawVec3Control(&Transform_Scale, GetDetailWidth() * GetSeperator(), y + 4.5f, GetDetailWidth() - GetDetailWidth() * GetSeperator() - 35.0f, 25, 1.0f, disableReadTransform);
				node3D->SetLocalScale(Transform_Scale);
				y -= 15; // Padding Bottom
				node3D->TickTransform(true);

				Transform_LastNode = node;
			}
		}
		if (UINode* uinode = node->As<UINode>())
		{
			skipFirstDerivative = true;
			y -= 35;
			if (EditorUI::CategoryShutter(0, "UINode", 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				y -= 34;
				DrawLabel("Anchor", y, 35.0f);
				DragFloat2(&uinode->m_Anchor, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
				y -= 34;
				DrawLabel("Width", y, 35.0f);
				EditorUI::Checkbox(&uinode->m_IsWidthRelative, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, 25.0f, 25.0f);
				EditorUI::Text("Is Relative", Font::Instance, GetDetailWidth() * GetSeperator() + 35.0f, y + 5.0f, 250.0f, 25.0f, 21.0f, Vec2(-1, 0), Color(1.0f));
				y -= 34;
				DrawLabel("", y, 35.0f);
				EditorUI::DragFloat(&uinode->m_Width, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
				y -= 34;
				DrawLabel("Height", y, 35.0f);
				EditorUI::Checkbox(&uinode->m_IsHeightRelative, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, 25.0f, 25.0f);
				EditorUI::Text("Is Relative", Font::Instance, GetDetailWidth() * GetSeperator() + 35.0f, y + 5.0f, 250.0f, 25.0f, 21.0f, Vec2(-1, 0), Color(1.0f));
				y -= 34;
				DrawLabel("", y, 35.0f);
				EditorUI::DragFloat(&uinode->m_Height, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
				y -= 34;
				DrawLabel("Pivot", y, 35.0f);
				DragFloat2(&uinode->m_Pivot, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
				y -= 34;
				DrawLabel("Absolute Pixel Offset", y, 35.0f);
				DragFloat3(&uinode->m_AbsolutePixelOffset, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
				y -= 34;
				DrawLabel("Euler Rotation around Anchor", y, 35.0f);
				DragFloat3(&uinode->m_EulerRotationAroundAnchor, GetDetailWidth() * GetSeperator() + 5.0f, y + 5.0f, GetDetailWidth() * (0.8f - GetSeperator()), 25.0f);
			}
		}

		// Node Derivates
		Array<Class> derivates = node->GetClass().GetInheritanceTree();
		int memberIndex = 0;
		for (int64_t i = skipFirstDerivative ? 3 : 2; i < derivates.Size(); i++)
		{
			if (!derivates[i].IsNative())
			{
				continue;
			}

			y -= 35;
			const ClassReflector& refl = ClassReflector::GetByClass(derivates[i]);
			if (EditorUI::CategoryShutter(1000 + i, refl.m_ClassName, 0, y, GetDetailWidth(), 35, ShutterPanelParams()))
			{
				for (Ref<ClassMemberProperty> member : refl.m_ClassProperties)
				{
					float x = 0;
					DrawClassMember(x, y, node, member.get(), memberIndex++);
				}

			}
			else
			{
				memberIndex += refl.m_ClassProperties.Size();
			}
		}

		y -= 50;
	}


	void NodeDetails::DrawClassMember(float& x, float& y, Node* obj, ClassMemberProperty* member, int memberIndex)
	{
		//y -= 10.0f;
		const auto type = member->m_Property->GetType();
		const auto mname = member->m_MemberName;
		const bool valueChangedBefore = obj->m_OverwrittenProperties.Contains(mname);
		DetailsPanel::Result result = DetailsPanel::Result::None;

		if (type == PropertyType::Int32)
		{
			int32_t* i = ClassMemberProperty::AccessMember<int32_t>(obj, member->m_MemberOffset);
			result = DrawInt32(i, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Float)
		{
			float* f = ClassMemberProperty::AccessMember<float>(obj, member->m_MemberOffset);
			result = DrawFloat(f, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Bool)
		{
			bool* b = ClassMemberProperty::AccessMember<bool>(obj, member->m_MemberOffset);
			result = DrawBool(b, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Vec3)
		{
			Vec3* v = ClassMemberProperty::AccessMember<Vec3>(obj, member->m_MemberOffset);
			result = DrawVec3(v, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Vec4)
		{
			Vec4* v = ClassMemberProperty::AccessMember<Vec4>(obj, member->m_MemberOffset);
			result = DrawVec4(v, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::ObjectPtr)
		{
			const Class objClass = ((ObjectPtrProperty*)(member->m_Property.get()))->m_ObjectClass;
			if (objClass.Inherits(Asset::StaticClass()))
			{
				Asset** asset = ClassMemberProperty::AccessMember<Asset*>(obj, member->m_MemberOffset);
				result = DrawAsset(asset, objClass, mname, y, valueChangedBefore);
			}
		}
		else if (type == PropertyType::MaterialSlots)
		{
			MaterialSlots* materials = ClassMemberProperty::AccessMember<MaterialSlots>(obj, member->m_MemberOffset);
			result = DrawMaterialSlots(materials, y, valueChangedBefore);
		}
		else if (type == PropertyType::Class)
		{
			Class* cls = ClassMemberProperty::AccessMember<Class>(obj, member->m_MemberOffset);
			result = DrawClass(cls, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::SubclassOf)
		{
			TSubclassOf* cls = ClassMemberProperty::AccessMember<TSubclassOf>(obj, member->m_MemberOffset);
			result = DrawSubclassOf(cls, mname, y, valueChangedBefore);
		}
		else if (type == PropertyType::Delegate)
		{
			TDelegate* delegate = ClassMemberProperty::AccessMember<TDelegate>(obj, member->m_MemberOffset);
			result = DrawDelegate(delegate, mname, y);
		}
		else
		{
			SuoraError("{0}: Missing ClassMember implementation!", __FUNCTION__);
		}

		if (result == DetailsPanel::Result::ValueReset)
		{
			obj->ResetProperty(*member);
		}
		else if (result == DetailsPanel::Result::ValueChange)
		{
			if (!obj->m_OverwrittenProperties.Contains(mname))
			{
				obj->m_OverwrittenProperties.Add(mname);
			}
		}
	}

}