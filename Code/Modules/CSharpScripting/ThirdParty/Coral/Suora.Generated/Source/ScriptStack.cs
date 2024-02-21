using System;
using System.Collections.Generic;

namespace Suora
{
	internal enum ScriptStackType
	{
		Int32
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
		
	}
	
}