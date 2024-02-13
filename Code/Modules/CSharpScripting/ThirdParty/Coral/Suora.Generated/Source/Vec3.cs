using System;

namespace Suora
{
	public struct Vec3
	{
		public float X, Y, Z;

		public static Vec3 Zero => new Vec3(0.0f);

		public Vec3(float scalar)
		{
			X = scalar;
			Y = scalar;
			Z = scalar;
		}

		public Vec3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		public static Vec3 operator +(Vec3 a, Vec3 b)
		{
			return new Vec3(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
		}

		public static Vec3 operator *(Vec3 vector, float scalar)
		{
			return new Vec3(vector.X * scalar, vector.Y * scalar, vector.Z * scalar);
		}

	}
}