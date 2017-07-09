
#include "FrameLib_Parameters.h"

// Constructors / Destructor

FrameLib_Parameters::Serial::Serial(BytePointer ptr, size_t size) : mPtr(ptr), mSize(0), mMaxSize(size), mOwner(false)
{
    alignmentChecks();
}

FrameLib_Parameters::Serial::Serial(size_t size) : mPtr(new Byte[size]), mSize(0), mMaxSize(size), mOwner(true)
{
    alignmentChecks();
}

FrameLib_Parameters::Serial::Serial() : mPtr(NULL), mSize(0), mMaxSize(0), mOwner(true)
{
    alignmentChecks();
}

FrameLib_Parameters::Serial::~Serial()
{
    if (mOwner)
        delete[] mPtr;
}

// Public Writes

void FrameLib_Parameters::Serial::write(Serial *serialised)
{
    if (!serialised || !checkSize(serialised->mSize))
        return;
    
    memcpy(mPtr + mSize, serialised->mPtr, serialised->mSize);
    mSize += serialised->mSize;
}

void FrameLib_Parameters::Serial::write(const char *tag, char *str)
{
    if (!checkSize(calcSize(tag, str)))
        return;
    
    writeType(kString);
    writeString(tag);
    writeString(str);
}

void FrameLib_Parameters::Serial::write(const char *tag, double *values, size_t N)
{    
    if (!checkSize(calcSize(tag, N)))
        return;
    
    writeType(kDoubleArray);
    writeString(tag);
    writeDoubles(values, N);
}

// Public Read

void FrameLib_Parameters::Serial::read(FrameLib_Parameters *parameters)
{
    BytePointer readPtr = mPtr;
    char *tag, *str;
    double *values;
    size_t N;
    
    while (readPtr < mPtr + mSize)
    {
        switch (readType(&readPtr))
        {
            case kDoubleArray:
                readString(&readPtr, &tag);
                readDoubles(&readPtr, &values, &N);
                parameters->set(tag, values, N);
                break;
                
            case kString:
                readString(&readPtr, &tag);
                readString(&readPtr, &str);
                parameters->set(tag, str);
                break;
        }
    }
}

// Implementation

void FrameLib_Parameters::Serial::alignmentChecks()
{
    // Assume that alignment of a double is fine for all natural alignment needs (including this class)
    
    assert(Serial::alignment >= sizeof(DataType) && "alignment assumptions are incorrect for FrameLib_Parameters::Serial::DataType");
    assert(Serial::alignment >= sizeof(size_t) && "alignment assumptions are incorrect for FrameLib_Parameters::Serial");
    assert(Serial::alignment >= sizeof(char) && "alignment assumptions are incorrect for FrameLib_Parameters::Serial");
    assert(Serial::alignment >= sizeof(char *) && "alignment assumptions are incorrect for FrameLib_Parameters::Serial");
    assert(Serial::alignment <= FrameLib_GlobalAllocator::getAlignment() && "alignment assumptions are incorrect for FrameLib_Parameters::Serial");
}

// Writes (private)

void FrameLib_Parameters::Serial::writeType(DataType type)
{
    *((DataType *) (mPtr + mSize)) = type;
    mSize += alignSize(sizeof(DataType));
}

void FrameLib_Parameters::Serial::writeSize(size_t size)
{
    *((size_t *) (mPtr + mSize)) = size;
    mSize += alignSize(sizeof(size_t));
}

void FrameLib_Parameters::Serial::writeString(const char *str)
{
    size_t N = strlen(str) + 1;
    writeSize(N);
    strcpy((char *) (mPtr + mSize), str);
    mSize += alignSize(N);
}

void FrameLib_Parameters::Serial::writeDoubles(double *ptr, size_t N)
{
    size_t size = N * sizeof(double);
    writeSize(N);
    memcpy(mPtr + mSize, ptr, size);
    mSize += alignSize(size);
}

// Reads (private)

