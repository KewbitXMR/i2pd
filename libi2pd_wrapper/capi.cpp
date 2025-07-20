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

#include "../libi2pd/api.h"
#include "../libi2pd/Destination.h"
#include "../libi2pd/ClientContext.h"
#include "../libi2pd/Streaming.h"
#include "../libi2pd/Log.h"

#include "capi.h"

#include <iostream>
#include <memory>
#include <cstring>
#include <unordered_map>
#include <mutex>

static std::unordered_map<void*, std::shared_ptr<i2p::client::ClientDestination>> g_destinations;
static std::mutex g_dest_mutex;

extern "C" {

void C_I2PInit(int argc, char* argv[], const char* appName) {
    i2p::api::InitI2P(argc, argv, appName);
}

void C_I2PShutdown() {
    i2p::api::TerminateI2P();
}

void* C_CreateLocalDestination(int isPublic, const char* base64_privkey) {
    std::shared_ptr<i2p::data::PrivateKeys> keys;
    if (base64_privkey) {
        std::string str(base64_privkey);
        auto buf = i2p::data::Base64ToByteStream(str);
        keys = std::make_shared<i2p::data::PrivateKeys>();
        keys->FromBuffer((const uint8_t*)buf.data(), buf.size());
    }

    auto dest = i2p::client::context.CreateNewLocalDestination(
        keys, isPublic != 0, i2p::data::SigningKeyType::SIGNING_KEY_TYPE_EDDSA_SHA512_ED25519
    );

    if (!dest) return nullptr;

    void* handle = static_cast<void*>(dest.get());

    {
        std::lock_guard<std::mutex> lock(g_dest_mutex);
        g_destinations[handle] = dest;
    }

    return handle;
}

void C_DestroyLocalDestination(void* handle) {
    std::lock_guard<std::mutex> lock(g_dest_mutex);
    auto it = g_destinations.find(handle);
    if (it != g_destinations.end()) {
        it->second->Stop();
        g_destinations.erase(it);
    }
}

void* C_ConnectStream(void* srcHandle, const char* base64_dest, uint16_t port) {
    std::lock_guard<std::mutex> lock(g_dest_mutex);
    auto it = g_destinations.find(srcHandle);
    if (it == g_destinations.end()) return nullptr;

    auto dest = it->second;
    auto ident = i2p::data::IdentityEx(base64_dest);
    if (!ident.IsValid()) return nullptr;

    auto remoteIdent = std::make_shared<i2p::data::IdentityEx>(ident);
    auto stream = dest->CreateStream(remoteIdent, port);

    if (!stream) return nullptr;
    return static_cast<void*>(stream);
}

void C_ListenStreams(void* srcHandle, void (*cb)(void* stream, void* userdata), void* userdata) {
    std::lock_guard<std::mutex> lock(g_dest_mutex);
    auto it = g_destinations.find(srcHandle);
    if (it == g_destinations.end()) return;

    it->second->CreateStreamAccepting([cb, userdata](std::shared_ptr<i2p::stream::Stream> s) {
        cb(static_cast<void*>(s.get()), userdata);
    });
}

void* C_CreateServerTunnel(void* dest_handle, uint16_t i2p_port, const char* local_host, uint16_t local_port) {
    std::lock_guard<std::mutex> lock(g_dest_mutex);
    auto it = g_destinations.find(dest_handle);
    if (it == g_destinations.end()) return nullptr;

    auto dest = it->second;

    auto tunnel = std::make_shared<i2p::client::I2PServerTunnel>(
        i2p_port,
        local_host,
        local_port,
        dest
    );

    tunnel->Start();

    void* handle = static_cast<void*>(tunnel.get());

    {
        std::lock_guard<std::mutex> tlock(g_tunnel_mutex);
        g_tunnels[handle] = tunnel;
    }

    return handle;
}

void C_StopTunnel(void* tunnel_handle) {
    std::lock_guard<std::mutex> lock(g_tunnel_mutex);
    auto it = g_tunnels.find(tunnel_handle);
    if (it != g_tunnels.end()) {
        it->second->Stop();
        g_tunnels.erase(it);
    }
}

}
