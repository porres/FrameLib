
#include "FrameLib_Uniform.h"
#include "FrameLib_MaxClass.h"

extern "C" int C74_EXPORT main(void)
{
    FrameLib_MaxClass_Expand<FrameLib_Uniform>::makeClass(CLASS_BOX, "fl.uniform~");
}