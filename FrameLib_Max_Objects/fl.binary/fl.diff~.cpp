
#include "FrameLib_Binary_Objects.h"
#include "FrameLib_MaxClass.h"

extern "C" int C74_EXPORT main(void)
{
    FrameLib_MaxClass_Expand<FrameLib_Diff, true>::makeClass(CLASS_BOX, "fl.diff~");
}
