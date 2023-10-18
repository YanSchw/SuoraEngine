#pragma once

#include <string>
#include <functional>
#include "Suora/Core/Object/Object.h"
#include "Suora/Common/VectorUtils.h"
#include "Suora/Common/Array.h"
#include "Suora/Reflection/ClassReflector.h"
#include "Suora/Common/Common.h"
#include "Suora/Assets/AssetManager.h"
#include "Suora/Assets/SuoraProject.h"
#include "InputModule.generated.h"

namespace Yaml
{
	struct Node;
}
namespace Suora
{
	class InputCategory;
	class InputAction; 
	struct InputDispatcher;
	enum class InputActionType : uint32_t;
	
	extern void DrawInputDispatcherDropDown(Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height);

	/** For Project Settings */
	class InputSettings
	{
	public:
		Array<Ref<InputCategory>> m_Categories;
		void Serialize(Yaml::Node& root);
		void Deserialize(Yaml::Node& root);

		InputCategory* GetCategory(const std::string& label) const;
	};

	/** For categoring all InputActions */
	class InputCategory
	{
	public:
		std::string m_CategoryName = "New InputCategory";
		Array<Ref<InputAction>> m_Actions;

		Ref<InputAction> GetAction(const std::string& label) const;
	};

	enum class InputActionType : uint32_t
	{
		None,
		Action,     // bool
		Axis,		// float
		Axis2D		// Vec2
	};
	enum class InputActionParam : uint32_t
	{
		Pressed,
		Released,
		Held
	};

	class InputAction;
	struct InputActionDispatcher;

	/** InputBindings for an InputAction */
	class InputBinding : public Object
	{
		SUORA_CLASS(5486349);
	public:
		InputActionType m_TargetActionType = InputActionType::None;
		std::string m_Label = "Binding";
	protected:
		virtual bool GetActionValue() { return false; }
		virtual float GetAxisValue() { return 0.0f; }
		virtual Vec2 GetAxis2DValue() { return Vec2(0.0f); }

		virtual void Serialize(Yaml::Node& root) { }
		virtual void Deserialize(Yaml::Node& root) { }

		friend class InputSettings;
		friend class InputAction;
	};


	/** Unique InputAction */
	class InputAction
	{
	public:
		std::string m_ActionName = "Action";
		InputActionType m_ActionType = InputActionType::Action;
		Array<Ref<InputBinding>> m_Bindings;

		template<class T>
		T GetValue();
	};

	struct InputDispatcher
	{
		std::string m_Label = "Some Input";
		InputActionType m_ActionSpecifier = InputActionType::None;
		std::function<bool(void)> m_BoolLambda = []() { return false; };
		std::function<float(void)> m_FloatLambda = []() { return 0.0f; };
		std::function<Vec2(void)> m_Vec2Lambda = []() { return Vec2(0.0f); };
	};

