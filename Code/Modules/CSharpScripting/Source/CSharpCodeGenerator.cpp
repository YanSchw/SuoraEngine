#include "CSharpCodeGenerator.h"
#include "Suora/Platform/Platform.h"

namespace Suora
{

    static Map<NativeClassID, Array<NativeFunction*>> s_NativeIdToFunctionTable;

    static void PrepareNativeIdToFunctionTable()
    {
        s_NativeIdToFunctionTable.Clear();

        Array<NativeFunction*> funcs = NativeFunction::s_NativeFunctions;
        for (NativeFunction* It : funcs)
        {
            s_NativeIdToFunctionTable[It->m_ClassID].Add(It);
        }
    }

    void CSharpCodeGenerator::Generate_AllNativeClasses_CS(String& code)
    {
        PrepareNativeIdToFunctionTable();

        code = "using System;\nusing Suora;\n\nnamespace Suora\n{\n";

        Array<Class> allClasses = Class::GetAllNativeClasses();
        for (const Class& cls : allClasses)
        {
            if (!cls.IsNative())
                continue;
            if (cls.GetNativeClassID() == 1)
                continue;

            code += "\t[NativeSuoraClass(NativeID = " + std::to_string(cls.GetNativeClassID()) + ")]\n";

            String parentNativeClass = (cls.GetParentClass() == Object::StaticClass()) ? "Suora.SuoraObject" : "Suora." + cls.GetParentClass().GetClassName();
            code += "\tpublic partial class " + cls.GetClassName() + " : " + parentNativeClass + "\n\t{\n";

            CSharpCodeGenerator::GenerateManagedFunctions(code, cls);

            code += "\t}\n\n";

        }

        code += "}";
    }

    static String GeneratedFunctionArguments(NativeFunction* func)
    {
        String args = "";

        for (int i = 0; i < func->m_Params.size(); i++)
        {
            String type = func->m_Params[i].m_Type;
            StringUtil::ReplaceSequence(type, "const", "");
            StringUtil::ReplaceSequence(type, "&", "");
            StringUtil::ReplaceSequence(type, " ", "");
            if (type.ends_with("*"))
            {
                type.erase(type.size() - 1, 1);
            }

            args += type + " " + func->m_Params[i].m_Name;

            if (i != func->m_Params.size() - 1)
            {
                args += ", ";
            }
        }

        return args;
    }

    void CSharpCodeGenerator::GenerateManagedFunctions(String& code, const Class& cls)
    {
        Array<NativeFunction*> funcs = s_NativeIdToFunctionTable[cls.GetNativeClassID()];

        for (NativeFunction* It : funcs)
        {
            if (It->IsFlagSet(FunctionFlags::NodeEvent))
            {
                code += "\t\t// Generated Managed Event from NativeEvent: " + It->m_Label + "\n";
                const String funcName = StringUtil::SplitString(StringUtil::SplitString(It->m_Label, ':')[2], '(')[0];
                code += "\t\tpublic virtual void " + funcName + "(" + GeneratedFunctionArguments(It) + ") {}\n";

                code += "\t\tinternal static void InvokeManagedEvent_" + std::to_string(It->m_Hash) + "(IntPtr ptr)\n\t\t{\n";
                
                code += "\t\t\tScriptStack stack = ScriptStack.Get();\n";
                // Unpack Args

                for (int i = It->m_Params.size() - 1; i >= 0; i--)
                {
                    code += "\t\t\tvar arg" + std::to_string(i) + " = ";
                    switch (StringToScriptDataType(It->m_Params[i].m_Type))
                    {
                    case ScriptDataType::Int32: code += "stack.PopInt32()"; break;
                    case ScriptDataType::Float: code += "stack.PopFloat()"; break;
                    case ScriptDataType::Vec3: code += "stack.PopVec3()"; break;
                    case ScriptDataType::ObjectPtr: 
                    {
                        String type = String(It->m_Params[i].m_Type);
                        type.erase(type.size() - 1, 1);
                        code += "(" + type + ")(stack.PopObjectPtr())"; 
                        break;
                    }
                    case ScriptDataType::None:
                    default: SuoraError("Missing Implementation: {0}", It->m_Params[i].m_Type); break;
                    }
                    code += ";\n";
                }

                // Call ManagedEvent with Args
                String args = "";
                for (int i = 0; i < It->m_Params.size(); i++)
                {
                    args += "arg" + std::to_string(i);

                    if (i != It->m_Params.size() - 1)
                    {
                        args += ", ";
                    }
                }
                code += "\t\t\t((" + cls.GetClassName() + ")SuoraObject.GetManagedObjectFromNativePtr(ptr))." + funcName + "(" + args + ");\n";

                code += "\t\t}\n\n";
            }
            else
            {

            }
        }
    }

}