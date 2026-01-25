#pragma once

namespace Vermilion::Offsets {
    namespace Raw {
        constexpr std::uintptr_t GAME_INSTANCE_DATA = 0x12677B8;
        constexpr std::uintptr_t NET_OBJ_MANAGER = 0x2677D0;
        constexpr std::uintptr_t MY_PLAYER = 0x12675A8;
        constexpr std::uintptr_t CHARACTER_MANAGER_CLIENT = 0x1267580;
        constexpr std::uintptr_t INPUT_MANAGER = 0x1267800;
        constexpr std::uintptr_t CONTRAPTION = 0x1267538;
        constexpr std::uintptr_t GAME_STATE = 0x1267598;
        constexpr std::uintptr_t RENDER_STATE_MANAGER = 0x1267820;

        constexpr std::uintptr_t FN_CLIENT_RECEIVE_PACKET = 0x406A60;
        constexpr std::uintptr_t FN_CLIENT_SEND_PACKET = 0x8FDDF0;
    };

    VERMILION_REBASE_OFFSET(GAME_INSTANCE_DATA);
    VERMILION_REBASE_OFFSET(NET_OBJ_MANAGER);
    VERMILION_REBASE_OFFSET(MY_PLAYER);
    VERMILION_REBASE_OFFSET(CHARACTER_MANAGER_CLIENT);
    VERMILION_REBASE_OFFSET(INPUT_MANAGER);
    VERMILION_REBASE_OFFSET(CONTRAPTION);
    VERMILION_REBASE_OFFSET(GAME_STATE);
    VERMILION_REBASE_OFFSET(RENDER_STATE_MANAGER);

    VERMILION_REBASE_OFFSET(FN_CLIENT_RECEIVE_PACKET);
    VERMILION_REBASE_OFFSET(FN_CLIENT_SEND_PACKET);
}