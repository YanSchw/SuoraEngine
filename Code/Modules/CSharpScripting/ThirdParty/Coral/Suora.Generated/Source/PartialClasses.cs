using System;

namespace Suora
{

    public enum UpdateFlag : int
    {
        WorldUpdate = 1
    }
    
    public partial class Node : SuoraObject
    {

        internal static unsafe delegate*<IntPtr, int, void> InternalSetUpdateFlag;
        public void SetUpdateFlag(UpdateFlag flag)
        {
            unsafe { InternalSetUpdateFlag(SuoraObject.GetNativePtrFromManagedObject(this), (int)flag); }
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
	
    public partial class Node3D : Node
    {

        internal static unsafe delegate*<IntPtr, float, float, float, void> InternalSetPosition;
        public void SetPosition(Vec3 pos)
        {
            unsafe { InternalSetPosition(SuoraObject.GetNativePtrFromManagedObject(this), pos.X, pos.Y, pos.Z); }
        }
		
        internal static unsafe delegate*<IntPtr, float, float, float, void> InternalAddWorldOffset;
        public void AddWorldOffset(Vec3 offset)
        {
            unsafe { InternalAddWorldOffset(SuoraObject.GetNativePtrFromManagedObject(this), offset.X, offset.Y, offset.Z); }
        }
        internal static unsafe delegate*<IntPtr, float, float, float, void> InternalSetRotation;
        public void SetRotation(Vec3 rot)
        {
            unsafe { InternalSetRotation(SuoraObject.GetNativePtrFromManagedObject(this), rot.X, rot.Y, rot.Z); }
        }
        internal static unsafe delegate*<IntPtr, float, float, float, void> InternalRotateEuler;
        public void RotateEuler(Vec3 euler)
        {
            unsafe { InternalRotateEuler(SuoraObject.GetNativePtrFromManagedObject(this), euler.X, euler.Y, euler.Z); }
        }
        internal static unsafe delegate*<IntPtr, float, float, float, void> InternalSetScale;
        public void SetScale(Vec3 scale)
        {
            unsafe { InternalSetScale(SuoraObject.GetNativePtrFromManagedObject(this), scale.X, scale.Y, scale.Z); }
        }
        
    }
    
}