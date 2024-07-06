#include "./SlotState.h"

SlotState SlotState::blend(SlotState& other, float ratio){
    SlotState result;

    float otherRatio = 1.0 - ratio;

    #define blend(a,b) a * ratio + b * otherRatio

    result.rgb[0] = blend(this->rgb[0], other.rgb[0]);
    result.rgb[1] = blend(this->rgb[1], other.rgb[1]);
    result.rgb[2] = blend(this->rgb[2], other.rgb[2]);
    result.rgb[3] = blend(this->rgb[3], other.rgb[3]);

    result.transparency = blend(this->transparency, other.transparency);

    result.posLeft = blend(this->posLeft, other.posLeft);
    result.posTop = blend(this->posTop, other.posTop);

    result.scale = blend(this->scale, other.scale);

    #undef blend

    return result;
}

SlotState SlotState::parent(SlotState& child){
    SlotState result;

    SlotState blended = child.blend(*this, child.rgb[3]);
    result.rgb[0] = blended.rgb[0];
    result.rgb[1] = blended.rgb[1];
    result.rgb[2] = blended.rgb[2];
    result.rgb[3] = blended.rgb[3];

    result.transparency = this->transparency * child.transparency;
    result.scale = this->scale * child.scale;
    result.posLeft = this->posLeft + child.posLeft;
    result.posTop = this->posTop + child.posTop;

    return result;
}