// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once
#include "Identity_Integration.h"

// TODO: Move Identity_Init into an Adv_Identity_Integration file
void Adv_Identity_Init(
        _In_ XTaskQueueHandle queue,
        _In_opt_ void* context,
        _In_opt_z_ char const* pathPrefix);