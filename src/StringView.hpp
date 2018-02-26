// Copyright (c) 2018 Tyler Veness. All Rights Reserved.

#pragma once

#include <stdint.h>

struct StringView {
  char* str = nullptr;
  size_t len = 0;
};
