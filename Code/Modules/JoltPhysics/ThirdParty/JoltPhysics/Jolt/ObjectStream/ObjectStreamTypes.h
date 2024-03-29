// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

// Note: Order is important, an enum is created and its value is stored in a binary stream!
JPH_DECLARE_PRIMITIVE(uint8)
JPH_DECLARE_PRIMITIVE(uint16)
JPH_DECLARE_PRIMITIVE(int)
JPH_DECLARE_PRIMITIVE(uint32)
JPH_DECLARE_PRIMITIVE(uint64)
JPH_DECLARE_PRIMITIVE(float)
JPH_DECLARE_PRIMITIVE(bool)
JPH_DECLARE_PRIMITIVE(JoltString)
JPH_DECLARE_PRIMITIVE(Float3)
JPH_DECLARE_PRIMITIVE(Vec3)
JPH_DECLARE_PRIMITIVE(Vec4)
JPH_DECLARE_PRIMITIVE(Quat)
JPH_DECLARE_PRIMITIVE(Mat44)
JPH_DECLARE_PRIMITIVE(double)
JPH_DECLARE_PRIMITIVE(DVec3)
JPH_DECLARE_PRIMITIVE(DMat44)
JPH_DECLARE_PRIMITIVE(Double3)

#undef JPH_DECLARE_PRIMITIVE
