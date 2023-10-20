#pragma once
#include <unordered_map>
#include "Suora/Reflection/Class.h"

namespace Suora
{
    class Object;
    struct Class;

    /** Used internally by all reflected SuoraClasses */
	struct ObjectFactory
	{
        template<class T>
        struct RegisterSuoraClass
        {
            RegisterSuoraClass()
            {
                s_ObjectAllocators[T::StaticClass().GetNativeClassID()] = InternalAllocate<T>;
            }
            explicit RegisterSuoraClass(const NativeClassID id)
            {
                s_ObjectAllocators[id] = InternalAllocate<T>;
            }
        };


        static Object* Allocate(const NativeClassID id);

        static Array<Class> GetAllNativeClasses();

        ObjectFactory() = delete;
    private:

        inline static std::unordered_map<NativeClassID, Object * (*)(void)> s_ObjectAllocators;

        template<class T>
        static Object* InternalAllocate() 
        {
            return (Object*) new T();
        }

        friend struct Class;
	};
}


#define SUORA_REGISTER_CLASS(name, classid) \
    ::Suora::ObjectFactory::RegisterSuoraClass<name> register_class_##name(##classid); \