FrameLib_Parameters::Serial::DataType FrameLib_Parameters::Serial::readType(BytePointer *readPtr)
{
    DataType type = *((DataType *) *readPtr);
    *readPtr += alignSize(sizeof(DataType));
    return type;
}

void FrameLib_Parameters::Serial::readSize(BytePointer *readPtr, size_t *size)
{
    *size = *((size_t *) *readPtr);
    *readPtr += alignSize(sizeof(size_t));
}

void FrameLib_Parameters::Serial::readDoubles(BytePointer *readPtr, double **values, size_t *N)
{
    readSize(readPtr, N);
    *values = ((double *) *readPtr);
    *readPtr += alignSize(*N * sizeof(double));
}

void FrameLib_Parameters::Serial::readString(BytePointer *readPtr, char **str)
{
    size_t size;
    readSize(readPtr, &size);
    *str = ((char *) *readPtr);
    *readPtr += alignSize(size);
}

// Size Check

bool FrameLib_Parameters::Serial::checkSize(size_t writeSize)
{
    size_t growSize;
    
    if (mSize + writeSize <= mMaxSize)
        return true;
    
    if (!mOwner)
        return false;
    
    // Calculate grow size
    
    growSize = (mSize + writeSize) - mMaxSize;
    growSize = growSize < minGrowSize ? minGrowSize : growSize;
    
    // Allocate required memory and copy old data before deleting the old pointer
    
    BytePointer newPtr = new Byte[mMaxSize + growSize];
    memcpy(newPtr, mPtr, mSize);
    delete[] mPtr;
    
    // Update
    
    mPtr = newPtr;
    mMaxSize += growSize;
    
    return true;
}

// ************************************************************************************** //

// Parameter Abstract Class

FrameLib_Parameters::Parameter::Parameter(const char *name, long argumentIdx) : mChanged(false)
{
    mName = name;
    mArgumentIdx = argumentIdx;
}

void FrameLib_Parameters::Parameter::addEnumItem(const char *str)
{
    assert(0 && "cannot add enum items to non-enum parameter");
}

void FrameLib_Parameters::Parameter::setMin(double min)
{
    assert(0 && "parameter type does not support minimum values");
}

void FrameLib_Parameters::Parameter::setMax(double max)
{
    assert(0 && "parameter type does not support maximum values");
}

void FrameLib_Parameters::Parameter::setClip(double min, double max)
{
    assert(0 && "parameter type does not support clipping values");
}

void FrameLib_Parameters::Parameter::set(double *values, size_t size)
{
    if (size)
        set(*values);
    else
        clear();
}

void FrameLib_Parameters::Parameter::getRange(double *min, double *max)
{
    *min = 0;
    *max = 0;
}

const char *FrameLib_Parameters::Parameter::getItemString(unsigned long item) const
{
    assert(0 && "cannot get enum string for non-enum parameter");
}

const double *FrameLib_Parameters::Parameter::getArray(size_t *size) const
{
    *size = getArraySize();
    return getArray();
}

bool FrameLib_Parameters::Parameter::changed()
{
    bool result = mChanged;
    mChanged = false;
    return result;
}

// ************************************************************************************** //

// Bool Parameter Class

void FrameLib_Parameters::Bool::set(double value)
{
    mValue = value ? true : false;
    mChanged = true;
}

void FrameLib_Parameters::Bool::set(double *values, size_t size)
{
    if (size)
        Bool::set(*values);
    else
        Bool::clear();
}

void FrameLib_Parameters::Bool::getRange(double *min, double *max)
{
    *min = false;
    *max = true;
}

// ************************************************************************************** //

// Enum Parameter Class

void FrameLib_Parameters::Enum::set(double value)
{
    mValue = ((value >= mItems.size()) ? (mItems.size() - 1) : (value < 0 ? 0 : value));
    mChanged = true;
}

void FrameLib_Parameters::Enum::set(const char *str)
{
    for (unsigned long i = 0; i < mItems.size(); i++)
    {
        if (strcmp(str, mItems[i].c_str()) == 0)
        {
            mValue = i;
            mChanged = true;
            return;
        }
    }
}

