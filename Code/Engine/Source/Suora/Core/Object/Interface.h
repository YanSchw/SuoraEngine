#pragma once
#include "Object.h"
#include "Interface.generated.h"

namespace Suora
{

    class Object;

    /** Baseclass to implement runtime-composition of multiple SuoraClasses */
    class Interface : public Object
    {
        SUORA_CLASS(84953);
    public:
        Interface()
        {
        }
        virtual ~Interface()
        {
        }
        virtual void OnImplementation()
        {
        }
        Object* GetRootObject() const
        {
            return m_RootObject;
        }

    private:
        Object* m_RootObject = nullptr;

        friend class Object;
    };

}

#define INTERFACE_FUNCTION(FunctionName, ReturnValue, ...)
#define IMPLEMENT(...)