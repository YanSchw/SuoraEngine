using System;
using System.Collections.Generic;

namespace Suora
{
	[SuoraClass]
	[NativeSuoraClass(NativeID = 1)]
	public class SuoraObject
	{
		
		public SuoraObject()
		{
			if (s_NativePtr == IntPtr.Zero)
			{
				return;
			}
			
			s_NativePtrToManagedObject[s_NativePtr] = this;
			s_ManagedObjectToNativePtr[this] = s_NativePtr;
			s_NativePtr = IntPtr.Zero;
		}
		
		internal static Dictionary<IntPtr, SuoraObject> s_NativePtrToManagedObject = new Dictionary<IntPtr, SuoraObject>();
		internal static Dictionary<SuoraObject, IntPtr> s_ManagedObjectToNativePtr = new Dictionary<SuoraObject, IntPtr>();
		
		internal static IntPtr s_NativePtr = IntPtr.Zero;
		
		internal static void CreateSuoraObject(string type, IntPtr ptr)
		{
			s_NativePtr = ptr;
			object obj = Activator.CreateInstance(Type.GetType(type));
		}
		internal static void DestroySuoraObject(IntPtr ptr)
		{
			s_ManagedObjectToNativePtr.Remove(s_NativePtrToManagedObject[ptr]);
			s_NativePtrToManagedObject.Remove(ptr);
		}

		public static SuoraObject GetManagedObjectFromNativePtr(IntPtr ptr)
		{
			return s_NativePtrToManagedObject.GetValueOrDefault(ptr, null);
		}

		public static IntPtr GetNativePtrFromManagedObject(SuoraObject obj)
		{
			return s_ManagedObjectToNativePtr.GetValueOrDefault(obj, IntPtr.Zero);
		}
		
	
		internal static unsafe delegate*<UInt64, void> s_CallNativeFunction;
		internal static void CallNativeFunction(UInt64 hash)
		{
			unsafe { s_CallNativeFunction(hash); }
		}
		
	}
	
}