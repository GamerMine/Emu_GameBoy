/*
 *  ____
 * /\  _`\                                       /'\_/`\  __
 * \ \ \L\_\     __      ___ ___      __   _ __ /\      \/\_\    ___      __
 *  \ \ \L_L   /'__`\  /' __` __`\  /'__`\/\`'__\ \ \__\ \/\ \ /' _ `\  /'__`\
 *   \ \ \/, \/\ \L\.\_/\ \/\ \/\ \/\  __/\ \ \/ \ \ \_/\ \ \ \/\ \/\ \/\  __/
 *    \ \____/\ \__/.\_\ \_\ \_\ \_\ \____\\ \_\  \ \_\\ \_\ \_\ \_\ \_\ \____\
 *     \/___/  \/__/\/_/\/_/\/_/\/_/\/____/ \/_/   \/_/ \/_/\/_/\/_/\/_/\/____/
 *
 * Copyright (c) 2024-2024 GamerMine <maxime-sav@outlook.fr>
 *
 * This Source Code From is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/ .
 */

#ifndef EMU_GAMEBOY_NOMBC_H
#define EMU_GAMEBOY_NOMBC_H

#include "Cartridge.h"

class NoMBC : public Cartridge {
public:
    uint8_t read(uint16_t addr) override {
        if (addr <= 0x7FFF) {
            return gameRom[addr];
        }

        return 0xFF;
    }

    void loadGame(const char *filename) override {
        std::ifstream file(filename, std::ios::binary);
        file.read(reinterpret_cast<char *>(gameRom.data()), sizeof(uint8_t) * 32*1024);
        file.close();
    }

private:
    std::array<uint8_t, 32*1024> gameRom;
};

#endif //EMU_GAMEBOY_NOMBC_H