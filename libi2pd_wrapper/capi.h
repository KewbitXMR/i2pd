/*
* Copyright (c) 2021-2022, The PurpleI2P Project
*
* This file is part of Purple i2pd project and licensed under BSD3
*
* See full license text in LICENSE file at top of project tree
*/

/*
* Copyright (c) 2025, KewbitXMR (kewbitxmr@protonmail.com)
*
* This file is part of Purple i2pd project and licensed under BSD3
*
* See full license text in LICENSE file at top of project tree
*/

// capi.h
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Handle types
typedef void* I2PDClient;

// Initialization / Shutdown
void C_I2PInit(const char* config_dir);
void C_I2PShutdown();

// Local Destination Management
I2PDClient C_CreateLocalDestination(bool isPublic);
void C_DestroyLocalDestination(I2PDClient client);

// Stream Management
int C_ConnectStream(I2PDClient client, const char* base64_dest);
void C_ListenStreams(I2PDClient client, void (*acceptor_cb)(int stream_id));

#ifdef __cplusplus
}
#endif
