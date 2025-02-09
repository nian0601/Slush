#pragma once

#define FW_ARRAY_BOUNDS_CHECK

#define FW_SAFE_DELETE(x) {delete x; x = nullptr;}

#include "FW_Assert.h"
#include "FW_GrowingArray.h"
#include "FW_StaticArray.h"
#include "FW_Hashmap.h"
#include "FW_Rect.h"
#include "FW_String.h"
#include "FW_Vector2.h"
#include "FW_Vector3.h"
#include "FW_Math.h"