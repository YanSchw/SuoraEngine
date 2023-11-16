#include "Precompiled.h"
#include "InputModule.h"
#include "Suora/Core/NativeInput.h"
#include "Suora/Common/Common.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/NodeScript/ScriptStack.h"

namespace Suora
{

	template<>
	bool InputMapping::GetValue<bool>()
	{
		SUORA_ASSERT(m_ActionType == InputActionType::Action);

		for (auto& It : m_Bindings)
		{
			if (It->GetActionValue())
			{
				return true;
			}
		}

		return false;
	}
	template<>
	float InputMapping::GetValue<float>()
	{
		SUORA_ASSERT(m_ActionType == InputActionType::Axis);

		float result = 0.0f;

		for (auto& It : m_Bindings)
		{
			result += It->GetAxisValue();
		}

		return result;
	}
	template<>
	Vec2 InputMapping::GetValue<Vec2>()
	{
		SUORA_ASSERT(m_ActionType == InputActionType::Axis2D);

		Vec2 result = Vec2(0.0f);

		for (auto& It : m_Bindings)
		{
			result += It->GetAxis2DValue();
		}

		return result;
	}

	PlayerInputNode::PlayerInputNode()
	{
		SetUpdateFlag(UpdateFlag::WorldUpdate);
	}

	void PlayerInputNode::Tick()
	{
		{
			Array<Object*> toRemove;

			for (auto& It : m_ObjectBindings)
			{
				if (!IsObjectRegistered(It.first))
				{
					RegisterObject(It.first);
				}
				if (It.second->m_Lifetime)
				{
					for (int64_t i = It.second->m_ActionInput.Last(); i >= 0; i--)
					{
						It.second->m_ActionLastFrameInput[i] = It.second->m_ActionCurrentFrameInput[i];
						It.second->m_ActionCurrentFrameInput[i] = It.second->m_ActionInput[i]->GetValue<bool>();
					}
					ProcessInputForObject(It.first);
				}
				else
				{
					toRemove.Add(It.first);
				}
			}

			for (auto& It : toRemove)
			{
				m_ObjectBindings.Remove(It);
			}
		}
	}

	void PlayerInputNode::RegisterObject(Object* obj)
	{
		m_ObjectBindings[obj] = Ref<ObjectBinding>(new ObjectBinding(obj));
	}

	bool PlayerInputNode::IsObjectRegistered(Object* obj) const
	{
		return m_ObjectBindings.ContainsKey(obj);
	}

	void PlayerInputNode::ProcessInputForObject(Object* obj)
	{
		if (!IsObjectRegistered(obj))
		{
			RegisterObject(obj);
		}
		Ref<ObjectBinding>& Binding = m_ObjectBindings[obj];

		for (int32_t i = 0; i < Binding->m_ActionInput.Size(); i++)
		{
			if (Binding->m_ActionEvent[i] == InputActionKind::Pressed && !Binding->m_ActionLastFrameInput[i] && Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}
			else if (Binding->m_ActionEvent[i] == InputActionKind::Released && Binding->m_ActionLastFrameInput[i] && !Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}
			else if (Binding->m_ActionEvent[i] == InputActionKind::Repeat && Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}

			Binding->m_ActionLastFrameInput[i] = Binding->m_ActionCurrentFrameInput[i];
			Binding->m_ActionCurrentFrameInput[i] = Binding->m_ActionInput[i]->GetValue<bool>();
		}
		for (int32_t i = 0; i < Binding->m_AxisInput.Size(); i++)
		{
			Binding->m_AxisFunctions[i](Binding->m_AxisInput[i]->GetValue<float>());
		}
		for (int32_t i = 0; i < m_ObjectBindings[obj]->m_Axis2DInput.Size(); i++)
		{
			Binding->m_Axis2DFunctions[i](Binding->m_Axis2DInput[i]->GetValue<Vec2>());
		}
	}

	InputMapping* PlayerInputNode::GetInputMappingByLabel(const String& label)
	{
		Array<InputMapping*> Mappings = AssetManager::GetAssets<InputMapping>();

		for (InputMapping* Mapping : Mappings)
		{
			if (Mapping->m_Label == label)
			{
				return Mapping;
			}
		}

		return nullptr;
	}

