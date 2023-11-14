#ifndef EMU_GAMEBOY_SHARPSM83_H
#define EMU_GAMEBOY_SHARPSM83_H

#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>
#include <string>

#include "Bus.h"
#include "../logging/Logger.h"

class SharpSM83 {
public:
    explicit SharpSM83(class Bus *bus);

    void reset();
    void operator()();

    union {
        struct {
            bool vblank : 1;
            bool lcd    : 1;
            bool timer  : 1;
            bool serial : 1;
            bool joypad : 1;
            bool unused : 3;
        };
        uint8_t raw;
    } IE {}; // Interrupt Enable controls handlers calls

    inline static union {
        struct {
            bool vblank : 1;
            bool lcd    : 1;
            bool timer  : 1;
            bool serial : 1;
            bool joypad : 1;
            bool unused : 3;
        };
        uint8_t raw;
    } IF {}; // Interrupt flag controls handlers request

    // Debug //
    struct debugInfo {
        const char *currentInstr;
        uint16_t currentAddr;
        bool Z;
        bool C;
        bool HC;
        bool N;
        ~debugInfo();
    };

    inline static bool executingInterrupt = false;
    static bool PAUSE;
    static bool NEXT_INSTR;
    static debugInfo DEBUG_INFO;
    inline static bool dmaCycles = false;
    inline static bool ENABLE_DEBUG_PRINTS = false;

private:
    union {
        struct {
            bool unused1;
            bool unused2;
            bool unused3;
            bool unused4;
            bool carry;
            bool halfCarry;
            bool negative;
            bool zero;
        };
        uint8_t rawFlags;
    } flags{};

    union {
        union {
            struct {
                uint8_t C;
                uint8_t B;
                uint8_t E;
                uint8_t D;
                uint8_t L;
                uint8_t H;
                uint8_t unused;
                uint8_t A;
            };
            struct {
                uint16_t BC;
                uint16_t DE;
                uint16_t HL;
            };
        };
        uint8_t rawRegisters[8];
    } registers{};