	enum class InputScriptEventFlags : uint64_t
	{
		None = 0,
		ButtonPressed = 1,
		ButtonReleased = 2,
		ButtonHelt = 4
	};
	inline InputScriptEventFlags operator|(InputScriptEventFlags a, InputScriptEventFlags b)
	{
		return static_cast<InputScriptEventFlags>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b));
	}

	/** Runtime Binding of InputActions to Nodes */
	class InputModule
	{
		struct ObjectBinding
		{
			ObjectBinding(Object* obj)
				: m_Lifetime(obj)
			{
			}

			Ptr<Object> m_Lifetime;
			Array<Ref<InputAction>> m_ActionInput;
			std::vector<std::function<void(void)>> m_ActionFunctions;
			Array<InputActionParam> m_ActionEvent;
			std::vector<bool> m_ActionLastFrameInput;
			std::vector<bool> m_ActionCurrentFrameInput;

			Array<Ref<InputAction>> m_AxisInput;
			std::vector<std::function<void(float)>> m_AxisFunctions;

			Array<Ref<InputAction>> m_Axis2DInput;
			std::vector<std::function<void(const Vec2&)>> m_Axis2DFunctions;
		};
		struct BlueprintInstanceBinding : public ObjectBinding
		{
			BlueprintInstanceBinding(Object* node)
				: ObjectBinding(node)
			{
			}
			bool m_IsPawnOnly = false;
			std::vector<size_t> m_ActionFunctionScriptHashes;
			std::vector<size_t> m_AxisFunctionScriptHashes;
			std::vector<size_t> m_Axis2DFunctionScriptHashes;
		};
	public:
		InputModule();
		void Tick();
		void RegisterObject(Object* obj);
		bool IsObjectRegistered(Object* obj) const;
		void ProcessInputForObject(Object* obj);
		void RegisterBlueprintInstance(class Node* node, bool pawnOnly);
		void UnregisterBlueprintInstance(class Node* node);
		bool IsBlueprintInstanceRegistered(class Node* node) const;
		void ProcessInputForBlueprintInstance(class Node* node);

		template<typename R, typename T, typename U>
		void BindAction(const std::string& label, InputActionParam event, U instance, R(T::* f)(void))
		{
			if (!IsObjectRegistered(instance))
			{
				RegisterObject(instance);
			}

			/*std::vector<std::string> Category_Action = Util::SplitString(label, '/');
			if (Category_Action.size() != 2) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputCategory* Category = ProjectSettings::Get()->m_InputSettings->GetCategory(Category_Action[0]);
			if (!Category) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputAction* Action = Category->GetAction(Category_Action[1]);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }*/
			Ref<InputAction> Action = GetInputActionByLabel(label);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }

			m_ObjectBindings[instance]->m_ActionInput.Add(Action);
			m_ObjectBindings[instance]->m_ActionFunctions.push_back([instance, f]() { (instance->*f)(); });
			m_ObjectBindings[instance]->m_ActionEvent.Add(event);
			m_ObjectBindings[instance]->m_ActionLastFrameInput.push_back(false);
			m_ObjectBindings[instance]->m_ActionCurrentFrameInput.push_back(false);
		}
		template<typename R, typename T, typename U>
		void BindAxis(const std::string& label, U instance, R(T::* f)(float))
		{
			if (!IsObjectRegistered(instance))
			{
				RegisterObject(instance);
			}

			/*std::vector<std::string> Category_Action = Util::SplitString(label, '/');
			if (Category_Action.size() != 2) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputCategory* Category = ProjectSettings::Get()->m_InputSettings->GetCategory(Category_Action[0]);
			if (!Category) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputAction* Action = Category->GetAction(Category_Action[1]);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }*/
			Ref<InputAction> Action = GetInputActionByLabel(label);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }

			m_ObjectBindings[instance]->m_AxisInput.Add(Action);
			m_ObjectBindings[instance]->m_AxisFunctions.push_back([instance, f](float value) { (instance->*f)(value); });
		}
		template<typename R, typename T, typename U>
		void BindAxis2D(const std::string& label, U instance, R(T::* f)(const Vec2&))
		{
			if (!IsObjectRegistered(instance))
			{
				RegisterObject(instance);
			}

			/*std::vector<std::string> Category_Action = Util::SplitString(label, '/');
			if (Category_Action.size() != 2) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputCategory* Category = ProjectSettings::Get()->m_InputSettings->GetCategory(Category_Action[0]);
			if (!Category) { SuoraError("Could not bind InputAction: {0}", label); return; }
			InputAction* Action = Category->GetAction(Category_Action[1]);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }*/
			Ref<InputAction> Action = GetInputActionByLabel(label);
			if (!Action) { SuoraError("Could not bind InputAction: {0}", label); return; }

			m_ObjectBindings[instance]->m_Axis2DInput.Add(Action);
			m_ObjectBindings[instance]->m_Axis2DFunctions.push_back([instance, f](const Vec2& value) { (instance->*f)(value); });
		}

		void BindInputScriptEvent(class Node* node, const std::string& label, InputScriptEventFlags flags, size_t scriptFunctionHash);

		Ref<InputAction> GetInputActionByLabel(const std::string& label);
		static Ref<InputDispatcher> GetInputDispatcherByLabel(const std::string& label);
		static void AddInputDispatcher(const Ref<InputDispatcher>& dispatcher);
	public:
		bool m_LockInputCursor = false;
	private:
		std::unordered_map<Object*, Ref<ObjectBinding>> m_ObjectBindings;
		std::unordered_map<class Node*, Ref<BlueprintInstanceBinding>> m_BlueprintInstanceBindings;

		inline static std::unordered_map<std::string, Ref<InputDispatcher>> s_RegisteredInputDispatchers;
		friend void DrawInputDispatcherDropDown(Ref<InputDispatcher>& dispatcher, InputActionType type, float x, float y, float width, float height);
		friend class GameInstance;
	};


}

/** InputBinding Implementations */

namespace Suora
{

	class InputBinding_DirectAction : public InputBinding
	{
		SUORA_CLASS(1137898);
	public:
		InputBinding_DirectAction()
		{
			m_TargetActionType = InputActionType::Action;
		}

		bool GetActionValue() override
		{
			if (!m_Dispatcher) return Super::GetActionValue();

			SUORA_ASSERT(m_Dispatcher->m_ActionSpecifier == InputActionType::Action);
			bool value = m_Dispatcher->m_BoolLambda();

			return value;
		}

		void Serialize(Yaml::Node& root) override;
		void Deserialize(Yaml::Node& root) override;

		Ref<InputDispatcher> m_Dispatcher = nullptr;
	};

	class InputBinding_DirectAxis : public InputBinding
	{
		SUORA_CLASS(75438979);
	public:
		InputBinding_DirectAxis()
		{
			m_TargetActionType = InputActionType::Axis;
		}