	Ref<InputDispatcher> PlayerInputNode::GetInputDispatcherByLabel(const String& label)
	{
		if (s_RegisteredInputDispatchers.find(label) == s_RegisteredInputDispatchers.end())
		{
			return nullptr;
		}
		else
		{
			return s_RegisteredInputDispatchers[label];
		}
	}

	void PlayerInputNode::AddInputDispatcher(const Ref<InputDispatcher>& dispatcher)
	{
		SUORA_ASSERT(s_RegisteredInputDispatchers.find(dispatcher->m_Label) == s_RegisteredInputDispatchers.end());

		s_RegisteredInputDispatchers[dispatcher->m_Label] = dispatcher;
	}

	static bool s_KeyboardKeyPressResult = false;
	struct InputDispatcher_Initializer
	{
		InputDispatcher_Initializer()
		{
#define ADD_INPUTDISPATCHER_KEY(_key_)\
			Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());\
			_key_->m_Label = String("Keyboard/") + #_key_;\
			_key_->m_ActionSpecifier = InputActionType::Action;\
			_key_->m_BoolLambda = []()\
			{\
				return NativeInput::IsKeyPressed(Key::_key_);\
			};\
			PlayerInputNode::AddInputDispatcher(_key_)

			ADD_INPUTDISPATCHER_KEY(A);
			ADD_INPUTDISPATCHER_KEY(B);
			ADD_INPUTDISPATCHER_KEY(C);
			ADD_INPUTDISPATCHER_KEY(D);
			ADD_INPUTDISPATCHER_KEY(E);
			ADD_INPUTDISPATCHER_KEY(F);
			ADD_INPUTDISPATCHER_KEY(G);
			ADD_INPUTDISPATCHER_KEY(H);
			ADD_INPUTDISPATCHER_KEY(I);
			ADD_INPUTDISPATCHER_KEY(J);
			ADD_INPUTDISPATCHER_KEY(K);
			ADD_INPUTDISPATCHER_KEY(L);
			ADD_INPUTDISPATCHER_KEY(M);
			ADD_INPUTDISPATCHER_KEY(N);
			ADD_INPUTDISPATCHER_KEY(O);
			ADD_INPUTDISPATCHER_KEY(P);
			ADD_INPUTDISPATCHER_KEY(Q);
			ADD_INPUTDISPATCHER_KEY(R);
			ADD_INPUTDISPATCHER_KEY(S);
			ADD_INPUTDISPATCHER_KEY(T);
			ADD_INPUTDISPATCHER_KEY(U);
			ADD_INPUTDISPATCHER_KEY(V);
			ADD_INPUTDISPATCHER_KEY(W);
			ADD_INPUTDISPATCHER_KEY(X);
			ADD_INPUTDISPATCHER_KEY(Y);
			ADD_INPUTDISPATCHER_KEY(Z);

