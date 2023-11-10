#include "Precompiled.h"
#include "InputModule.h"
#include "Suora/Core/NativeInput.h"
#include "Suora/Common/Common.h"
#include "Suora/Serialization/Yaml.h"
#include "Suora/GameFramework/Node.h"
#include "Suora/NodeScript/ScriptStack.h"

namespace Suora
{

	void InputSettings::Serialize(Yaml::Node& root)
	{
		root = Yaml::Node();
		int64_t i = 0;
		for (auto& Category : m_Categories)
		{
			Yaml::Node& cat = root["Category_" + std::to_string(i++)];
			cat["m_CategoryName"] = Category->m_CategoryName;

			int64_t j = 0;
			for (auto& Action : Category->m_Actions)
			{
				Yaml::Node& action = cat["Action_" + std::to_string(j++)];
				action["m_ActionName"] = Action->m_ActionName;
				action["m_ActionType"] = std::to_string((uint8_t)Action->m_ActionType);

				int64_t k = 0;
				for (auto& Binding : Action->m_Bindings)
				{
					Yaml::Node& binding = action["Binding_" + std::to_string(k++)];
					binding["Class"] = Binding->GetClass().ToString();
					binding["m_Label"] = Binding->m_Label;
					binding["m_TargetActionType"] = std::to_string((uint8_t)Binding->m_TargetActionType);
					Binding->Serialize(binding);
				}
			}
		}
	}
	
	void InputSettings::Deserialize(Yaml::Node& root)
	{
		int64_t i = 0;
		while (true)
		{
			Yaml::Node& cat = root["Category_" + std::to_string(i++)];
			if (cat.IsNone()) break;
			Ref<InputCategory> Category = Ref<InputCategory>(new InputCategory());
			Category->m_CategoryName = cat["m_CategoryName"].As<std::string>();
			m_Categories.Add(Category);

			int64_t j = 0;
			while (true)
			{
				Yaml::Node& action = cat["Action_" + std::to_string(j++)];
				if (action.IsNone()) break;
				Ref<InputAction> Action = Ref<InputAction>(new InputAction());
				Action->m_ActionName = action["m_ActionName"].As<std::string>();
				Action->m_ActionType = (InputActionType)std::stoi(action["m_ActionType"].As<std::string>());
				Category->m_Actions.Add(Action);

				int64_t k = 0;
				while (true)
				{
					Yaml::Node& binding = action["Binding_" + std::to_string(k++)];
					if (binding.IsNone()) break;
					Ref<InputBinding> Binding = Ref<InputBinding>(New(Class::FromString(binding["Class"].As<std::string>()))->As<InputBinding>());
					Binding->m_Label = binding["m_Label"].As<std::string>();
					Binding->m_TargetActionType = (InputActionType)std::stoi(binding["m_TargetActionType"].As<std::string>());
					Action->m_Bindings.Add(Binding);

					Binding->Deserialize(binding);
				}
			}
		}
	}

	InputCategory* InputSettings::GetCategory(const std::string& label) const
	{
		for (const auto& It : m_Categories)
		{
			if (It->m_CategoryName == label)
			{
				return It.get();
			}
		}

		return nullptr;
	}

	template<>
	bool InputAction::GetValue<bool>()
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
	float InputAction::GetValue<float>()
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
	Vec2 InputAction::GetValue<Vec2>()
	{
		SUORA_ASSERT(m_ActionType == InputActionType::Axis2D);

		Vec2 result = Vec2(0.0f);

		for (auto& It : m_Bindings)
		{
			result += It->GetAxis2DValue();
		}

		return result;
	}

	InputModule::InputModule()
	{
		
	}