		float GetAxisValue() override
		{
			if (!m_Dispatcher) return Super::GetAxisValue();

			SUORA_ASSERT(m_Dispatcher->m_ActionSpecifier == InputActionType::Axis);
			float value = m_Dispatcher->m_FloatLambda();

			return value;
		}

		void Serialize(Yaml::Node& root) override;
		void Deserialize(Yaml::Node& root) override;

		Ref<InputDispatcher> m_Dispatcher = nullptr;
	};

	class InputBinding_DirectAxis2D : public InputBinding
	{
		SUORA_CLASS(654739842);
	public:
		InputBinding_DirectAxis2D()
		{
			m_TargetActionType = InputActionType::Axis2D;
		}

		Vec2 GetAxis2DValue() override
		{
			if (!m_Dispatcher) return Super::GetAxis2DValue();

			SUORA_ASSERT(m_Dispatcher->m_ActionSpecifier == InputActionType::Axis2D);
			Vec2 value = m_Dispatcher->m_Vec2Lambda();

			return value;
		}

		void Serialize(Yaml::Node& root) override;
		void Deserialize(Yaml::Node& root) override;

		Ref<InputDispatcher> m_Dispatcher = nullptr;
	};

	class InputBinding_DirectionalActionMapping : public InputBinding
	{
		SUORA_CLASS(93789678);
	public:
		InputBinding_DirectionalActionMapping()
		{
			m_TargetActionType = InputActionType::Axis;
		}

		float GetAxisValue() override
		{
			if (!m_DispatcherPos && !m_DispatcherNeg) return Super::GetAxisValue();

			if (m_DispatcherPos) { SUORA_ASSERT(m_DispatcherPos->m_ActionSpecifier == InputActionType::Action); }
			if (m_DispatcherNeg) { SUORA_ASSERT(m_DispatcherNeg->m_ActionSpecifier == InputActionType::Action); }

			const bool POS = m_DispatcherPos->m_BoolLambda();
			const bool NEG = m_DispatcherNeg->m_BoolLambda();

			if ((POS && NEG) || (!POS && !NEG)) return 0.0f;
			if (POS && !NEG) return PosValue;
			if (!POS && NEG) return NegValue;

			return 0.0f;
		}

		void Serialize(Yaml::Node& root) override;
		void Deserialize(Yaml::Node& root) override;

		Ref<InputDispatcher> m_DispatcherPos = nullptr;
		Ref<InputDispatcher> m_DispatcherNeg = nullptr;

		float PosValue = 1.0f;
		float NegValue = -1.0f;
	};
	class InputBinding_BidirectionalActionMapping: public InputBinding
	{
		SUORA_CLASS(43855832);
	public:
		InputBinding_BidirectionalActionMapping()
		{
			m_TargetActionType = InputActionType::Axis2D;
		}

		Vec2 GetAxis2DValue() override
		{
			if (!m_DispatcherUp && !m_DispatcherDown && !m_DispatcherLeft && !m_DispatcherRight) return Super::GetAxis2DValue();

			if (m_DispatcherUp) { SUORA_ASSERT(m_DispatcherUp->m_ActionSpecifier == InputActionType::Action); }
			if (m_DispatcherDown) { SUORA_ASSERT(m_DispatcherDown->m_ActionSpecifier == InputActionType::Action); }
			if (m_DispatcherLeft) { SUORA_ASSERT(m_DispatcherLeft->m_ActionSpecifier == InputActionType::Action); }
			if (m_DispatcherRight) { SUORA_ASSERT(m_DispatcherRight->m_ActionSpecifier == InputActionType::Action); }
			
			Vec2 value = Vec2(0.0f);

			{
				const bool POS = m_DispatcherUp->m_BoolLambda();
				const bool NEG = m_DispatcherDown->m_BoolLambda();

				if ((POS && NEG) || (!POS && !NEG)) value.y = 0.0f;
				else if (POS && !NEG) value.y = 1.0f;
				else if (!POS && NEG) value.y = -1.0f;
			}
			{
				const bool POS = m_DispatcherRight->m_BoolLambda();
				const bool NEG = m_DispatcherLeft->m_BoolLambda();

				if ((POS && NEG) || (!POS && !NEG)) value.x = 0.0f;
				else if (POS && !NEG) value.x = 1.0f;
				else if (!POS && NEG) value.x = -1.0f;
			}

			if (m_Normalize) value = glm::normalize(value);

			return value;
		}

		void Serialize(Yaml::Node& root) override;
		void Deserialize(Yaml::Node& root) override;

		Ref<InputDispatcher> m_DispatcherUp = nullptr;
		Ref<InputDispatcher> m_DispatcherDown = nullptr;
		Ref<InputDispatcher> m_DispatcherLeft = nullptr;
		Ref<InputDispatcher> m_DispatcherRight = nullptr;

		bool m_Normalize = false;
	};

}