#include "KeyFrame.h"

namespace LUS {

LUS::KeyFrameSkel::~KeyFrameSkel() { 
    delete[] skelData.limbsFlex;
}

KeyFrameSkeletonData* KeyFrameSkel::GetPointer() {
    return &skelData;
}

size_t KeyFrameSkel::GetPointerSize() {
    return sizeof(skelData);
}


LUS::KeyFrameAnim::~KeyFrameAnim() {
    delete[] animData.bitFlags;
    delete[] animData.keyFrames;
    delete[] animData.kfNums;
    delete[] animData.presetValues;
}

KeyFrameAnimationData* LUS::KeyFrameAnim::GetPointer() {
    return &animData;
}

size_t KeyFrameAnim::GetPointerSize() {
    return sizeof(animData);
}

}