	void InputModule::Tick()
	{
		{
			std::vector<Object*> toRemove;

			for (auto& It : m_ObjectBindings)
			{
				if (!It.second->m_Lifetime)
				{
					toRemove.push_back(It.first);
				}
				else
				{
					for (int64_t i = It.second->m_ActionInput.Last(); i >= 0; i--)
					{
						/*It.second->m_ActionLastFrameInput[i] = It.second->m_ActionCurrentFrameInput[i];
						It.second->m_ActionCurrentFrameInput[i] = It.second->m_ActionInput[i]->GetValue<bool>();*/
					}
				}
			}
			for (auto& It : toRemove)
			{
				m_ObjectBindings.erase(It);
			}
		}
		{
			std::vector<Node*> toRemove;

			for (auto& It : m_BlueprintInstanceBindings)
			{
				if (!It.second->m_Lifetime)
				{
					toRemove.push_back(It.first);
				}
				else
				{
					for (int64_t i = It.second->m_ActionInput.Last(); i >= 0; i--)
					{
						It.second->m_ActionLastFrameInput[i] = It.second->m_ActionCurrentFrameInput[i];
						It.second->m_ActionCurrentFrameInput[i] = It.second->m_ActionInput[i]->GetValue<bool>();
					}
				}
			}
			for (auto& It : toRemove)
			{
				m_BlueprintInstanceBindings.erase(It);
			}
		}
	}

	void InputModule::RegisterObject(Object* obj)
	{
		m_ObjectBindings[obj] = Ref<ObjectBinding>(new ObjectBinding(obj));
	}

	bool InputModule::IsObjectRegistered(Object* obj) const
	{
		return m_ObjectBindings.find(obj) != m_ObjectBindings.end();
	}

	void InputModule::ProcessInputForObject(Object* obj)
	{
		Ref<ObjectBinding> Binding = m_ObjectBindings[obj];

		for (int32_t i = 0; i < Binding->m_ActionInput.Size(); i++)
		{
			Binding->m_ActionLastFrameInput[i] = Binding->m_ActionCurrentFrameInput[i];
			Binding->m_ActionCurrentFrameInput[i] = Binding->m_ActionInput[i]->GetValue<bool>();
			
			if (Binding->m_ActionEvent[i] == InputActionParam::Pressed && !Binding->m_ActionLastFrameInput[i] && Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}
			else if (Binding->m_ActionEvent[i] == InputActionParam::Released && Binding->m_ActionLastFrameInput[i] && !Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}
			else if (Binding->m_ActionEvent[i] == InputActionParam::Held && Binding->m_ActionCurrentFrameInput[i])
			{
				Binding->m_ActionFunctions[i]();
			}
		}
		for (int32_t i = 0; i < Binding->m_AxisInput.Size(); i++)
		{
			Binding->m_AxisFunctions[i](Binding->m_AxisInput[i]->GetValue<float>());
		}
		for (int32_t i = 0; i < Binding->m_Axis2DInput.Size(); i++)
		{
			Binding->m_Axis2DFunctions[i](Binding->m_Axis2DInput[i]->GetValue<Vec2>());
		}
	}

	void InputModule::RegisterBlueprintInstance(Node* node, bool pawnOnly)
	{
		m_BlueprintInstanceBindings[node] = Ref<BlueprintInstanceBinding>(new BlueprintInstanceBinding(node));
		m_BlueprintInstanceBindings[node]->m_IsPawnOnly = pawnOnly;
	}

	void InputModule::UnregisterBlueprintInstance(Node* node)
	{
		m_BlueprintInstanceBindings.erase(node);
	}

	bool InputModule::IsBlueprintInstanceRegistered(Node* node) const
	{
		return m_BlueprintInstanceBindings.find(node) != m_BlueprintInstanceBindings.end();
	}

