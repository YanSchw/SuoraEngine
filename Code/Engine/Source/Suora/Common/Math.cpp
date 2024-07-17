#include "Precompiled.h"
#include "Math.h"

namespace Suora
{
    
    // Source: https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
    static Vec3 rgb2hsv(Vec3 in)
    {
        Vec3         out;
        double      min, max, delta;

        min = in.r < in.g ? in.r : in.g;
        min = min < in.b ? min : in.b;

        max = in.r > in.g ? in.r : in.g;
        max = max > in.b ? max : in.b;

        out.z = max;                                // v
        delta = max - min;
        if (delta < 0.00001)
        {
            out.y = 0;
            out.x = 0; // undefined, maybe nan?
            return out;
        }
        if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
            out.y = (delta / max);                  // s
        }
        else {
            // if max is 0, then r = g = b = 0              
            // s = 0, h is undefined
            out.y = 0.0;
            out.x = NAN;                            // its now undefined
            return out;
        }
        if (in.r >= max)                           // > is bogus, just keeps compilor happy
            out.x = (in.g - in.b) / delta;        // between yellow & magenta
        else
            if (in.g >= max)
                out.x = 2.0 + (in.b - in.r) / delta;  // between cyan & yellow
            else
                out.x = 4.0 + (in.r - in.g) / delta;  // between magenta & cyan

        out.x *= 60.0;                              // degrees

        if (out.x < 0.0)
            out.x += 360.0;

        return out;
    }

    static Vec3 hsv2rgb(Vec3 in)
    {
        double      hh, p, q, t, ff;
        long        i;
        Vec3         out;

        if (in.y <= 0.0) {       // < is bogus, just shuts up warnings
            out.r = in.z;
            out.g = in.z;
            out.b = in.z;
            return out;
        }
        hh = in.x;
        if (hh >= 360.0) hh = 0.0;
        hh /= 60.0;
        i = (long)hh;
        ff = hh - i;
        p = in.z * (1.0 - in.y);
        q = in.z * (1.0 - (in.y * ff));
        t = in.z * (1.0 - (in.y * (1.0 - ff)));

        switch (i) {
        case 0:
            out.r = in.z;
            out.g = t;
            out.b = p;
            break;
        case 1:
            out.r = q;
            out.g = in.z;
            out.b = p;
            break;
        case 2:
            out.r = p;
            out.g = in.z;
            out.b = t;
            break;

        case 3:
            out.r = p;
            out.g = q;
            out.b = in.z;
            break;
        case 4:
            out.r = t;
            out.g = p;
            out.b = in.z;
            break;
        case 5:
        default:
            out.r = in.z;
            out.g = p;
            out.b = q;
            break;
        }
        return out;
    }


    Color Math::ConvertRGB2HSV(const Color& InRGB)
    {
        return Color(rgb2hsv(Vec3(InRGB)), InRGB.a);
    }
    Color Math::ConvertHSV2RGB(const Color& InHSV)
    {
        return Color(hsv2rgb(Vec3(InHSV)), InHSV.a);
    }

}