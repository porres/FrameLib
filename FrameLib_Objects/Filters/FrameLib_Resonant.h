
#ifndef FRAMELIB_RESONANT_H
#define FRAMELIB_RESONANT_H

#include "FrameLib_Filter_Template.h"

class Resonant : public FrameLib_FilterBase<Resonant, 2, 1>
{
    
public:
    
    Resonant() : scl(0.0), r2(0.0), y1(0.0), y2(0.0) {}
    
    // Filter Implementation

    double lpf(double x);

    void reset();
    
    void updateCoefficients(double freq, double reson, double samplingRate);
    
    // Parameters / Modes
    
    constexpr static ParamType sParameters
    {{
        Param("freq", 500.0, Min(0.0)),
        Param("reson", 0.0, Clip(0.0, 1.0))
    }};
    
    constexpr static ModeType sModes
    {{
        Mode("lpf", &Resonant::lpf)
    }};
    
private:
    
    // Coefficients / Memories
    
    double scl, r2;
    double y1, y2;
};

using FrameLib_Resonant = FrameLib_Filter<Resonant>;

#endif
