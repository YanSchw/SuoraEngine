using System;
using Coral.Managed.Interop;

namespace Suora
{
	public class Debug
	{
		
		internal static unsafe delegate*<NativeString, void> LogInfo;
		internal static unsafe delegate*<NativeString, void> LogWarn;
		internal static unsafe delegate*<NativeString, void> LogError;
		
		public static void Info(string info)
		{
			NativeString str = info;
			unsafe { LogInfo(str); }
		}
		public static void Warn(string warn)
		{
			NativeString str = warn;
			unsafe { LogWarn(str); }
		}
		public static void Error(string error)
		{
			NativeString str = error;
			unsafe { LogError(str); }
		}
	}
}