	void InputModule::ProcessInputForBlueprintInstance(Node* node)
	{
		Ref<BlueprintInstanceBinding> Binding = m_BlueprintInstanceBindings[node];

		if (Binding->m_IsPawnOnly && !node->IsPossessed()) return;

		for (int32_t i = 0; i < Binding->m_ActionInput.Size(); i++)
		{
			if (Binding->m_ActionEvent[i] == InputActionParam::Pressed && !Binding->m_ActionLastFrameInput[i] && Binding->m_ActionCurrentFrameInput[i])
			{
				node->__NodeEventDispatch(Binding->m_ActionFunctionScriptHashes[i]);
			}
			else if (Binding->m_ActionEvent[i] == InputActionParam::Released && Binding->m_ActionLastFrameInput[i] && !Binding->m_ActionCurrentFrameInput[i])
			{
				node->__NodeEventDispatch(Binding->m_ActionFunctionScriptHashes[i]);
			}
			else if (Binding->m_ActionEvent[i] == InputActionParam::Held && Binding->m_ActionCurrentFrameInput[i])
			{
				node->__NodeEventDispatch(Binding->m_ActionFunctionScriptHashes[i]);
			}
		}
		for (int32_t i = 0; i < Binding->m_AxisInput.Size(); i++)
		{
			ScriptStack stack;
			stack.ConvertToStack<float>(Binding->m_AxisInput[i]->GetValue<float>());
			node->__NodeEventDispatch(Binding->m_AxisFunctionScriptHashes[i], stack);
		}
		for (int32_t i = 0; i < Binding->m_Axis2DInput.Size(); i++)
		{
			ScriptStack stack;
			stack.ConvertToStack<Vec2>(Binding->m_AxisInput[i]->GetValue<Vec2>());
			node->__NodeEventDispatch(Binding->m_Axis2DFunctionScriptHashes[i], stack);
		}
	}

	void InputModule::BindInputScriptEvent(Node* node, const std::string& label, InputScriptEventFlags flags, size_t scriptFunctionHash)
	{
		if (!IsBlueprintInstanceRegistered(node))
		{
			SuoraVerify(false, "Blueprint Instance has to be registered first!");
		}
		Ref<InputAction> Action = GetInputActionByLabel(label);

		if (!Action) { SuoraError("Could not bind BlueprintInputEvent to InputAction '{0}'", label); return; }

		if (Action->m_ActionType == InputActionType::Action)
		{
			InputActionParam event = InputActionParam::Pressed;

			if (((uint64_t)flags & (uint64_t)InputScriptEventFlags::ButtonPressed) != 0) event = InputActionParam::Pressed;
			if (((uint64_t)flags & (uint64_t)InputScriptEventFlags::ButtonReleased) != 0) event = InputActionParam::Released;
			if (((uint64_t)flags & (uint64_t)InputScriptEventFlags::ButtonHelt) != 0) event = InputActionParam::Held;

			m_BlueprintInstanceBindings[node]->m_ActionInput.Add(Action);
			m_BlueprintInstanceBindings[node]->m_ActionFunctions.push_back(nullptr); // Just to keep everything inline. Not used for Blueprints
			m_BlueprintInstanceBindings[node]->m_ActionFunctionScriptHashes.push_back(scriptFunctionHash);
			m_BlueprintInstanceBindings[node]->m_ActionEvent.Add(event);
			m_BlueprintInstanceBindings[node]->m_ActionLastFrameInput.push_back(false);
			m_BlueprintInstanceBindings[node]->m_ActionCurrentFrameInput.push_back(false);
		}
		else
		{
			SuoraVerify(false, "Missing Implementation");
		}
		
	}

	Ref<InputAction> InputModule::GetInputActionByLabel(const std::string& label)
	{
		std::vector<std::string> Category_Action = StringUtil::SplitString(label, '/');
		if (Category_Action.size() != 2) { return nullptr; }
		InputCategory* Category = ProjectSettings::Get()->m_InputSettings->GetCategory(Category_Action[0]);
		if (!Category) { return nullptr; }
		Ref<InputAction> Action = Category->GetAction(Category_Action[1]);

		return Action;
	}

	Ref<InputDispatcher> InputModule::GetInputDispatcherByLabel(const std::string& label)
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

