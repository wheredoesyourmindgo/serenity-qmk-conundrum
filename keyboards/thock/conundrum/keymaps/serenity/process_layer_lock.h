// users/serenity includes QMK 0.28's core Layer Lock header; on 0.26 getreuer/layer_lock
// provides the same functions (see compat.h)
#pragma once

#include "layer_lock.h"

#define process_layer_lock process_record_layer_lock
