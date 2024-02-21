using System;

namespace Suora
{

    public enum UpdateFlag : int
    {
        NeverUpdate = 0,
        WorldUpdate = 1
    }
    
    public partial class Node : SuoraObject
    {

        internal static unsafe delegate*<IntPtr, int, void> InternalSetUpdateFlag;
        public void SetUpdateFlag(UpdateFlag flag)
        {
            unsafe { InternalSetUpdateFlag(SuoraObject.GetNativePtrFromManagedObject(this), (int)flag); }
        }
    }
	
    public partial class Node3D : Node
    {
        
    }
    
}