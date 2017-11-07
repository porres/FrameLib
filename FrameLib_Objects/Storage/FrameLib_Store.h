
#ifndef FRAMELIB_STORE_H
#define FRAMELIB_STORE_H

#include "FrameLib_DSP.h"

class FrameLib_Store : public FrameLib_Processor
{
    // Parameter Enums and Info

	enum ParameterList { kName };
    
    struct ParameterInfo : public FrameLib_Parameters::Info { ParameterInfo(); };

public:
	
    // Constructor / Destructor

    FrameLib_Store(FrameLib_Context context, FrameLib_Parameters::Serial *serialisedParameters, void *owner);
    ~FrameLib_Store();
    
    // Info
    
    std::string objectInfo(bool verbose);
    std::string inputInfo(unsigned long idx, bool verbose);
    std::string outputInfo(unsigned long idx, bool verbose);
    
    // Stream Awareness
    
    virtual void setStream(unsigned long stream);
    
private:
    
    // Object Reset and Process
    
    void objectReset();
    void process();
    
    // Data
    
    FrameLib_LocalAllocator::Storage *mStorage;
    
    static ParameterInfo sParamInfo;
    
    std::string mName;
};

#endif
