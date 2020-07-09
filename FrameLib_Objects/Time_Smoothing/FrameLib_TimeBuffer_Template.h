
#ifndef FRAMELIB_TIMEBUFFER_TEMPLATE_H
#define FRAMELIB_TIMEBUFFER_TEMPLATE_H

#include "FrameLib_RingBuffer.h"
#include "FrameLib_PaddedVector.h"
#include "FrameLib_DSP.h"

template <class T, unsigned long nParams = 0>
class FrameLib_TimeBuffer : public FrameLib_Processor, private FrameLib_RingBuffer
{
    
protected:
    
    enum ParameterList
    {
        kMaxFrames = 0,
        kNumFrames = 1,
        kDefault = 2 + nParams,
        kMode = 3 + nParams
    };
    
    enum Modes { kPadIn, kValid };
    
    // Parameter Info

    struct ParameterInfo : public FrameLib_Parameters::Info
    {
        ParameterInfo()
        {
            add("Sets the maximum number of frames that can be set as a time period - changing resets the buffer.");
            add("Sets the current integer number of frames for calculation.");
        }
    };
    
    void completeDefaultParameters(const FrameLib_Parameters::Serial *serialisedParameters)
    {
        mParameters.addDouble(kDefault, "default", 0.0, kDefault);
        
        mParameters.addEnum(kMode, "mode");
        mParameters.addEnumItem(kPadIn, "pad_in");
        mParameters.addEnumItem(kValid, "valid");
        
        mParameters.set(serialisedParameters);
    }
    
public:
    
    // Constructor
    
    FrameLib_TimeBuffer(FrameLib_Context context, const FrameLib_Parameters::Serial *serialisedParameters, FrameLib_Proxy *proxy) : FrameLib_Processor(context, proxy, &sParamInfo, 3, 2), FrameLib_RingBuffer(this), mLastNumFrames(0)
    {
        mParameters.addInt(kMaxFrames, "max_frames", 10, 0);
        mParameters.setMin(1);
    
        mParameters.addInt(kNumFrames, "num_frames", 10, 1);
        mParameters.setMin(1);
        
        if (!nParams)
            completeDefaultParameters(serialisedParameters);
        
        setParameterInput(2);
    }
    
protected:
    
    unsigned long getMaxFrames() const  { return mParameters.getInt(kMaxFrames); }

    unsigned long getNumFrames(bool forceValid = false) const
    {
        bool valid = forceValid || mParameters.getInt(kMode) == kValid;
        
        if (valid)
            return std::min(getRequestedNumFrames(), getValidFrames() + 1);
        else
            return getRequestedNumFrames();
    }
    
private:
    
    unsigned long getRequestedNumFrames() const
    {
        unsigned long maxFrames = getMaxFrames();
        unsigned long numFrames = mParameters.getInt(kNumFrames);
        
        return std::min(maxFrames, numFrames);
    }
    
    // Smooth
    
    virtual void add(const double *newFrame, unsigned long size) = 0;
    virtual void remove(const double *oldFrame, unsigned long size) = 0;

    virtual void exchange(const double *newFrame, const double *oldFrame, unsigned long size)
    {
        remove(oldFrame, size);
        add(newFrame, size);
    }
    
    virtual void result(double *output, unsigned long size, const PaddedVector& pad, unsigned long padSize) = 0;
    
    virtual void resetSize(unsigned long maxFrames, unsigned long size) = 0;
    
    // Object reset
    
    void objectReset() override
    {
        resize(0, 0);
        mLastNumFrames = 0;
        mLastResetTime = FrameLib_TimeFormat(0);
    }
    
    // Process
    
    void process() override
    {
        Modes mode = static_cast<Modes>(mParameters.getInt(kMode));
        double pad = mParameters.getValue(kDefault);
        
        unsigned long sizeIn, sizeReset, sizeOut, sizeValid;
        unsigned long maxFrames = getMaxFrames();
        unsigned long requestedFrames = getRequestedNumFrames();
        unsigned long numFrames;
        
        const double *input = getInput(0, &sizeIn);
        const double *resetInput = getInput(1, &sizeReset);

        bool forceReset = mLastResetTime != getInputFrameTime(1);

        if (forceReset || getFrameLength() != sizeIn || FrameLib_RingBuffer::getNumFrames() != maxFrames)
        {
            resize(maxFrames, sizeIn);
            resetSize(maxFrames, sizeIn);
            mLastNumFrames = 0;
            mLastResetTime = getInputFrameTime(1);
        }

        // N.B. retrieve number of frames after reset
        
        numFrames = getNumFrames(true);
        unsigned long padSize = mode == kPadIn ? requestedFrames - numFrames : 0;
        
        requestOutputSize(0, getFrameLength());
        requestOutputSize(1, 1);
        allocateOutputs();
        double *outputValue = getOutput(0, &sizeOut);
        double *outputValid = getOutput(1, &sizeValid);
        
        if (sizeIn)
        {
            if (numFrames > mLastNumFrames)
            {
                for (unsigned long i = numFrames; i > mLastNumFrames + 1; i--)
                    add(getFrame(i - 1), sizeIn);
                
                add(input, sizeIn);
            }
            else
            {
                for (unsigned long i = mLastNumFrames; i > numFrames; i--)
                    remove(getFrame(i), sizeIn);
                
                exchange(input, getFrame(numFrames), sizeIn);
            }
            
            result(outputValue, sizeOut, PaddedVector(resetInput, sizeReset, pad), padSize);
            write(input, sizeIn);
            mLastNumFrames = numFrames;
        }
        
        if (sizeValid)
            outputValid[0] = sizeIn && requestedFrames != numFrames ? 1 : 0;
    }
    
    static ParameterInfo sParamInfo;

    FrameLib_TimeFormat mLastResetTime;
    unsigned long mLastNumFrames;
};

template <class T, unsigned long N>
typename FrameLib_TimeBuffer<T, N>::ParameterInfo FrameLib_TimeBuffer<T, N>::sParamInfo;

#endif