    uint16_t PC, SP;
    bool interruptShouldBeEnabled, IME; // Interrupt master enable flag
    Bus *mBus;
    static Logger *logger;

private:
    using opcodeFunc = std::function<uint8_t()>;
    opcodeFunc opcodes[256] = {
            [this]() {return NOP();},
            [this]() {return LD(&registers.BC, nullptr);},
            [this]() {return LD(registers.BC, &registers.A);},
            [this]() {return INC(&registers.BC);},
            [this]() {return INC(&registers.B);},
            [this]() {return DEC(&registers.B);},
            [this]() {return LD(&registers.B, nullptr);},
            [this]() {return RLCA();},
            [this]() {return LD(nullptr, &SP);},
            [this]() {return ADD(&registers.HL, &registers.BC);},
            [this]() {return LD(&registers.A, registers.BC);},
            [this]() {return DEC(&registers.BC);},
            [this]() {return INC(&registers.C);},
            [this]() {return DEC(&registers.C);},
            [this]() {return LD(&registers.C, nullptr);},
            [this]() {return RRCA();},
            [this]() {return STOP();},
            [this]() {return LD(&registers.DE, nullptr);},
            [this]() {return LD(registers.DE, &registers.A);},
            [this]() {return INC(&registers.DE);},
            [this]() {return INC(&registers.D);},
            [this]() {return DEC(&registers.D);},
            [this]() {return LD(&registers.D, nullptr);},
            [this]() {return RLA();},
            [this]() {return JR(nullptr);},
            [this]() {return ADD(&registers.HL, &registers.DE);},
            [this]() {return LD(&registers.A, registers.DE);},
            [this]() {return DEC(&registers.DE);},
            [this]() {return INC(&registers.E);},
            [this]() {return DEC(&registers.E);},
            [this]() {return LD(&registers.E, nullptr);},
            [this]() {return RRA();},
            [this]() {return JR(&flags.zero, true);},
            [this]() {return LD(&registers.HL, nullptr);},
            [this]() {return LD(registers.HL, &registers.A, true, false);},
            [this]() {return INC(&registers.HL);},
            [this]() {return INC(&registers.H);},
            [this]() {return DEC(&registers.H);},
            [this]() {return LD(&registers.H, nullptr);},
            [this]() {return DAA();},
            [this]() {return JR(&flags.zero);},
            [this]() {return ADD(&registers.HL, &registers.HL);},
            [this]() {return LD(&registers.A, registers.HL, true, false);},
            [this]() {return DEC(&registers.HL);},
            [this]() {return INC(&registers.L);},
            [this]() {return DEC(&registers.L);},
            [this]() {return LD(&registers.L, nullptr);},
            [this]() {return CPL();},
            [this]() {return JR(&flags.carry, true);},
            [this]() {return LD(&SP, nullptr);},
            [this]() {return LD(registers.HL, &registers.A, false, true);},
            [this]() {return INC(&SP);},
            [this]() {return INC(registers.HL);},
            [this]() {return DEC(registers.HL);},
            [this]() {return LD(registers.HL, nullptr);},
            [this]() {return SCF();},
            [this]() {return JR(&flags.carry);},
            [this]() {return ADD(&registers.HL, &SP);},
            [this]() {return LD(&registers.A, registers.HL, false, true);},
            [this]() {return DEC(&SP);},
            [this]() {return INC(&registers.A);},
            [this]() {return DEC(&registers.A);},
            [this]() {return LD(&registers.A, nullptr);},
            [this]() {return CCF();},
            [this]() {return LD(&registers.B, &registers.B);},
            [this]() {return LD(&registers.B, &registers.C);},
            [this]() {return LD(&registers.B, &registers.D);},
            [this]() {return LD(&registers.B, &registers.E);},
            [this]() {return LD(&registers.B, &registers.H);},
            [this]() {return LD(&registers.B, &registers.L);},
            [this]() {return LD(&registers.B, registers.HL);},
            [this]() {return LD(&registers.B, &registers.A);},
            [this]() {return LD(&registers.C, &registers.B);},
            [this]() {return LD(&registers.C, &registers.C);},
            [this]() {return LD(&registers.C, &registers.D);},
            [this]() {return LD(&registers.C, &registers.E);},
            [this]() {return LD(&registers.C, &registers.H);},
            [this]() {return LD(&registers.C, &registers.L);},
            [this]() {return LD(&registers.C, registers.HL);},
            [this]() {return LD(&registers.C, &registers.A);},
            [this]() {return LD(&registers.D, &registers.B);},
            [this]() {return LD(&registers.D, &registers.C);},
            [this]() {return LD(&registers.D, &registers.D);},
            [this]() {return LD(&registers.D, &registers.E);},
            [this]() {return LD(&registers.D, &registers.H);},
            [this]() {return LD(&registers.D, &registers.L);},
            [this]() {return LD(&registers.D, registers.HL);},
            [this]() {return LD(&registers.D, &registers.A);},
            [this]() {return LD(&registers.E, &registers.B);},
            [this]() {return LD(&registers.E, &registers.C);},
            [this]() {return LD(&registers.E, &registers.D);},
            [this]() {return LD(&registers.E, &registers.E);},
            [this]() {return LD(&registers.E, &registers.H);},
            [this]() {return LD(&registers.E, &registers.L);},
            [this]() {return LD(&registers.E, registers.HL);},
            [this]() {return LD(&registers.E, &registers.A);},
            [this]() {return LD(&registers.H, &registers.B);},
            [this]() {return LD(&registers.H, &registers.C);},
            [this]() {return LD(&registers.H, &registers.D);},
            [this]() {return LD(&registers.H, &registers.E);},
            [this]() {return LD(&registers.H, &registers.H);},
            [this]() {return LD(&registers.H, &registers.L);},
            [this]() {return LD(&registers.H, registers.HL);},
            [this]() {return LD(&registers.H, &registers.A);},
            [this]() {return LD(&registers.L, &registers.C);},
            [this]() {return LD(&registers.L, &registers.D);},
            [this]() {return LD(&registers.L, &registers.B);},
            [this]() {return LD(&registers.L, &registers.E);},
            [this]() {return LD(&registers.L, &registers.H);},
            [this]() {return LD(&registers.L, &registers.L);},
            [this]() {return LD(&registers.L, registers.HL);},
            [this]() {return LD(&registers.L, &registers.A);},
            [this]() {return LD(registers.HL, &registers.B);},
            [this]() {return LD(registers.HL, &registers.C);},
            [this]() {return LD(registers.HL, &registers.D);},
            [this]() {return LD(registers.HL, &registers.E);},
            [this]() {return LD(registers.HL, &registers.H);},
            [this]() {return LD(registers.HL, &registers.L);},
            [this]() {return HALT();},
            [this]() {return LD(registers.HL, &registers.A);},
            [this]() {return LD(&registers.A, &registers.B);},
            [this]() {return LD(&registers.A, &registers.C);},
            [this]() {return LD(&registers.A, &registers.D);},
            [this]() {return LD(&registers.A, &registers.E);},
            [this]() {return LD(&registers.A, &registers.H);},
            [this]() {return LD(&registers.A, &registers.L);},
            [this]() {return LD(&registers.A, registers.HL);},
            [this]() {return LD(&registers.A, &registers.A);},
            [this]() {return ADD(&registers.B);},
            [this]() {return ADD(&registers.C);},
            [this]() {return ADD(&registers.D);},
            [this]() {return ADD(&registers.E);},
            [this]() {return ADD(&registers.H);},
            [this]() {return ADD(&registers.L);},
            [this]() {return ADD(registers.HL);},
            [this]() {return ADD(&registers.A);},
            [this]() {return ADC(&registers.B);},
            [this]() {return ADC(&registers.C);},
            [this]() {return ADC(&registers.D);},
            [this]() {return ADC(&registers.E);},
            [this]() {return ADC(&registers.H);},
            [this]() {return ADC(&registers.L);},
            [this]() {return ADC(registers.HL);},
            [this]() {return ADC(&registers.A);},
            [this]() {return SUB(&registers.B);},
            [this]() {return SUB(&registers.C);},
            [this]() {return SUB(&registers.D);},
            [this]() {return SUB(&registers.E);},
            [this]() {return SUB(&registers.H);},
            [this]() {return SUB(&registers.L);},
            [this]() {return SUB(registers.HL);},
            [this]() {return SUB(&registers.A);},
            [this]() {return SBC(&registers.A, &registers.B);},
            [this]() {return SBC(&registers.A, &registers.C);},
            [this]() {return SBC(&registers.A, &registers.D);},
            [this]() {return SBC(&registers.A, &registers.E);},
            [this]() {return SBC(&registers.A, &registers.H);},
            [this]() {return SBC(&registers.A, &registers.L);},
            [this]() {return SBC(&registers.A, nullptr);},
            [this]() {return SBC(&registers.A, &registers.A);},
            [this]() {return AND(&registers.B);},
            [this]() {return AND(&registers.C);},
            [this]() {return AND(&registers.D);},
            [this]() {return AND(&registers.E);},
            [this]() {return AND(&registers.H);},
            [this]() {return AND(&registers.L);},
            [this]() {return AND(registers.HL);},
            [this]() {return AND(&registers.A);},
            [this]() {return XOR(&registers.B);},
            [this]() {return XOR(&registers.C);},
            [this]() {return XOR(&registers.D);},
            [this]() {return XOR(&registers.E);},
            [this]() {return XOR(&registers.H);},
            [this]() {return XOR(&registers.L);},
            [this]() {return XOR(registers.HL);},
            [this]() {return XOR(&registers.A);},
            [this]() {return OR(&registers.B);},
            [this]() {return OR(&registers.C);},
            [this]() {return OR(&registers.D);},
            [this]() {return OR(&registers.E);},
            [this]() {return OR(&registers.H);},
            [this]() {return OR(&registers.L);},
            [this]() {return OR(registers.HL);},
            [this]() {return OR(&registers.A);},
            [this]() {return CP(&registers.B);},
            [this]() {return CP(&registers.C);},
            [this]() {return CP(&registers.D);},
            [this]() {return CP(&registers.E);},
            [this]() {return CP(&registers.H);},
            [this]() {return CP(&registers.L);},
            [this]() {return CP(registers.HL);},
            [this]() {return CP(&registers.A);},
            [this]() {return RET(&flags.zero, true);},
            [this]() {return POP(&registers.BC);},
            [this]() {return JP(&flags.zero, true);},
            [this]() {return JP(nullptr, false);},
            [this]() {return CALL(&flags.zero, true);},
            [this]() {return PUSH(&registers.BC);},
            [this]() {return ADD(nullptr);},
            [this]() {return RST(0x0000);},
            [this]() {return RET(&flags.zero);},
            [this]() {return RET(nullptr);},
            [this]() {return JP(&flags.zero);},
            [this]() {return PREFIX();},
            [this]() {return CALL(&flags.zero);},
            [this]() {return CALL(nullptr);},
            [this]() {return ADC(nullptr);},
            [this]() {return RST(0x0008);},
            [this]() {return RET(&flags.carry, true);},
            [this]() {return POP(&registers.DE);},
            [this]() {return JP(&flags.carry, true);},
            [this]() {return NIMP();},
            [this]() {return CALL(&flags.carry, true);},
            [this]() {return PUSH(&registers.DE);},
            [this]() {return SUB(nullptr);},
            [this]() {return RST(0x0010);},
            [this]() {return RET(&flags.carry);},
            [this]() {return RETI();},
            [this]() {return JP(&flags.carry);},
            [this]() {return NIMP();},
            [this]() {return CALL(&flags.carry);},
            [this]() {return NIMP();},
            [this]() {return SBC(&registers.A, nullptr);},
            [this]() {return RST(0x0018);},
            [this]() {return LDH(nullptr);},
            [this]() {return POP(&registers.HL);},
            [this]() {return LD(registers.C, &registers.A);},
            [this]() {return NIMP();},
            [this]() {return NIMP();},
            [this]() {return PUSH(&registers.HL);},
            [this]() {return AND(nullptr);},
            [this]() {return RST(0x0020);},
            [this]() {return ADD(&SP, nullptr);},
            [this]() {return JP(&registers.HL);},
            [this]() {return LD(nullptr);},
            [this]() {return NIMP();},
            [this]() {return NIMP();},
            [this]() {return NIMP();},
            [this]() {return XOR(nullptr);},
            [this]() {return RST(0x0028);},
            [this]() {return LDH(&registers.A);},
            [this]() {return POP(nullptr);},
            [this]() {return LD(&registers.A, registers.C);},
            [this]() {return DI();},
            [this]() {return NIMP();},
            [this]() {return PUSH(nullptr);},
            [this]() {return OR(nullptr);},
            [this]() {return RST(0x0030);},
            [this]() {return LD(&registers.HL, &SP, true);},
            [this]() {return LD(&SP, &registers.HL);},
            [this]() {return LD(&registers.A);},
            [this]() {return EI();},
            [this]() {return NIMP();},
            [this]() {return NIMP();},
            [this]() {return CP(nullptr);},
            [this]() {return RST(0x0038);},
    };

