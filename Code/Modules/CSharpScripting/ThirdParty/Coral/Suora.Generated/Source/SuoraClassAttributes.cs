using System;

namespace Suora
{
	[AttributeUsage(AttributeTargets.Class, Inherited = true)]
	public sealed class SuoraClass : Attribute
	{
	}
	
	[AttributeUsage(AttributeTargets.Class, Inherited = false)]
	public sealed class NativeSuoraClass : Attribute
	{
		public Int64 NativeID;
	}
}