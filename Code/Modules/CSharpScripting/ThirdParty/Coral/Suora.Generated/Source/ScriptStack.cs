using System;
using System.Collections.Generic;

namespace Suora
{
	internal enum ScriptStackType
	{
		Int32,
		Float,
		Vec3,
		ObjectPtr
	}
	
	internal class ScriptStack
	{
		internal static ScriptStack s_Slot = null;
		
		internal static void PrepareSlot()
		{
			s_Slot = new ScriptStack();
		}
		internal static ScriptStack Get()
		{
			ScriptStack slot = s_Slot;
			s_Slot = null;
			return slot;
		}
		private List<ScriptStackType> m_StackTypes = new List<ScriptStackType>();
		
		
		internal static unsafe delegate*<void> PrepareSlotInCPP;

		internal void SendToCPP()
		{
			unsafe { PrepareSlotInCPP(); }

			while (m_StackTypes.Count > 0)
			{
				switch (m_StackTypes[m_StackTypes.Count - 1])
				{
					case ScriptStackType.Int32: unsafe { SendInt32ToCPP(PopInt32()); } break;
					case ScriptStackType.Float: unsafe { SendFloatToCPP(PopFloat()); } break;
					case ScriptStackType.Vec3:
					{
						Vec3 vec = PopVec3();
						unsafe { SendVec3ToCPP(vec.X, vec.Y, vec.Z); } 
						break;
					}
					case ScriptStackType.ObjectPtr: unsafe { SendObjectPtrToCPP(SuoraObject.GetNativePtrFromManagedObject(PopObjectPtr())); } break;
					default: throw new Exception("ScriptStack.SendToCPP() missing Implementation!");
				}
			}
		}
		
		
		#region Int32
		internal static unsafe delegate*<Int32, void> SendInt32ToCPP;
		private List<Int32> m_StackInt32 = new List<Int32>();
		internal void PushInt32(Int32 item)
		{
			m_StackInt32.Add(item);
			m_StackTypes.Add(ScriptStackType.Int32);
		}
		internal Int32 PopInt32()
		{
			m_StackTypes.RemoveAt(m_StackTypes.Count - 1);
			Int32 value = m_StackInt32[m_StackInt32.Count - 1];
			m_StackInt32.RemoveAt(m_StackInt32.Count - 1);
			return value;
		}
		internal static void UploadInt32(Int32 item)
		{
			s_Slot.PushInt32(item);
		}
		#endregion
		
		#region Float
		internal static unsafe delegate*<float, void> SendFloatToCPP;
		private List<float> m_StackFloat = new List<float>();
		internal void PushFloat(float item)
		{
			m_StackFloat.Add(item);
			m_StackTypes.Add(ScriptStackType.Float);
		}
		internal float PopFloat()
		{
			m_StackTypes.RemoveAt(m_StackTypes.Count - 1);
			float value = m_StackFloat[m_StackFloat.Count - 1];
			m_StackFloat.RemoveAt(m_StackFloat.Count - 1);
			return value;
		}
		internal static void UploadFloat(float item)
		{
			s_Slot.PushFloat(item);
		}
		#endregion
		
		#region Vec3
		internal static unsafe delegate*<float, float, float, void> SendVec3ToCPP;
		private List<Vec3> m_StackVec3 = new List<Vec3>();
		internal void PushVec3(Vec3 item)
		{
			m_StackVec3.Add(item);
			m_StackTypes.Add(ScriptStackType.Vec3);
		}
		internal Vec3 PopVec3()
		{
			m_StackTypes.RemoveAt(m_StackTypes.Count - 1);
			Vec3 value = m_StackVec3[m_StackVec3.Count - 1];
			m_StackVec3.RemoveAt(m_StackVec3.Count - 1);
			return value;
		}
		internal static void UploadVec3(float x, float y, float z)
		{
			s_Slot.PushVec3(new Vec3(x, y, z));
		}
		#endregion
		
		#region ObjectPtr
		internal static unsafe delegate*<IntPtr, void> SendObjectPtrToCPP;
		private List<SuoraObject> m_StackObjectPtr = new List<SuoraObject>();
		internal void PushObjectPtr(SuoraObject item)
		{
			m_StackObjectPtr.Add(item);
			m_StackTypes.Add(ScriptStackType.ObjectPtr);
		}
		internal SuoraObject PopObjectPtr()
		{
			m_StackTypes.RemoveAt(m_StackTypes.Count - 1);
			SuoraObject value = m_StackObjectPtr[m_StackObjectPtr.Count - 1];
			m_StackObjectPtr.RemoveAt(m_StackObjectPtr.Count - 1);
			return value;
		}
		internal static void UploadObjectPtr(IntPtr ptr)
		{
			s_Slot.PushObjectPtr(SuoraObject.GetManagedObjectFromNativePtr(ptr));
		}
		#endregion
	}
	
}