    opcodeFunc pfxOpcodes[256] = {
        [this]() {return RLC(&registers.B);},
        [this]() {return RLC(&registers.C);},
        [this]() {return RLC(&registers.D);},
        [this]() {return RLC(&registers.E);},
        [this]() {return RLC(&registers.H);},
        [this]() {return RLC(&registers.L);},
        [this]() {return RLC(nullptr);},
        [this]() {return RLC(&registers.A);},
        [this]() {return RRC(&registers.B);},
        [this]() {return RRC(&registers.C);},
        [this]() {return RRC(&registers.D);},
        [this]() {return RRC(&registers.E);},
        [this]() {return RRC(&registers.H);},
        [this]() {return RRC(&registers.L);},
        [this]() {return RRC(nullptr);},
        [this]() {return RRC(&registers.A);},
        [this]() {return RL(&registers.B);},
        [this]() {return RL(&registers.C);},
        [this]() {return RL(&registers.D);},
        [this]() {return RL(&registers.E);},
        [this]() {return RL(&registers.H);},
        [this]() {return RL(&registers.L);},
        [this]() {return RL(nullptr);},
        [this]() {return RL(&registers.A);},
        [this]() {return RR(&registers.B);},
        [this]() {return RR(&registers.C);},
        [this]() {return RR(&registers.D);},
        [this]() {return RR(&registers.E);},
        [this]() {return RR(&registers.H);},
        [this]() {return RR(&registers.L);},
        [this]() {return RR(nullptr);},
        [this]() {return RR(&registers.A);},
        [this]() {return SLA(&registers.B);},
        [this]() {return SLA(&registers.C);},
        [this]() {return SLA(&registers.D);},
        [this]() {return SLA(&registers.E);},
        [this]() {return SLA(&registers.H);},
        [this]() {return SLA(&registers.L);},
        [this]() {return SLA(nullptr);},
        [this]() {return SLA(&registers.A);},
        [this]() {return SRA(&registers.B);},
        [this]() {return SRA(&registers.C);},
        [this]() {return SRA(&registers.D);},
        [this]() {return SRA(&registers.E);},
        [this]() {return SRA(&registers.H);},
        [this]() {return SRA(&registers.L);},
        [this]() {return SRA(nullptr);},
        [this]() {return SRA(&registers.A);},
        [this]() {return SWAP(&registers.B);},
        [this]() {return SWAP(&registers.C);},
        [this]() {return SWAP(&registers.D);},
        [this]() {return SWAP(&registers.E);},
        [this]() {return SWAP(&registers.H);},
        [this]() {return SWAP(&registers.L);},
        [this]() {return SWAP(nullptr);},
        [this]() {return SWAP(&registers.A);},
        [this]() {return SRL(&registers.B);},
        [this]() {return SRL(&registers.C);},
        [this]() {return SRL(&registers.D);},
        [this]() {return SRL(&registers.E);},
        [this]() {return SRL(&registers.H);},
        [this]() {return SRL(&registers.L);},
        [this]() {return SRL(nullptr);},
        [this]() {return SRL(&registers.A);},
        [this]() {return BIT(0, &registers.B);},
        [this]() {return BIT(0, &registers.C);},
        [this]() {return BIT(0, &registers.D);},
        [this]() {return BIT(0, &registers.E);},
        [this]() {return BIT(0, &registers.H);},
        [this]() {return BIT(0, &registers.L);},
        [this]() {return BIT(0, nullptr);},
        [this]() {return BIT(0, &registers.A);},
        [this]() {return BIT(1, &registers.B);},
        [this]() {return BIT(1, &registers.C);},
        [this]() {return BIT(1, &registers.D);},
        [this]() {return BIT(1, &registers.E);},
        [this]() {return BIT(1, &registers.H);},
        [this]() {return BIT(1, &registers.L);},
        [this]() {return BIT(1, nullptr);},
        [this]() {return BIT(1, &registers.A);},
        [this]() {return BIT(2, &registers.B);},
        [this]() {return BIT(2, &registers.C);},
        [this]() {return BIT(2, &registers.D);},
        [this]() {return BIT(2, &registers.E);},
        [this]() {return BIT(2, &registers.H);},
        [this]() {return BIT(2, &registers.L);},
        [this]() {return BIT(2, nullptr);},
        [this]() {return BIT(2, &registers.A);},
        [this]() {return BIT(3, &registers.B);},
        [this]() {return BIT(3, &registers.C);},
        [this]() {return BIT(3, &registers.D);},
        [this]() {return BIT(3, &registers.E);},
        [this]() {return BIT(3, &registers.H);},
        [this]() {return BIT(3, &registers.L);},
        [this]() {return BIT(3, nullptr);},
        [this]() {return BIT(3, &registers.A);},
        [this]() {return BIT(4, &registers.B);},
        [this]() {return BIT(4, &registers.C);},
        [this]() {return BIT(4, &registers.D);},
        [this]() {return BIT(4, &registers.E);},
        [this]() {return BIT(4, &registers.H);},
        [this]() {return BIT(4, &registers.L);},
        [this]() {return BIT(4, nullptr);},
        [this]() {return BIT(4, &registers.A);},
        [this]() {return BIT(5, &registers.B);},
        [this]() {return BIT(5, &registers.C);},
        [this]() {return BIT(5, &registers.D);},
        [this]() {return BIT(5, &registers.E);},
        [this]() {return BIT(5, &registers.H);},
        [this]() {return BIT(5, &registers.L);},
        [this]() {return BIT(5, nullptr);},
        [this]() {return BIT(5, &registers.A);},
        [this]() {return BIT(6, &registers.B);},
        [this]() {return BIT(6, &registers.C);},
        [this]() {return BIT(6, &registers.D);},
        [this]() {return BIT(6, &registers.E);},
        [this]() {return BIT(6, &registers.H);},
        [this]() {return BIT(6, &registers.L);},
        [this]() {return BIT(6, nullptr);},
        [this]() {return BIT(6, &registers.A);},
        [this]() {return BIT(7, &registers.B);},
        [this]() {return BIT(7, &registers.C);},
        [this]() {return BIT(7, &registers.D);},
        [this]() {return BIT(7, &registers.E);},
        [this]() {return BIT(7, &registers.H);},
        [this]() {return BIT(7, &registers.L);},
        [this]() {return BIT(7, nullptr);},
        [this]() {return BIT(7, &registers.A);},
        [this]() {return RES(0, &registers.B);},
        [this]() {return RES(0, &registers.C);},
        [this]() {return RES(0, &registers.D);},
        [this]() {return RES(0, &registers.E);},
        [this]() {return RES(0, &registers.H);},
        [this]() {return RES(0, &registers.L);},
        [this]() {return RES(0, nullptr);},
        [this]() {return RES(0, &registers.A);},
        [this]() {return RES(1, &registers.B);},
        [this]() {return RES(1, &registers.C);},
        [this]() {return RES(1, &registers.D);},
        [this]() {return RES(1, &registers.E);},
        [this]() {return RES(1, &registers.H);},
        [this]() {return RES(1, &registers.L);},
        [this]() {return RES(1, nullptr);},
        [this]() {return RES(1, &registers.A);},
        [this]() {return RES(2, &registers.B);},
        [this]() {return RES(2, &registers.C);},
        [this]() {return RES(2, &registers.D);},
        [this]() {return RES(2, &registers.E);},
        [this]() {return RES(2, &registers.H);},
        [this]() {return RES(2, &registers.L);},
        [this]() {return RES(2, nullptr);},
        [this]() {return RES(2, &registers.A);},
        [this]() {return RES(3, &registers.B);},
        [this]() {return RES(3, &registers.C);},
        [this]() {return RES(3, &registers.D);},
        [this]() {return RES(3, &registers.E);},
        [this]() {return RES(3, &registers.H);},
        [this]() {return RES(3, &registers.L);},
        [this]() {return RES(3, nullptr);},
        [this]() {return RES(3, &registers.A);},
        [this]() {return RES(4, &registers.B);},
        [this]() {return RES(4, &registers.C);},
        [this]() {return RES(4, &registers.D);},
        [this]() {return RES(4, &registers.E);},
        [this]() {return RES(4, &registers.H);},
        [this]() {return RES(4, &registers.L);},
        [this]() {return RES(4, nullptr);},
        [this]() {return RES(4, &registers.A);},
        [this]() {return RES(5, &registers.B);},
        [this]() {return RES(5, &registers.C);},
        [this]() {return RES(5, &registers.D);},
        [this]() {return RES(5, &registers.E);},
        [this]() {return RES(5, &registers.H);},
        [this]() {return RES(5, &registers.L);},
        [this]() {return RES(5, nullptr);},
        [this]() {return RES(5, &registers.A);},
        [this]() {return RES(6, &registers.B);},
        [this]() {return RES(6, &registers.C);},
        [this]() {return RES(6, &registers.D);},
        [this]() {return RES(6, &registers.E);},
        [this]() {return RES(6, &registers.H);},
        [this]() {return RES(6, &registers.L);},
        [this]() {return RES(6, nullptr);},
        [this]() {return RES(6, &registers.A);},
        [this]() {return RES(7, &registers.B);},
        [this]() {return RES(7, &registers.C);},
        [this]() {return RES(7, &registers.D);},
        [this]() {return RES(7, &registers.E);},
        [this]() {return RES(7, &registers.H);},
        [this]() {return RES(7, &registers.L);},
        [this]() {return RES(7, nullptr);},
        [this]() {return RES(7, &registers.A);},
        [this]() {return SET(0, &registers.B);},
        [this]() {return SET(0, &registers.C);},
        [this]() {return SET(0, &registers.D);},
        [this]() {return SET(0, &registers.E);},
        [this]() {return SET(0, &registers.H);},
        [this]() {return SET(0, &registers.L);},
        [this]() {return SET(0, nullptr);},
        [this]() {return SET(0, &registers.A);},
        [this]() {return SET(1, &registers.B);},
        [this]() {return SET(1, &registers.C);},
        [this]() {return SET(1, &registers.D);},
        [this]() {return SET(1, &registers.E);},
        [this]() {return SET(1, &registers.H);},
        [this]() {return SET(1, &registers.L);},
        [this]() {return SET(1, nullptr);},
        [this]() {return SET(1, &registers.A);},
        [this]() {return SET(2, &registers.B);},
        [this]() {return SET(2, &registers.C);},
        [this]() {return SET(2, &registers.D);},
        [this]() {return SET(2, &registers.E);},
        [this]() {return SET(2, &registers.H);},
        [this]() {return SET(2, &registers.L);},
        [this]() {return SET(2, nullptr);},
        [this]() {return SET(2, &registers.A);},
        [this]() {return SET(3, &registers.B);},
        [this]() {return SET(3, &registers.C);},
        [this]() {return SET(3, &registers.D);},
        [this]() {return SET(3, &registers.E);},
        [this]() {return SET(3, &registers.H);},
        [this]() {return SET(3, &registers.L);},
        [this]() {return SET(3, nullptr);},
        [this]() {return SET(3, &registers.A);},
        [this]() {return SET(4, &registers.B);},
        [this]() {return SET(4, &registers.C);},
        [this]() {return SET(4, &registers.D);},
        [this]() {return SET(4, &registers.E);},
        [this]() {return SET(4, &registers.H);},
        [this]() {return SET(4, &registers.L);},
        [this]() {return SET(4, nullptr);},
        [this]() {return SET(4, &registers.A);},
        [this]() {return SET(5, &registers.B);},
        [this]() {return SET(5, &registers.C);},
        [this]() {return SET(5, &registers.D);},
        [this]() {return SET(5, &registers.E);},
        [this]() {return SET(5, &registers.H);},
        [this]() {return SET(5, &registers.L);},
        [this]() {return SET(5, nullptr);},
        [this]() {return SET(5, &registers.A);},
        [this]() {return SET(6, &registers.B);},
        [this]() {return SET(6, &registers.C);},
        [this]() {return SET(6, &registers.D);},
        [this]() {return SET(6, &registers.E);},
        [this]() {return SET(6, &registers.H);},
        [this]() {return SET(6, &registers.L);},
        [this]() {return SET(6, nullptr);},
        [this]() {return SET(6, &registers.A);},
        [this]() {return SET(7, &registers.B);},
        [this]() {return SET(7, &registers.C);},
        [this]() {return SET(7, &registers.D);},
        [this]() {return SET(7, &registers.E);},
        [this]() {return SET(7, &registers.H);},
        [this]() {return SET(7, &registers.L);},
        [this]() {return SET(7, nullptr);},
        [this]() {return SET(7, &registers.A);},

    };

public:
    inline static const char *opcodeStr[256] = {
            "NOP",
            "LD BC, n16",
            "LD [BC], A",
            "INC BC",
            "INC B",
            "DEC B",
            "LD B, n8",
            "RLCA",
            "LD [a16], SP",
            "ADD HL, BC",
            "LD A, [BC]",
            "DEC BC",
            "INC C",
            "DEC C",
            "LD C, n8",
            "RRCA",
            "STOP n8",
            "LD DE, n16",
            "LD [DE], A",
            "INC DE",
            "INC D",
            "DEC D",
            "LD D, n8",
            "RLA",
            "JR e8",
            "ADD HL, DE",
            "LD A, [DE]",
            "DEC DE",
            "INC E",
            "DEC E",
            "LD E, n8",
            "RRA",
            "JR NZ, e8",
            "LD HL, n16",
            "LD [HL+], A",
            "INC HL",
            "INC H",
            "DEC H",
            "LD H, n8",
            "DAA",
            "JR Z, e8",
            "ADD HL, HL",
            "LD A, [HL+]",
            "DEC HL",
            "INC L",
            "DEC L",
            "LD L, n8",
            "CPL",
            "JR NC, e8",
            "LD SP, n16",
            "LD [HL-], A",
            "INC SP",
            "INC [HL]",
            "DEC [HL]",
            "LD [HL], n8",
            "SCF",
            "JR C, e8",
            "ADD HL, SP",
            "LD A, [HL-]",
            "DEC SP",
            "INC A",
            "DEC A",
            "LD A, n8",
            "CCF",
            "LD B, B",
            "LD B, C",
            "LD B, D",
            "LD B, E",
            "LD B, H",
            "LD B, L",
            "LD B, [HL]",
            "LD B, A",
            "LD C, B",
            "LD C, C",
            "LD C, D",
            "LD C, E",
            "LD C, H",
            "LD C, L",
            "LD C, [HL]",
            "LD C, A",
            "LD D, B",
            "LD D, C",
            "LD D, D",
            "LD D, E",
            "LD D, H",
            "LD D, L",
            "LD D, [HL]",
            "LD D, A",
            "LD E, B",
            "LD E, C",
            "LD E, D",
            "LD E, E",
            "LD E, H",
            "LD E, L",
            "LD E, [HL]",
            "LD E, A",
            "LD H, B",
            "LD H, C",
            "LD H, D",
            "LD H, E",
            "LD H, H",
            "LD H, L",
            "LD H, [HL]",
            "LD H, A",
            "LD L, C",
            "LD L, D",
            "LD L, B",
            "LD L, E",
            "LD L, H",
            "LD L, L",
            "LD L, [HL]",
            "LD L, A",
            "LD [HL], B",
            "LD [HL], C",
            "LD [HL], D",
            "LD [HL], E",
            "LD [HL], H",
            "LD [HL], L",
            "HALT",
            "LD [HL], A",
            "LD A, B",
            "LD A, C",
            "LD A, D",
            "LD A, E",
            "LD A, H",
            "LD A, L",
            "LD A, [HL]",
            "LD A, A",
            "ADD A, B",
            "ADD A, C",
            "ADD A, D",
            "ADD A, E",
            "ADD A, H",
            "ADD A, L",
            "ADD A, [HL]",
            "ADD A, A",
            "ADC A, B",
            "ADC A, C",
            "ADC A, D",
            "ADC A, E",
            "ADC A, H",
            "ADC A, L",
            "ADC A, [HL]",
            "ADC A, A",
            "SUB A, B",
            "SUB A, C",
            "SUB A, D",
            "SUB A, E",
            "SUB A, H",
            "SUB A, L",
            "SUB A, [HL]",
            "SUB A, A",
            "SBC A, B",
            "SBC A, C",
            "SBC A, D",
            "SBC A, E",
            "SBC A, H",
            "SBC A, L",
            "SBC A, [HL]",
            "SBC A, A",
            "AND A, B",
            "AND A, C",
            "AND A, D",
            "AND A, E",
            "AND A, H",
            "AND A, L",
            "AND A, [HL]",
            "AND A, A",
            "XOR A, B",
            "XOR A, C",
            "XOR A, D",
            "XOR A, E",
            "XOR A, H",
            "XOR A, L",
            "XOR A, [HL]",
            "XOR A, A",
            "OR A, B",
            "OR A, C",
            "OR A, D",
            "OR A, E",
            "OR A, H",
            "OR A, L",
            "OR A, [HL]",
            "OR A, A",
            "CP A, B",
            "CP A, C",
            "CP A, D",
            "CP A, E",
            "CP A, H",
            "CP A, L",
            "CP A, [HL]",
            "CP A, A",
            "RET NZ",
            "POP BC",
            "JP NZ, a16",
            "JP a16",
            "CALL NZ, a16",
            "PUSH BC",
            "ADD A, n8",
            "RST 0x0000",
            "RET Z",
            "RET",
            "JP Z, a16",
            "PREFIX",
            "CALL Z, a16",
            "CALL a16",
            "ADC A, n8",
            "RST 0x0008",
            "RET NC",
            "POP DE",
            "JP NC, a16",
            "NIMP",
            "CALL NC, a16",
            "PUSH DE",
            "SUB A, n8",
            "RST 0x0010",
            "RET C",
            "RETI",
            "JP C, a16",
            "NIMP",
            "CALL C, a16",
            "NIMP",
            "SBC A, n8",
            "RST 0x0018",
            "LDH [a8], A",
            "POP HL",
            "LD [C], A",
            "NIMP",
            "NIMP",
            "PUSH HL",
            "AND A, n8",
            "RST 0x0020",
            "ADD SP, e8",
            "JP HL",
            "LD [a16], A",
            "NIMP",
            "NIMP",
            "NIMP",
            "XOR A, n8",
            "RST 0x0028",
            "LDH A, [a8]",
            "POP AF",
            "LD A, [C]",
            "DI",
            "NIMP",
            "PUSH AF",
            "OR A, n8",
            "RST 0x0030",
            "LD HL, SP + e8",
            "LD SP, HL",
            "LD A, [a16]",
            "EI",
            "NIMP",
            "NIMP",
            "CP A, n8",
            "RST 0x0038",
    };

private:
    // Normal instructions
    uint8_t NIMP();
    uint8_t NOP();
    uint8_t LD(uint8_t *reg);
    uint8_t LD(uint8_t *reg1, const uint8_t *reg2);
    uint8_t LD(uint8_t *reg1, uint8_t reg2);
    uint8_t LD(uint8_t reg1, const uint8_t *reg2);
    uint8_t LD(uint16_t *reg1, const uint16_t *reg2, bool addDataToSP = false);
    uint8_t LD(uint16_t reg1, const uint8_t *reg2, bool incrementReg1 = false, bool decrementReg1 = false);
    uint8_t LD(uint8_t *reg1, uint16_t reg2, bool incrementReg2 = false, bool decrementReg2 = false);
    uint8_t INC(uint8_t *reg);
    uint8_t INC(uint16_t *reg);
    uint8_t INC(uint16_t reg);
    uint8_t DEC(uint8_t *reg);
    uint8_t DEC(uint16_t *reg);
    uint8_t DEC(uint16_t reg);
    uint8_t RLCA();
    uint8_t ADD(const uint8_t *reg);
    uint8_t ADD(uint16_t reg);
    uint8_t ADD(uint16_t *reg1, const uint16_t *reg2);
    uint8_t ADC(const uint8_t *reg);
    uint8_t ADC(uint16_t reg);
    uint8_t SUB(const uint8_t *reg);
    uint8_t SUB(uint16_t reg);
    uint8_t SBC(uint8_t *reg1, uint8_t *reg2);
    uint8_t AND(const uint8_t *reg);
    uint8_t AND(uint16_t reg);
    uint8_t XOR(const uint8_t *reg);
    uint8_t XOR(uint16_t reg);
    uint8_t OR(const uint8_t *reg);
    uint8_t OR(uint16_t reg);
    uint8_t CP(const uint8_t *reg);
    uint8_t CP(uint16_t reg);
    uint8_t RRCA();
    uint8_t STOP();
    uint8_t RLA();
    uint8_t JR(const bool *flag, bool invert = false);
    uint8_t RRA();
    uint8_t DAA();
    uint8_t CPL();
    uint8_t SCF();
    uint8_t CCF();
    uint8_t HALT();
    uint8_t RET(const bool *flag, bool invert = false);
    uint8_t POP(uint16_t *reg);
    uint8_t JP(const bool *flag, bool invert = false);
    uint8_t JP(const uint16_t *reg);
    uint8_t CALL(const bool *flag, bool invert = false);
    uint8_t PUSH(const uint16_t *reg);
    uint8_t RST(uint16_t addr);
    uint8_t PREFIX();
    uint8_t RETI();
    uint8_t LDH(const uint8_t *reg);
    uint8_t DI();
    uint8_t EI();

    // Prefixed instruction (using 0xCB PREFIX instruction)
    uint8_t RLC(uint8_t *reg);
    uint8_t RRC(uint8_t *reg);
    uint8_t RL(uint8_t *reg);
    uint8_t RR(uint8_t *reg);
    uint8_t SLA(uint8_t *reg);
    uint8_t SRA(uint8_t *reg);
    uint8_t SWAP(uint8_t *reg);
    uint8_t SRL(uint8_t *reg);
    uint8_t BIT(uint8_t bit, const uint8_t *reg);
    uint8_t RES(uint8_t bit, uint8_t *reg);
    uint8_t SET(uint8_t bit, uint8_t *reg);
};

#endif //EMU_GAMEBOY_SHARPSM83_H