void FrameLib_Parameters::Enum::set(double *values, size_t size)
{
    if (size)
        Enum::set(*values);
    else
        Enum::clear();
}

void FrameLib_Parameters::Enum::getRange(double *min, double *max)
{
    *min = 0;
    *max = mItems.size() - 1;
}

// ************************************************************************************** //

// Double Parameter Class

void FrameLib_Parameters::Double::set(double value)
{
    mValue = (value < mMin) ? mMin : ((value > mMax) ? mMax : value);
    mChanged = true;
}

void FrameLib_Parameters::Double::set(double *values, size_t size)
{
    if (size)
        Double::set(*values);
    else
        Double::clear();
}

void FrameLib_Parameters::Double::getRange(double *min, double *max)
{
    *min = mMin;
    *max = mMax;
}

void FrameLib_Parameters::Double::setMin(double min)
{
    mMin = min;
    mMax = std::numeric_limits<double>::infinity();
}

void FrameLib_Parameters::Double::setMax(double max)
{
    mMin = -std::numeric_limits<double>::infinity();
    mMax = max;
}

void FrameLib_Parameters::Double::setClip(double min, double max)
{
    mMin = min;
    mMax = max;
}

// ************************************************************************************** //

// String Parameter Class

FrameLib_Parameters::String::String(const char *name, const char *str, long argumentIdx) : Parameter(name, argumentIdx)
{
    String::set(str);
}

void FrameLib_Parameters::String::set(const char *str)
{
    size_t i = 0;
    
    if (str != NULL)
    {
        for (i = 0; i < maxLen; i++)
            if ((mCString[i] = str[i]) == 0)
                break;
    }
    
    mCString[i] = 0;
    mChanged = true;
}

// ************************************************************************************** //

// Array Parameter Class

FrameLib_Parameters::Array::Array(const char *name, long argumentIdx, double defaultValue, size_t size)
: Parameter(name, argumentIdx), mMode(kNone), mDefaultValue(defaultValue), mSize(size), mVariableSize(false)
{
    mItems.resize(size);
    
    for (size_t i = 0; i < mSize; i++)
        mItems[i] = defaultValue;
}

FrameLib_Parameters::Array::Array(const char *name, long argumentIdx, double defaultValue, size_t maxSize, size_t size)
: Parameter(name, argumentIdx), mMode(kNone), mDefaultValue(defaultValue), mVariableSize(true)
{
    mItems.resize(maxSize);
    
    mSize = size < maxSize ? size : maxSize;
    
    for (size_t i = 0; i < mSize; i++)
        mItems[i] = defaultValue;
}

void FrameLib_Parameters::Array::setMin(double min)
{
    mMode = kMin;
    mMin = min;
}

void FrameLib_Parameters::Array::setMax(double max)
{
    mMode = kMax;
    mMax = max;
}

void FrameLib_Parameters::Array::setClip(double min, double max)
{
    mMode = kClip;
    mMin = min;
    mMax = max;
}

void FrameLib_Parameters::Array::set(double *values, size_t size)
{
    size = size > mItems.size() ? mItems.size() : size;
    
    switch (mMode)
    {
        case kNone:
            for (size_t i = 0; i < size; i++)
                mItems[i] = values[i];
            break;
        case kMin:
            for (size_t i = 0; i < size; i++)
                mItems[i] = values[i] < mMin ? mMin : values[i];
            break;
        case kMax:
            for (size_t i = 0; i < size; i++)
                mItems[i] = values[i] > mMax ? mMax : values[i];
            break;
        case kClip:
            for (size_t i = 0; i < size; i++)
                mItems[i] = values[i] < mMin ? mMin : (values[i] > mMax ? mMax : values[i]);
            break;
    }
    
    if (!mVariableSize)
        for (size_t i = size; i < mItems.size(); i++)
            mItems[i] = mDefaultValue;
    else
        mSize = size;
    
    mChanged = true;
}

void FrameLib_Parameters::Array::getRange(double *min, double *max)
{
    *min = mMin;
    *max = mMax;
}