			ADD_INPUTDISPATCHER_KEY(Space);
			ADD_INPUTDISPATCHER_KEY(Up);
			ADD_INPUTDISPATCHER_KEY(Down);
			ADD_INPUTDISPATCHER_KEY(Left);
			ADD_INPUTDISPATCHER_KEY(Right);

			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse X";
				_key_->m_ActionSpecifier = InputActionType::Axis;
				_key_->m_FloatLambda = []()
					{
						return NativeInput::GetMouseX();
					}; 
				PlayerInputNode::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Y";
				_key_->m_ActionSpecifier = InputActionType::Axis;
				_key_->m_FloatLambda = []()
				{
					return NativeInput::GetMouseY();
				};
				PlayerInputNode::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Scroll Delta";
				_key_->m_ActionSpecifier = InputActionType::Axis;
				_key_->m_FloatLambda = []()
				{
					return NativeInput::GetMouseScrollDelta();
				};
				PlayerInputNode::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Position Delta";
				_key_->m_ActionSpecifier = InputActionType::Axis2D;
				_key_->m_Vec2Lambda = []()
				{
					return NativeInput::GetMouseDelta();
				};
				PlayerInputNode::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Left MouseButton";
				_key_->m_ActionSpecifier = InputActionType::Action;
				_key_->m_BoolLambda = []()
				{
					return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft);
				};
				PlayerInputNode::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Right MouseButton";
				_key_->m_ActionSpecifier = InputActionType::Action;
				_key_->m_BoolLambda = []()
				{
					return NativeInput::GetMouseButtonDown(Mouse::ButtonRight);
				};
				PlayerInputNode::AddInputDispatcher(_key_);
			}
		}
	};
	static InputDispatcher_Initializer _init;

	void InputBinding_DirectAction::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Dispatcher"] = m_Dispatcher ? m_Dispatcher->m_Label : "None";
	}
	void InputBinding_DirectAction::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_Dispatcher = PlayerInputNode::GetInputDispatcherByLabel(root["m_Dispatcher"].As<String>());
	}
	void InputBinding_DirectAxis::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Dispatcher"] = m_Dispatcher ? m_Dispatcher->m_Label : "None";
	}
	void InputBinding_DirectAxis::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_Dispatcher = PlayerInputNode::GetInputDispatcherByLabel(root["m_Dispatcher"].As<String>());
	}
	void InputBinding_DirectAxis2D::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Dispatcher"] = m_Dispatcher ? m_Dispatcher->m_Label : "None";
	}
	void InputBinding_DirectAxis2D::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_Dispatcher = PlayerInputNode::GetInputDispatcherByLabel(root["m_Dispatcher"].As<String>());
	}

	void InputBinding_DirectionalActionMapping::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_DispatcherPos"] = m_DispatcherPos ? m_DispatcherPos->m_Label : "None";
		root["m_DispatcherNeg"] = m_DispatcherNeg ? m_DispatcherNeg->m_Label : "None";
		root["PosValue"] = std::to_string(PosValue);
		root["NegValue"] = std::to_string(NegValue);
	}

	void InputBinding_DirectionalActionMapping::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_DispatcherPos = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherPos"].As<String>());
		m_DispatcherNeg = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherNeg"].As<String>());
		PosValue = std::stof(root["PosValue"].As<String>());
		NegValue = std::stof(root["NegValue"].As<String>());
	}

	void InputBinding_BidirectionalActionMapping::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_DispatcherUp"] = m_DispatcherUp ? m_DispatcherUp->m_Label : "None";
		root["m_DispatcherDown"] = m_DispatcherDown ? m_DispatcherDown->m_Label : "None";
		root["m_DispatcherLeft"] = m_DispatcherLeft ? m_DispatcherLeft->m_Label : "None";
		root["m_DispatcherRight"] = m_DispatcherRight ? m_DispatcherRight->m_Label : "None";
		root["m_Normalize"] = m_Normalize ? "true" : "false";
	}

	void InputBinding_BidirectionalActionMapping::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_DispatcherUp = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherUp"].As<String>());
		m_DispatcherDown = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherDown"].As<String>());
		m_DispatcherLeft = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherLeft"].As<String>());
		m_DispatcherRight = PlayerInputNode::GetInputDispatcherByLabel(root["m_DispatcherRight"].As<String>());
		m_Normalize = root["m_Normalize"].As<String>() == "true";
	}

	void InputMapping::PreInitializeAsset(const String& str)
	{
		Super::PreInitializeAsset(str);
	}

	void InputMapping::InitializeAsset(const String& str)
	{
		Super::InitializeAsset(str);

		Yaml::Node root;
		Yaml::Parse(root, str);

		m_Label = root["m_Label"].As<String>();

		int64_t i = 0;
		while (true)
		{
			Yaml::Node& binding = root["Binding_" + std::to_string(i++)];
			if (binding.IsNone()) break;

			Ref<InputBinding> Binding = Ref<InputBinding>(New(Class::FromString(binding["Class"].As<String>()))->As<InputBinding>());
			Binding->m_TargetActionType = (InputActionType)std::stoi(binding["m_TargetActionType"].As<String>());
			m_Bindings.Add(Binding);

			Binding->Deserialize(binding);
		}
	}

	void InputMapping::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Label"] = m_Label;

		int64_t i = 0;
		for (auto& Binding : m_Bindings)
		{
			Yaml::Node& binding = root["Binding_" + std::to_string(i++)];
			binding["Class"] = Binding->GetClass().ToString();
			binding["m_TargetActionType"] = std::to_string((uint8_t)Binding->m_TargetActionType);
			Binding->Serialize(binding);
		}
	}

	void PlayerInputNode::WorldUpdate(float deltaTime)
	{
		Super::WorldUpdate(deltaTime);

		Tick();

	}

}