	void InputModule::AddInputDispatcher(const Ref<InputDispatcher>& dispatcher)
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
			_key_->m_Label = std::string("Keyboard/") + #_key_;\
			_key_->m_ActionSpecifier = InputActionType::Action;\
			_key_->m_BoolLambda = []()\
			{\
				return NativeInput::GetKey(Key::_key_);\
			};\
			InputModule::AddInputDispatcher(_key_)

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
				InputModule::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Y";
				_key_->m_ActionSpecifier = InputActionType::Axis;
				_key_->m_FloatLambda = []()
				{
					return NativeInput::GetMouseY();
				};
				InputModule::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Scroll Delta";
				_key_->m_ActionSpecifier = InputActionType::Axis;
				_key_->m_FloatLambda = []()
				{
					return NativeInput::GetMouseScrollDelta();
				};
				InputModule::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Mouse Position Delta";
				_key_->m_ActionSpecifier = InputActionType::Axis2D;
				_key_->m_Vec2Lambda = []()
				{
					return NativeInput::GetMouseDelta();
				};
				InputModule::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Left MouseButton";
				_key_->m_ActionSpecifier = InputActionType::Action;
				_key_->m_BoolLambda = []()
				{
					return NativeInput::GetMouseButtonDown(Mouse::ButtonLeft);
				};
				InputModule::AddInputDispatcher(_key_);
			}
			{
				Ref<InputDispatcher> _key_ = Ref<InputDispatcher>(new InputDispatcher());
				_key_->m_Label = "Mouse/Right MouseButton";
				_key_->m_ActionSpecifier = InputActionType::Action;
				_key_->m_BoolLambda = []()
				{
					return NativeInput::GetMouseButtonDown(Mouse::ButtonRight);
				};
				InputModule::AddInputDispatcher(_key_);
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

		m_Dispatcher = InputModule::GetInputDispatcherByLabel(root["m_Dispatcher"].As<std::string>());
	}
	void InputBinding_DirectAxis::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Dispatcher"] = m_Dispatcher ? m_Dispatcher->m_Label : "None";
	}
	void InputBinding_DirectAxis::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_Dispatcher = InputModule::GetInputDispatcherByLabel(root["m_Dispatcher"].As<std::string>());
	}
	void InputBinding_DirectAxis2D::Serialize(Yaml::Node& root)
	{
		Super::Serialize(root);

		root["m_Dispatcher"] = m_Dispatcher ? m_Dispatcher->m_Label : "None";
	}
	void InputBinding_DirectAxis2D::Deserialize(Yaml::Node& root)
	{
		Super::Deserialize(root);

		m_Dispatcher = InputModule::GetInputDispatcherByLabel(root["m_Dispatcher"].As<std::string>());
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

		m_DispatcherPos = InputModule::GetInputDispatcherByLabel(root["m_DispatcherPos"].As<std::string>());
		m_DispatcherNeg = InputModule::GetInputDispatcherByLabel(root["m_DispatcherNeg"].As<std::string>());
		PosValue = std::stof(root["PosValue"].As<std::string>());
		NegValue = std::stof(root["NegValue"].As<std::string>());
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

		m_DispatcherUp = InputModule::GetInputDispatcherByLabel(root["m_DispatcherUp"].As<std::string>());
		m_DispatcherDown = InputModule::GetInputDispatcherByLabel(root["m_DispatcherDown"].As<std::string>());
		m_DispatcherLeft = InputModule::GetInputDispatcherByLabel(root["m_DispatcherLeft"].As<std::string>());
		m_DispatcherRight = InputModule::GetInputDispatcherByLabel(root["m_DispatcherRight"].As<std::string>());
		m_Normalize = root["m_Normalize"].As<std::string>() == "true";
	}

	Ref<InputAction> InputCategory::GetAction(const std::string& label) const
	{
		for (const auto& It : m_Actions)
		{
			if (It->m_ActionName == label)
			{
				return It;
			}
		}

		return nullptr;
	}

}