using System;

namespace Suora
{

    public enum UpdateFlag : int
    {
        WorldUpdate = 1
    }
    
    public partial class Node : SuoraObject
    {

        internal static unsafe delegate*<int, void> InternalSetUpdateFlag;
        public void SetUpdateFlag(UpdateFlag flag)
        {
            unsafe { InternalSetUpdateFlag((int)flag); }
        }
        
        public virtual void Begin() { }
        public virtual void OnNodeDestroy() { }
        public virtual void WorldUpdate(float deltaTime) { }

        internal static void InvokeManagedEvent_Begin(IntPtr ptr)
        {
            ((Node)SuoraObject.GetManagedObjectFromNativePtr(ptr)).Begin();
        }
        internal static void InvokeManagedEvent_OnNodeDestroy(IntPtr ptr)
        {
            ((Node)SuoraObject.GetManagedObjectFromNativePtr(ptr)).OnNodeDestroy();
        }
        internal static void InvokeManagedEvent_WorldUpdate(IntPtr ptr, float deltaTime)
        {
            ((Node)SuoraObject.GetManagedObjectFromNativePtr(ptr)).WorldUpdate(deltaTime);
        }
    }
    
}