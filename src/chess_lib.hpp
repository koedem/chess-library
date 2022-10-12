#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "sliders.hpp"

using namespace Chess_Lookup::Fancy;

namespace Chess
{

// *******************
// TYPES DEFINITION
// *******************
#define U64 unsigned long long
#define Score int16_t
#define TimePoint std::chrono::high_resolution_clock
#define MAX_SQ 64
#define DEFAULT_POS std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")

static constexpr int MAX_PLY = 120;
static constexpr int MAX_MOVES = 128;

static constexpr U64 WK_CASTLE_MASK = (1ULL << SQ_F1) | (1ULL << SQ_G1);
static constexpr U64 WQ_CASTLE_MASK = (1ULL << SQ_D1) | (1ULL << SQ_C1) | (1ULL << SQ_B1);

static constexpr U64 BK_CASTLE_MASK = (1ULL << SQ_F8) | (1ULL << SQ_G8);
static constexpr U64 BQ_CASTLE_MASK = (1ULL << SQ_D8) | (1ULL << SQ_C8) | (1ULL << SQ_B8);

static constexpr U64 DEFAULT_CHECKMASK = 18446744073709551615ULL;

// clang-format off

// pre calculated lookup table for knight attacks
static constexpr uint64_t KNIGHT_ATTACKS_TABLE[64] = {
    0x0000000000020400, 0x0000000000050800, 0x00000000000A1100, 0x0000000000142200, 0x0000000000284400,
    0x0000000000508800, 0x0000000000A01000, 0x0000000000402000, 0x0000000002040004, 0x0000000005080008,
    0x000000000A110011, 0x0000000014220022, 0x0000000028440044, 0x0000000050880088, 0x00000000A0100010,
    0x0000000040200020, 0x0000000204000402, 0x0000000508000805, 0x0000000A1100110A, 0x0000001422002214,
    0x0000002844004428, 0x0000005088008850, 0x000000A0100010A0, 0x0000004020002040, 0x0000020400040200,
    0x0000050800080500, 0x00000A1100110A00, 0x0000142200221400, 0x0000284400442800, 0x0000508800885000,
    0x0000A0100010A000, 0x0000402000204000, 0x0002040004020000, 0x0005080008050000, 0x000A1100110A0000,
    0x0014220022140000, 0x0028440044280000, 0x0050880088500000, 0x00A0100010A00000, 0x0040200020400000,
    0x0204000402000000, 0x0508000805000000, 0x0A1100110A000000, 0x1422002214000000, 0x2844004428000000,
    0x5088008850000000, 0xA0100010A0000000, 0x4020002040000000, 0x0400040200000000, 0x0800080500000000,
    0x1100110A00000000, 0x2200221400000000, 0x4400442800000000, 0x8800885000000000, 0x100010A000000000,
    0x2000204000000000, 0x0004020000000000, 0x0008050000000000, 0x00110A0000000000, 0x0022140000000000,
    0x0044280000000000, 0x0088500000000000, 0x0010A00000000000, 0x0020400000000000};

// pre calculated lookup table for king attacks
static constexpr uint64_t KING_ATTACKS_TABLE[64] = {
    0x0000000000000302, 0x0000000000000705, 0x0000000000000E0A, 0x0000000000001C14, 0x0000000000003828,
    0x0000000000007050, 0x000000000000E0A0, 0x000000000000C040, 0x0000000000030203, 0x0000000000070507,
    0x00000000000E0A0E, 0x00000000001C141C, 0x0000000000382838, 0x0000000000705070, 0x0000000000E0A0E0,
    0x0000000000C040C0, 0x0000000003020300, 0x0000000007050700, 0x000000000E0A0E00, 0x000000001C141C00,
    0x0000000038283800, 0x0000000070507000, 0x00000000E0A0E000, 0x00000000C040C000, 0x0000000302030000,
    0x0000000705070000, 0x0000000E0A0E0000, 0x0000001C141C0000, 0x0000003828380000, 0x0000007050700000,
    0x000000E0A0E00000, 0x000000C040C00000, 0x0000030203000000, 0x0000070507000000, 0x00000E0A0E000000,
    0x00001C141C000000, 0x0000382838000000, 0x0000705070000000, 0x0000E0A0E0000000, 0x0000C040C0000000,
    0x0003020300000000, 0x0007050700000000, 0x000E0A0E00000000, 0x001C141C00000000, 0x0038283800000000,
    0x0070507000000000, 0x00E0A0E000000000, 0x00C040C000000000, 0x0302030000000000, 0x0705070000000000,
    0x0E0A0E0000000000, 0x1C141C0000000000, 0x3828380000000000, 0x7050700000000000, 0xE0A0E00000000000,
    0xC040C00000000000, 0x0203000000000000, 0x0507000000000000, 0x0A0E000000000000, 0x141C000000000000,
    0x2838000000000000, 0x5070000000000000, 0xA0E0000000000000, 0x40C0000000000000};

// pre calculated lookup table for pawn attacks
static constexpr uint64_t PAWN_ATTACKS_TABLE[2][64] = {
    // white pawn attacks
    { 0x200, 0x500, 0xa00, 0x1400,
      0x2800, 0x5000, 0xa000, 0x4000,
      0x20000, 0x50000, 0xa0000, 0x140000,
      0x280000, 0x500000, 0xa00000, 0x400000,
      0x2000000, 0x5000000, 0xa000000, 0x14000000,
      0x28000000, 0x50000000, 0xa0000000, 0x40000000,
      0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
      0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
      0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
      0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
      0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
      0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
      0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
      0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
      0x0, 0x0, 0x0, 0x0,
      0x0, 0x0, 0x0, 0x0 },

      // black pawn attacks
      { 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0,
        0x2, 0x5, 0xa, 0x14,
        0x28, 0x50, 0xa0, 0x40,
        0x200, 0x500, 0xa00, 0x1400,
        0x2800, 0x5000, 0xa000, 0x4000,
        0x20000, 0x50000, 0xa0000, 0x140000,
        0x280000, 0x500000, 0xa00000, 0x400000,
        0x2000000, 0x5000000, 0xa000000, 0x14000000,
        0x28000000, 0x50000000, 0xa0000000, 0x40000000,
        0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
        0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
        0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
        0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
        0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
        0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000
      }
};

static constexpr U64 RANDOM_ARRAY[781] = {
    0x9D39247E33776D41, 0x2AF7398005AAA5C7, 0x44DB015024623547, 0x9C15F73E62A76AE2, 0x75834465489C0C89,
    0x3290AC3A203001BF, 0x0FBBAD1F61042279, 0xE83A908FF2FB60CA, 0x0D7E765D58755C10, 0x1A083822CEAFE02D,
    0x9605D5F0E25EC3B0, 0xD021FF5CD13A2ED5, 0x40BDF15D4A672E32, 0x011355146FD56395, 0x5DB4832046F3D9E5,
    0x239F8B2D7FF719CC, 0x05D1A1AE85B49AA1, 0x679F848F6E8FC971, 0x7449BBFF801FED0B, 0x7D11CDB1C3B7ADF0,
    0x82C7709E781EB7CC, 0xF3218F1C9510786C, 0x331478F3AF51BBE6, 0x4BB38DE5E7219443, 0xAA649C6EBCFD50FC,
    0x8DBD98A352AFD40B, 0x87D2074B81D79217, 0x19F3C751D3E92AE1, 0xB4AB30F062B19ABF, 0x7B0500AC42047AC4,
    0xC9452CA81A09D85D, 0x24AA6C514DA27500, 0x4C9F34427501B447, 0x14A68FD73C910841, 0xA71B9B83461CBD93,
    0x03488B95B0F1850F, 0x637B2B34FF93C040, 0x09D1BC9A3DD90A94, 0x3575668334A1DD3B, 0x735E2B97A4C45A23,
    0x18727070F1BD400B, 0x1FCBACD259BF02E7, 0xD310A7C2CE9B6555, 0xBF983FE0FE5D8244, 0x9F74D14F7454A824,
    0x51EBDC4AB9BA3035, 0x5C82C505DB9AB0FA, 0xFCF7FE8A3430B241, 0x3253A729B9BA3DDE, 0x8C74C368081B3075,
    0xB9BC6C87167C33E7, 0x7EF48F2B83024E20, 0x11D505D4C351BD7F, 0x6568FCA92C76A243, 0x4DE0B0F40F32A7B8,
    0x96D693460CC37E5D, 0x42E240CB63689F2F, 0x6D2BDCDAE2919661, 0x42880B0236E4D951, 0x5F0F4A5898171BB6,
    0x39F890F579F92F88, 0x93C5B5F47356388B, 0x63DC359D8D231B78, 0xEC16CA8AEA98AD76, 0x5355F900C2A82DC7,
    0x07FB9F855A997142, 0x5093417AA8A7ED5E, 0x7BCBC38DA25A7F3C, 0x19FC8A768CF4B6D4, 0x637A7780DECFC0D9,
    0x8249A47AEE0E41F7, 0x79AD695501E7D1E8, 0x14ACBAF4777D5776, 0xF145B6BECCDEA195, 0xDABF2AC8201752FC,
    0x24C3C94DF9C8D3F6, 0xBB6E2924F03912EA, 0x0CE26C0B95C980D9, 0xA49CD132BFBF7CC4, 0xE99D662AF4243939,
    0x27E6AD7891165C3F, 0x8535F040B9744FF1, 0x54B3F4FA5F40D873, 0x72B12C32127FED2B, 0xEE954D3C7B411F47,
    0x9A85AC909A24EAA1, 0x70AC4CD9F04F21F5, 0xF9B89D3E99A075C2, 0x87B3E2B2B5C907B1, 0xA366E5B8C54F48B8,
    0xAE4A9346CC3F7CF2, 0x1920C04D47267BBD, 0x87BF02C6B49E2AE9, 0x092237AC237F3859, 0xFF07F64EF8ED14D0,
    0x8DE8DCA9F03CC54E, 0x9C1633264DB49C89, 0xB3F22C3D0B0B38ED, 0x390E5FB44D01144B, 0x5BFEA5B4712768E9,
    0x1E1032911FA78984, 0x9A74ACB964E78CB3, 0x4F80F7A035DAFB04, 0x6304D09A0B3738C4, 0x2171E64683023A08,
    0x5B9B63EB9CEFF80C, 0x506AACF489889342, 0x1881AFC9A3A701D6, 0x6503080440750644, 0xDFD395339CDBF4A7,
    0xEF927DBCF00C20F2, 0x7B32F7D1E03680EC, 0xB9FD7620E7316243, 0x05A7E8A57DB91B77, 0xB5889C6E15630A75,
    0x4A750A09CE9573F7, 0xCF464CEC899A2F8A, 0xF538639CE705B824, 0x3C79A0FF5580EF7F, 0xEDE6C87F8477609D,
    0x799E81F05BC93F31, 0x86536B8CF3428A8C, 0x97D7374C60087B73, 0xA246637CFF328532, 0x043FCAE60CC0EBA0,
    0x920E449535DD359E, 0x70EB093B15B290CC, 0x73A1921916591CBD, 0x56436C9FE1A1AA8D, 0xEFAC4B70633B8F81,
    0xBB215798D45DF7AF, 0x45F20042F24F1768, 0x930F80F4E8EB7462, 0xFF6712FFCFD75EA1, 0xAE623FD67468AA70,
    0xDD2C5BC84BC8D8FC, 0x7EED120D54CF2DD9, 0x22FE545401165F1C, 0xC91800E98FB99929, 0x808BD68E6AC10365,
    0xDEC468145B7605F6, 0x1BEDE3A3AEF53302, 0x43539603D6C55602, 0xAA969B5C691CCB7A, 0xA87832D392EFEE56,
    0x65942C7B3C7E11AE, 0xDED2D633CAD004F6, 0x21F08570F420E565, 0xB415938D7DA94E3C, 0x91B859E59ECB6350,
    0x10CFF333E0ED804A, 0x28AED140BE0BB7DD, 0xC5CC1D89724FA456, 0x5648F680F11A2741, 0x2D255069F0B7DAB3,
    0x9BC5A38EF729ABD4, 0xEF2F054308F6A2BC, 0xAF2042F5CC5C2858, 0x480412BAB7F5BE2A, 0xAEF3AF4A563DFE43,
    0x19AFE59AE451497F, 0x52593803DFF1E840, 0xF4F076E65F2CE6F0, 0x11379625747D5AF3, 0xBCE5D2248682C115,
    0x9DA4243DE836994F, 0x066F70B33FE09017, 0x4DC4DE189B671A1C, 0x51039AB7712457C3, 0xC07A3F80C31FB4B4,
    0xB46EE9C5E64A6E7C, 0xB3819A42ABE61C87, 0x21A007933A522A20, 0x2DF16F761598AA4F, 0x763C4A1371B368FD,
    0xF793C46702E086A0, 0xD7288E012AEB8D31, 0xDE336A2A4BC1C44B, 0x0BF692B38D079F23, 0x2C604A7A177326B3,
    0x4850E73E03EB6064, 0xCFC447F1E53C8E1B, 0xB05CA3F564268D99, 0x9AE182C8BC9474E8, 0xA4FC4BD4FC5558CA,
    0xE755178D58FC4E76, 0x69B97DB1A4C03DFE, 0xF9B5B7C4ACC67C96, 0xFC6A82D64B8655FB, 0x9C684CB6C4D24417,
    0x8EC97D2917456ED0, 0x6703DF9D2924E97E, 0xC547F57E42A7444E, 0x78E37644E7CAD29E, 0xFE9A44E9362F05FA,
    0x08BD35CC38336615, 0x9315E5EB3A129ACE, 0x94061B871E04DF75, 0xDF1D9F9D784BA010, 0x3BBA57B68871B59D,
    0xD2B7ADEEDED1F73F, 0xF7A255D83BC373F8, 0xD7F4F2448C0CEB81, 0xD95BE88CD210FFA7, 0x336F52F8FF4728E7,
    0xA74049DAC312AC71, 0xA2F61BB6E437FDB5, 0x4F2A5CB07F6A35B3, 0x87D380BDA5BF7859, 0x16B9F7E06C453A21,
    0x7BA2484C8A0FD54E, 0xF3A678CAD9A2E38C, 0x39B0BF7DDE437BA2, 0xFCAF55C1BF8A4424, 0x18FCF680573FA594,
    0x4C0563B89F495AC3, 0x40E087931A00930D, 0x8CFFA9412EB642C1, 0x68CA39053261169F, 0x7A1EE967D27579E2,
    0x9D1D60E5076F5B6F, 0x3810E399B6F65BA2, 0x32095B6D4AB5F9B1, 0x35CAB62109DD038A, 0xA90B24499FCFAFB1,
    0x77A225A07CC2C6BD, 0x513E5E634C70E331, 0x4361C0CA3F692F12, 0xD941ACA44B20A45B, 0x528F7C8602C5807B,
    0x52AB92BEB9613989, 0x9D1DFA2EFC557F73, 0x722FF175F572C348, 0x1D1260A51107FE97, 0x7A249A57EC0C9BA2,
    0x04208FE9E8F7F2D6, 0x5A110C6058B920A0, 0x0CD9A497658A5698, 0x56FD23C8F9715A4C, 0x284C847B9D887AAE,
    0x04FEABFBBDB619CB, 0x742E1E651C60BA83, 0x9A9632E65904AD3C, 0x881B82A13B51B9E2, 0x506E6744CD974924,
    0xB0183DB56FFC6A79, 0x0ED9B915C66ED37E, 0x5E11E86D5873D484, 0xF678647E3519AC6E, 0x1B85D488D0F20CC5,
    0xDAB9FE6525D89021, 0x0D151D86ADB73615, 0xA865A54EDCC0F019, 0x93C42566AEF98FFB, 0x99E7AFEABE000731,
    0x48CBFF086DDF285A, 0x7F9B6AF1EBF78BAF, 0x58627E1A149BBA21, 0x2CD16E2ABD791E33, 0xD363EFF5F0977996,
    0x0CE2A38C344A6EED, 0x1A804AADB9CFA741, 0x907F30421D78C5DE, 0x501F65EDB3034D07, 0x37624AE5A48FA6E9,
    0x957BAF61700CFF4E, 0x3A6C27934E31188A, 0xD49503536ABCA345, 0x088E049589C432E0, 0xF943AEE7FEBF21B8,
    0x6C3B8E3E336139D3, 0x364F6FFA464EE52E, 0xD60F6DCEDC314222, 0x56963B0DCA418FC0, 0x16F50EDF91E513AF,
    0xEF1955914B609F93, 0x565601C0364E3228, 0xECB53939887E8175, 0xBAC7A9A18531294B, 0xB344C470397BBA52,
    0x65D34954DAF3CEBD, 0xB4B81B3FA97511E2, 0xB422061193D6F6A7, 0x071582401C38434D, 0x7A13F18BBEDC4FF5,
    0xBC4097B116C524D2, 0x59B97885E2F2EA28, 0x99170A5DC3115544, 0x6F423357E7C6A9F9, 0x325928EE6E6F8794,
    0xD0E4366228B03343, 0x565C31F7DE89EA27, 0x30F5611484119414, 0xD873DB391292ED4F, 0x7BD94E1D8E17DEBC,
    0xC7D9F16864A76E94, 0x947AE053EE56E63C, 0xC8C93882F9475F5F, 0x3A9BF55BA91F81CA, 0xD9A11FBB3D9808E4,
    0x0FD22063EDC29FCA, 0xB3F256D8ACA0B0B9, 0xB03031A8B4516E84, 0x35DD37D5871448AF, 0xE9F6082B05542E4E,
    0xEBFAFA33D7254B59, 0x9255ABB50D532280, 0xB9AB4CE57F2D34F3, 0x693501D628297551, 0xC62C58F97DD949BF,
    0xCD454F8F19C5126A, 0xBBE83F4ECC2BDECB, 0xDC842B7E2819E230, 0xBA89142E007503B8, 0xA3BC941D0A5061CB,
    0xE9F6760E32CD8021, 0x09C7E552BC76492F, 0x852F54934DA55CC9, 0x8107FCCF064FCF56, 0x098954D51FFF6580,
    0x23B70EDB1955C4BF, 0xC330DE426430F69D, 0x4715ED43E8A45C0A, 0xA8D7E4DAB780A08D, 0x0572B974F03CE0BB,
    0xB57D2E985E1419C7, 0xE8D9ECBE2CF3D73F, 0x2FE4B17170E59750, 0x11317BA87905E790, 0x7FBF21EC8A1F45EC,
    0x1725CABFCB045B00, 0x964E915CD5E2B207, 0x3E2B8BCBF016D66D, 0xBE7444E39328A0AC, 0xF85B2B4FBCDE44B7,
    0x49353FEA39BA63B1, 0x1DD01AAFCD53486A, 0x1FCA8A92FD719F85, 0xFC7C95D827357AFA, 0x18A6A990C8B35EBD,
    0xCCCB7005C6B9C28D, 0x3BDBB92C43B17F26, 0xAA70B5B4F89695A2, 0xE94C39A54A98307F, 0xB7A0B174CFF6F36E,
    0xD4DBA84729AF48AD, 0x2E18BC1AD9704A68, 0x2DE0966DAF2F8B1C, 0xB9C11D5B1E43A07E, 0x64972D68DEE33360,
    0x94628D38D0C20584, 0xDBC0D2B6AB90A559, 0xD2733C4335C6A72F, 0x7E75D99D94A70F4D, 0x6CED1983376FA72B,
    0x97FCAACBF030BC24, 0x7B77497B32503B12, 0x8547EDDFB81CCB94, 0x79999CDFF70902CB, 0xCFFE1939438E9B24,
    0x829626E3892D95D7, 0x92FAE24291F2B3F1, 0x63E22C147B9C3403, 0xC678B6D860284A1C, 0x5873888850659AE7,
    0x0981DCD296A8736D, 0x9F65789A6509A440, 0x9FF38FED72E9052F, 0xE479EE5B9930578C, 0xE7F28ECD2D49EECD,
    0x56C074A581EA17FE, 0x5544F7D774B14AEF, 0x7B3F0195FC6F290F, 0x12153635B2C0CF57, 0x7F5126DBBA5E0CA7,
    0x7A76956C3EAFB413, 0x3D5774A11D31AB39, 0x8A1B083821F40CB4, 0x7B4A38E32537DF62, 0x950113646D1D6E03,
    0x4DA8979A0041E8A9, 0x3BC36E078F7515D7, 0x5D0A12F27AD310D1, 0x7F9D1A2E1EBE1327, 0xDA3A361B1C5157B1,
    0xDCDD7D20903D0C25, 0x36833336D068F707, 0xCE68341F79893389, 0xAB9090168DD05F34, 0x43954B3252DC25E5,
    0xB438C2B67F98E5E9, 0x10DCD78E3851A492, 0xDBC27AB5447822BF, 0x9B3CDB65F82CA382, 0xB67B7896167B4C84,
    0xBFCED1B0048EAC50, 0xA9119B60369FFEBD, 0x1FFF7AC80904BF45, 0xAC12FB171817EEE7, 0xAF08DA9177DDA93D,
    0x1B0CAB936E65C744, 0xB559EB1D04E5E932, 0xC37B45B3F8D6F2BA, 0xC3A9DC228CAAC9E9, 0xF3B8B6675A6507FF,
    0x9FC477DE4ED681DA, 0x67378D8ECCEF96CB, 0x6DD856D94D259236, 0xA319CE15B0B4DB31, 0x073973751F12DD5E,
    0x8A8E849EB32781A5, 0xE1925C71285279F5, 0x74C04BF1790C0EFE, 0x4DDA48153C94938A, 0x9D266D6A1CC0542C,
    0x7440FB816508C4FE, 0x13328503DF48229F, 0xD6BF7BAEE43CAC40, 0x4838D65F6EF6748F, 0x1E152328F3318DEA,
    0x8F8419A348F296BF, 0x72C8834A5957B511, 0xD7A023A73260B45C, 0x94EBC8ABCFB56DAE, 0x9FC10D0F989993E0,
    0xDE68A2355B93CAE6, 0xA44CFE79AE538BBE, 0x9D1D84FCCE371425, 0x51D2B1AB2DDFB636, 0x2FD7E4B9E72CD38C,
    0x65CA5B96B7552210, 0xDD69A0D8AB3B546D, 0x604D51B25FBF70E2, 0x73AA8A564FB7AC9E, 0x1A8C1E992B941148,
    0xAAC40A2703D9BEA0, 0x764DBEAE7FA4F3A6, 0x1E99B96E70A9BE8B, 0x2C5E9DEB57EF4743, 0x3A938FEE32D29981,
    0x26E6DB8FFDF5ADFE, 0x469356C504EC9F9D, 0xC8763C5B08D1908C, 0x3F6C6AF859D80055, 0x7F7CC39420A3A545,
    0x9BFB227EBDF4C5CE, 0x89039D79D6FC5C5C, 0x8FE88B57305E2AB6, 0xA09E8C8C35AB96DE, 0xFA7E393983325753,
    0xD6B6D0ECC617C699, 0xDFEA21EA9E7557E3, 0xB67C1FA481680AF8, 0xCA1E3785A9E724E5, 0x1CFC8BED0D681639,
    0xD18D8549D140CAEA, 0x4ED0FE7E9DC91335, 0xE4DBF0634473F5D2, 0x1761F93A44D5AEFE, 0x53898E4C3910DA55,
    0x734DE8181F6EC39A, 0x2680B122BAA28D97, 0x298AF231C85BAFAB, 0x7983EED3740847D5, 0x66C1A2A1A60CD889,
    0x9E17E49642A3E4C1, 0xEDB454E7BADC0805, 0x50B704CAB602C329, 0x4CC317FB9CDDD023, 0x66B4835D9EAFEA22,
    0x219B97E26FFC81BD, 0x261E4E4C0A333A9D, 0x1FE2CCA76517DB90, 0xD7504DFA8816EDBB, 0xB9571FA04DC089C8,
    0x1DDC0325259B27DE, 0xCF3F4688801EB9AA, 0xF4F5D05C10CAB243, 0x38B6525C21A42B0E, 0x36F60E2BA4FA6800,
    0xEB3593803173E0CE, 0x9C4CD6257C5A3603, 0xAF0C317D32ADAA8A, 0x258E5A80C7204C4B, 0x8B889D624D44885D,
    0xF4D14597E660F855, 0xD4347F66EC8941C3, 0xE699ED85B0DFB40D, 0x2472F6207C2D0484, 0xC2A1E7B5B459AEB5,
    0xAB4F6451CC1D45EC, 0x63767572AE3D6174, 0xA59E0BD101731A28, 0x116D0016CB948F09, 0x2CF9C8CA052F6E9F,
    0x0B090A7560A968E3, 0xABEEDDB2DDE06FF1, 0x58EFC10B06A2068D, 0xC6E57A78FBD986E0, 0x2EAB8CA63CE802D7,
    0x14A195640116F336, 0x7C0828DD624EC390, 0xD74BBE77E6116AC7, 0x804456AF10F5FB53, 0xEBE9EA2ADF4321C7,
    0x03219A39EE587A30, 0x49787FEF17AF9924, 0xA1E9300CD8520548, 0x5B45E522E4B1B4EF, 0xB49C3B3995091A36,
    0xD4490AD526F14431, 0x12A8F216AF9418C2, 0x001F837CC7350524, 0x1877B51E57A764D5, 0xA2853B80F17F58EE,
    0x993E1DE72D36D310, 0xB3598080CE64A656, 0x252F59CF0D9F04BB, 0xD23C8E176D113600, 0x1BDA0492E7E4586E,
    0x21E0BD5026C619BF, 0x3B097ADAF088F94E, 0x8D14DEDB30BE846E, 0xF95CFFA23AF5F6F4, 0x3871700761B3F743,
    0xCA672B91E9E4FA16, 0x64C8E531BFF53B55, 0x241260ED4AD1E87D, 0x106C09B972D2E822, 0x7FBA195410E5CA30,
    0x7884D9BC6CB569D8, 0x0647DFEDCD894A29, 0x63573FF03E224774, 0x4FC8E9560F91B123, 0x1DB956E450275779,
    0xB8D91274B9E9D4FB, 0xA2EBEE47E2FBFCE1, 0xD9F1F30CCD97FB09, 0xEFED53D75FD64E6B, 0x2E6D02C36017F67F,
    0xA9AA4D20DB084E9B, 0xB64BE8D8B25396C1, 0x70CB6AF7C2D5BCF0, 0x98F076A4F7A2322E, 0xBF84470805E69B5F,
    0x94C3251F06F90CF3, 0x3E003E616A6591E9, 0xB925A6CD0421AFF3, 0x61BDD1307C66E300, 0xBF8D5108E27E0D48,
    0x240AB57A8B888B20, 0xFC87614BAF287E07, 0xEF02CDD06FFDB432, 0xA1082C0466DF6C0A, 0x8215E577001332C8,
    0xD39BB9C3A48DB6CF, 0x2738259634305C14, 0x61CF4F94C97DF93D, 0x1B6BACA2AE4E125B, 0x758F450C88572E0B,
    0x959F587D507A8359, 0xB063E962E045F54D, 0x60E8ED72C0DFF5D1, 0x7B64978555326F9F, 0xFD080D236DA814BA,
    0x8C90FD9B083F4558, 0x106F72FE81E2C590, 0x7976033A39F7D952, 0xA4EC0132764CA04B, 0x733EA705FAE4FA77,
    0xB4D8F77BC3E56167, 0x9E21F4F903B33FD9, 0x9D765E419FB69F6D, 0xD30C088BA61EA5EF, 0x5D94337FBFAF7F5B,
    0x1A4E4822EB4D7A59, 0x6FFE73E81B637FB3, 0xDDF957BC36D8B9CA, 0x64D0E29EEA8838B3, 0x08DD9BDFD96B9F63,
    0x087E79E5A57D1D13, 0xE328E230E3E2B3FB, 0x1C2559E30F0946BE, 0x720BF5F26F4D2EAA, 0xB0774D261CC609DB,
    0x443F64EC5A371195, 0x4112CF68649A260E, 0xD813F2FAB7F5C5CA, 0x660D3257380841EE, 0x59AC2C7873F910A3,
    0xE846963877671A17, 0x93B633ABFA3469F8, 0xC0C0F5A60EF4CDCF, 0xCAF21ECD4377B28C, 0x57277707199B8175,
    0x506C11B9D90E8B1D, 0xD83CC2687A19255F, 0x4A29C6465A314CD1, 0xED2DF21216235097, 0xB5635C95FF7296E2,
    0x22AF003AB672E811, 0x52E762596BF68235, 0x9AEBA33AC6ECC6B0, 0x944F6DE09134DFB6, 0x6C47BEC883A7DE39,
    0x6AD047C430A12104, 0xA5B1CFDBA0AB4067, 0x7C45D833AFF07862, 0x5092EF950A16DA0B, 0x9338E69C052B8E7B,
    0x455A4B4CFE30E3F5, 0x6B02E63195AD0CF8, 0x6B17B224BAD6BF27, 0xD1E0CCD25BB9C169, 0xDE0C89A556B9AE70,
    0x50065E535A213CF6, 0x9C1169FA2777B874, 0x78EDEFD694AF1EED, 0x6DC93D9526A50E68, 0xEE97F453F06791ED,
    0x32AB0EDB696703D3, 0x3A6853C7E70757A7, 0x31865CED6120F37D, 0x67FEF95D92607890, 0x1F2B1D1F15F6DC9C,
    0xB69E38A8965C6B65, 0xAA9119FF184CCCF4, 0xF43C732873F24C13, 0xFB4A3D794A9A80D2, 0x3550C2321FD6109C,
    0x371F77E76BB8417E, 0x6BFA9AAE5EC05779, 0xCD04F3FF001A4778, 0xE3273522064480CA, 0x9F91508BFFCFC14A,
    0x049A7F41061A9E60, 0xFCB6BE43A9F2FE9B, 0x08DE8A1C7797DA9B, 0x8F9887E6078735A1, 0xB5B4071DBFC73A66,
    0x230E343DFBA08D33, 0x43ED7F5A0FAE657D, 0x3A88A0FBBCB05C63, 0x21874B8B4D2DBC4F, 0x1BDEA12E35F6A8C9,
    0x53C065C6C8E63528, 0xE34A1D250E7A8D6B, 0xD6B04D3B7651DD7E, 0x5E90277E7CB39E2D, 0x2C046F22062DC67D,
    0xB10BB459132D0A26, 0x3FA9DDFB67E2F199, 0x0E09B88E1914F7AF, 0x10E8B35AF3EEAB37, 0x9EEDECA8E272B933,
    0xD4C718BC4AE8AE5F, 0x81536D601170FC20, 0x91B534F885818A06, 0xEC8177F83F900978, 0x190E714FADA5156E,
    0xB592BF39B0364963, 0x89C350C893AE7DC1, 0xAC042E70F8B383F2, 0xB49B52E587A1EE60, 0xFB152FE3FF26DA89,
    0x3E666E6F69AE2C15, 0x3B544EBE544C19F9, 0xE805A1E290CF2456, 0x24B33C9D7ED25117, 0xE74733427B72F0C1,
    0x0A804D18B7097475, 0x57E3306D881EDB4F, 0x4AE7D6A36EB5DBCB, 0x2D8D5432157064C8, 0xD1E649DE1E7F268B,
    0x8A328A1CEDFE552C, 0x07A3AEC79624C7DA, 0x84547DDC3E203C94, 0x990A98FD5071D263, 0x1A4FF12616EEFC89,
    0xF6F7FD1431714200, 0x30C05B1BA332F41C, 0x8D2636B81555A786, 0x46C9FEB55D120902, 0xCCEC0A73B49C9921,
    0x4E9D2827355FC492, 0x19EBB029435DCB0F, 0x4659D2B743848A2C, 0x963EF2C96B33BE31, 0x74F85198B05A2E7D,
    0x5A0F544DD2B1FB18, 0x03727073C2E134B1, 0xC7F6AA2DE59AEA61, 0x352787BAA0D7C22F, 0x9853EAB63B5E0B35,
    0xABBDCDD7ED5C0860, 0xCF05DAF5AC8D77B0, 0x49CAD48CEBF4A71E, 0x7A4C10EC2158C4A6, 0xD9E92AA246BF719E,
    0x13AE978D09FE5557, 0x730499AF921549FF, 0x4E4B705B92903BA4, 0xFF577222C14F0A3A, 0x55B6344CF97AAFAE,
    0xB862225B055B6960, 0xCAC09AFBDDD2CDB4, 0xDAF8E9829FE96B5F, 0xB5FDFC5D3132C498, 0x310CB380DB6F7503,
    0xE87FBB46217A360E, 0x2102AE466EBB1148, 0xF8549E1A3AA5E00D, 0x07A69AFDCC42261A, 0xC4C118BFE78FEAAE,
    0xF9F4892ED96BD438, 0x1AF3DBE25D8F45DA, 0xF5B4B0B0D2DEEEB4, 0x962ACEEFA82E1C84, 0x046E3ECAAF453CE9,
    0xF05D129681949A4C, 0x964781CE734B3C84, 0x9C2ED44081CE5FBD, 0x522E23F3925E319E, 0x177E00F9FC32F791,
    0x2BC60A63A6F3B3F2, 0x222BBFAE61725606, 0x486289DDCC3D6780, 0x7DC7785B8EFDFC80, 0x8AF38731C02BA980,
    0x1FAB64EA29A2DDF7, 0xE4D9429322CD065A, 0x9DA058C67844F20C, 0x24C0E332B70019B0, 0x233003B5A6CFE6AD,
    0xD586BD01C5C217F6, 0x5E5637885F29BC2B, 0x7EBA726D8C94094B, 0x0A56A5F0BFE39272, 0xD79476A84EE20D06,
    0x9E4C1269BAA4BF37, 0x17EFEE45B0DEE640, 0x1D95B0A5FCF90BC6, 0x93CBE0B699C2585D, 0x65FA4F227A2B6D79,
    0xD5F9E858292504D5, 0xC2B5A03F71471A6F, 0x59300222B4561E00, 0xCE2F8642CA0712DC, 0x7CA9723FBB2E8988,
    0x2785338347F2BA08, 0xC61BB3A141E50E8C, 0x150F361DAB9DEC26, 0x9F6A419D382595F4, 0x64A53DC924FE7AC9,
    0x142DE49FFF7A7C3D, 0x0C335248857FA9E7, 0x0A9C32D5EAE45305, 0xE6C42178C4BBB92E, 0x71F1CE2490D20B07,
    0xF1BCC3D275AFE51A, 0xE728E8C83C334074, 0x96FBF83A12884624, 0x81A1549FD6573DA5, 0x5FA7867CAF35E149,
    0x56986E2EF3ED091B, 0x917F1DD5F8886C61, 0xD20D8C88C8FFE65F, 0x31D71DCE64B2C310, 0xF165B587DF898190,
    0xA57E6339DD2CF3A0, 0x1EF6E6DBB1961EC9, 0x70CC73D90BC26E24, 0xE21A6B35DF0C3AD7, 0x003A93D8B2806962,
    0x1C99DED33CB890A1, 0xCF3145DE0ADD4289, 0xD0E4427A5514FB72, 0x77C621CC9FB3A483, 0x67A34DAC4356550B,
    0xF8D626AAAF278509};

static constexpr U64 castlingKey[16] = {0,
                                        RANDOM_ARRAY[768],
                                        RANDOM_ARRAY[768 + 1],
                                        RANDOM_ARRAY[768] ^ RANDOM_ARRAY[768 + 1],
                                        RANDOM_ARRAY[768 + 2],
                                        RANDOM_ARRAY[768] ^ RANDOM_ARRAY[768 + 2],
                                        RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 2],
                                        RANDOM_ARRAY[768] ^ RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 2],
                                        RANDOM_ARRAY[768 + 3],
                                        RANDOM_ARRAY[768] ^ RANDOM_ARRAY[768 + 3],
                                        RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 3],
                                        RANDOM_ARRAY[768] ^ RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 3],
                                        RANDOM_ARRAY[768 + 3] ^ RANDOM_ARRAY[768 + 2],
                                        RANDOM_ARRAY[768 + 3] ^ RANDOM_ARRAY[768 + 2] ^ RANDOM_ARRAY[768],
                                        RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 2] ^ RANDOM_ARRAY[768 + 3],
                                        RANDOM_ARRAY[768 + 1] ^ RANDOM_ARRAY[768 + 2] ^ RANDOM_ARRAY[768 + 3] ^
                                            RANDOM_ARRAY[768]};

// clang-format on

static constexpr int hash_piece[12] = {1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10};

/// @brief convert a piece to a piecetype
static constexpr PieceType PieceToPieceType[12] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
                                                   PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

// file masks

/// @brief Bitboard of all squares
static constexpr U64 MASK_FILE[8] = {
    0x101010101010101,  0x202020202020202,  0x404040404040404,  0x808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

// rank masks

/// @brief Bitboard of all ranks
static constexpr U64 MASK_RANK[8] = {0xff,         0xff00,         0xff0000,         0xff000000,
                                     0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000};

// diagonal masks
static constexpr U64 MASK_DIAGONAL[15] = {
    0x80,
    0x8040,
    0x804020,
    0x80402010,
    0x8040201008,
    0x804020100804,
    0x80402010080402,
    0x8040201008040201,
    0x4020100804020100,
    0x2010080402010000,
    0x1008040201000000,
    0x804020100000000,
    0x402010000000000,
    0x201000000000000,
    0x100000000000000,
};

// anti-diagonal masks
static constexpr U64 MASK_ANTI_DIAGONAL[15] = {0x1,
                                               0x102,
                                               0x10204,
                                               0x1020408,
                                               0x102040810,
                                               0x10204081020,
                                               0x1020408102040,
                                               0x102040810204080,
                                               0x204081020408000,
                                               0x408102040800000,
                                               0x810204080000000,
                                               0x1020408000000000,
                                               0x2040800000000000,
                                               0x4080000000000000,
                                               0x8000000000000000};

enum Movetype : uint8_t
{
    ALL,
    CAPTURE,
    QUIET
};

enum Color : uint8_t
{
    White,
    Black,
    NO_COLOR
};

constexpr Color operator~(Color C)
{
    return Color(C ^ Black);
}

enum Phase : int
{
    MG,
    EG
};

enum Piece : uint8_t
{
    WhitePawn,
    WhiteKnight,
    WhiteBishop,
    WhiteRook,
    WhiteQueen,
    WhiteKing,
    BlackPawn,
    BlackKnight,
    BlackBishop,
    BlackRook,
    BlackQueen,
    BlackKing,
    None
};

enum PieceType : uint8_t
{
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING,
    NONETYPE
};

// clang-format off
enum Square : uint8_t {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    NO_SQ
};

/// @brief convert a square number to a string
const std::string squareToString[64] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
};

// clang-format on

enum CastlingRight : uint8_t
{
    wk = 1,
    wq = 2,
    bk = 4,
    bq = 8
};

enum Direction : int8_t
{
    NORTH = 8,
    WEST = -1,
    SOUTH = -8,
    EAST = 1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_WEST = -9,
    SOUTH_EAST = -7
};

enum Move : uint16_t
{
    NO_MOVE = 0,
    NULL_MOVE = 65
};

static std::unordered_map<Piece, char> pieceToChar({{WhitePawn, 'P'},
                                                    {WhiteKnight, 'N'},
                                                    {WhiteBishop, 'B'},
                                                    {WhiteRook, 'R'},
                                                    {WhiteQueen, 'Q'},
                                                    {WhiteKing, 'K'},
                                                    {BlackPawn, 'p'},
                                                    {BlackKnight, 'n'},
                                                    {BlackBishop, 'b'},
                                                    {BlackRook, 'r'},
                                                    {BlackQueen, 'q'},
                                                    {BlackKing, 'k'},
                                                    {None, '.'}});

static std::unordered_map<char, Piece> charToPiece({{'P', WhitePawn},
                                                    {'N', WhiteKnight},
                                                    {'B', WhiteBishop},
                                                    {'R', WhiteRook},
                                                    {'Q', WhiteQueen},
                                                    {'K', WhiteKing},
                                                    {'p', BlackPawn},
                                                    {'n', BlackKnight},
                                                    {'b', BlackBishop},
                                                    {'r', BlackRook},
                                                    {'q', BlackQueen},
                                                    {'k', BlackKing},
                                                    {'.', None}});

static std::unordered_map<PieceType, char> PieceTypeToPromPiece(
    {{KNIGHT, 'n'}, {BISHOP, 'b'}, {ROOK, 'r'}, {QUEEN, 'q'}});

static std::unordered_map<char, PieceType> pieceToInt(
    {{'n', KNIGHT}, {'b', BISHOP}, {'r', ROOK}, {'q', QUEEN}, {'N', KNIGHT}, {'B', BISHOP}, {'R', ROOK}, {'Q', QUEEN}});

static std::unordered_map<Square, CastlingRight> castlingMapRook({{SQ_A1, wq}, {SQ_H1, wk}, {SQ_A8, bq}, {SQ_H8, bk}});

inline Square from(Move move)
{
    return Square(move & 0b111111);
}

inline Square to(Move move)
{
    return Square((move & 0b111111000000) >> 6);
}

inline PieceType piece(Move move)
{
    return PieceType((move & 0b111000000000000) >> 12);
}

inline bool promoted(Move move)
{
    return bool((move & 0b1000000000000000) >> 15);
}

inline Move make(PieceType piece = NONETYPE, Square source = NO_SQ, Square target = NO_SQ, bool promoted = false)
{
    return Move((uint16_t)source | (uint16_t)target << 6 | (uint16_t)piece << 12 | (uint16_t)promoted << 15);
}

template <PieceType piece, bool promoted> Move make(Square source = NO_SQ, Square target = NO_SQ)
{
    return Move((uint16_t)source | (uint16_t)target << 6 | (uint16_t)piece << 12 | (uint16_t)promoted << 15);
}

struct State
{
    Square enPassant{};
    uint8_t castling{};
    uint8_t halfMove{};
    Piece capturedPiece = None;
    State(Square enpassantCopy = {}, uint8_t castlingRightsCopy = {}, uint8_t halfMoveCopy = {},
          Piece capturedPieceCopy = None)
        : enPassant(enpassantCopy), castling(castlingRightsCopy), halfMove(halfMoveCopy),
          capturedPiece(capturedPieceCopy)
    {
    }
};

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

PACK(struct ExtMove {
    int value = -100'000;
    Move move;
});

inline constexpr bool operator==(const ExtMove &a, const ExtMove &b)
{
    return a.move == b.move;
}

inline constexpr bool operator>(const ExtMove &a, const ExtMove &b)
{
    return a.value > b.value;
}

inline constexpr bool operator<(const ExtMove &a, const ExtMove &b)
{
    return a.value < b.value;
}

struct Movelist
{
    ExtMove list[MAX_MOVES] = {};
    uint8_t size = 0;

    inline void Add(Move move)
    {
        list[size].move = move;
        list[size].value = 0;
        size++;
    }

    inline constexpr ExtMove &operator[](int i)
    {
        return list[i];
    }
};

// *******************
// INTRINSIC FUNCTIONS
// *******************

// Compiler specific functions, taken from Stockfish https://github.com/official-stockfish/Stockfish
#if defined(__GNUC__) // GCC, Clang, ICC

inline Square bsf(U64 b)
{
    if (!b)
        return NO_SQ;
    return Square(__builtin_ctzll(b));
}

inline Square bsr(U64 b)
{
    if (!b)
        return NO_SQ;
    return Square(63 ^ __builtin_clzll(b));
}

#elif defined(_MSC_VER) // MSVC

#ifdef _WIN64 // MSVC, WIN64
#include <intrin.h>
inline Square bsf(U64 b)
{
    unsigned long idx;
    _BitScanForward64(&idx, b);
    return (Square)idx;
}

inline Square bsr(U64 b)
{
    unsigned long idx;
    _BitScanReverse64(&idx, b);
    return (Square)idx;
}

#else // MSVC, WIN32
#include <intrin.h>
inline Square bsf(U64 b)
{
    unsigned long idx;

    if (b & 0xffffffff)
    {
        _BitScanForward(&idx, int32_t(b));
        return Square(idx);
    }
    else
    {
        _BitScanForward(&idx, int32_t(b >> 32));
        return Square(idx + 32);
    }
}

inline Square bsr(U64 b)
{
    unsigned long idx;

    if (b >> 32)
    {
        _BitScanReverse(&idx, int32_t(b >> 32));
        return Square(idx + 32);
    }
    else
    {
        _BitScanReverse(&idx, int32_t(b));
        return Square(idx);
    }
}

#endif

#else // Compiler is neither GCC nor MSVC compatible

#error "Compiler not supported."

#endif

inline uint8_t popcount(U64 mask)
{
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)

    return (uint8_t)_mm_popcnt_u64(mask);

#else // Assumed gcc or compatible compiler

    return __builtin_popcountll(mask);

#endif
}

/// @brief return the lsb and remove it
/// @param mask
/// @return
inline Square poplsb(U64 &mask)
{
    int8_t s = bsf(mask);
    mask &= mask - 1;
    // mask = _blsr_u64(mask);
    return Square(s);
}

/// @brief splits a string into multiple parts, delimiter is whitespace
/// @param fen
/// @return std::vector
inline std::vector<std::string> splitInput(std::string fen)
{
    std::stringstream fen_stream(fen);
    std::string segment;
    std::vector<std::string> seglist;

    while (std::getline(fen_stream, segment, ' '))
    {
        seglist.push_back(segment);
    }
    return seglist;
}

/// @brief file a = 0, file h = 7
/// @param sq
/// @return
inline uint8_t square_file(Square sq)
{
    return sq & 7;
}

/// @brief rank 1 = 0 rank 8 = 7
/// @param sq
/// @return
inline uint8_t square_rank(Square sq)
{
    return sq >> 3;
}

/// @brief distance between two squares
/// @param a
/// @param b
/// @return
inline uint8_t square_distance(Square a, Square b)
{
    return std::max(std::abs(square_file(a) - square_file(b)), std::abs(square_rank(a) - square_rank(b)));
}

inline uint8_t diagonal_of(Square sq)
{
    return 7 + square_rank(sq) - square_file(sq);
}

inline uint8_t anti_diagonal_of(Square sq)
{
    return square_rank(sq) + square_file(sq);
}

/// @brief manhatten distance between two squares
/// @param sq1
/// @param sq2
/// @return
inline uint8_t manhatten_distance(Square sq1, Square sq2)
{
    return std::abs(square_file(sq1) - square_file(sq2)) + std::abs(square_rank(sq1) - square_rank(sq2));
}

/// @brief color of a square, has nothing to do with whose piece is on that square
/// @param square
/// @return
inline Color get_square_color(Square square)
{
    if ((square % 8) % 2 == (square / 8) % 2)
    {
        return Black;
    }
    else
    {
        return White;
    }
}

/// @brief get the piecetype of a piece
/// @param piece
/// @return the piecetype
inline PieceType type_of_piece(Piece piece)
{
    if (piece == None)
        return NONETYPE;
    return PieceToPieceType[piece];
}

/// @brief uci representation of a move
/// @param move
/// @return
inline std::string uciRep(Move move)
{
    std::string m = "";
    m += squareToString[from(move)];
    m += squareToString[to(move)];
    if (promoted(move))
        m += PieceTypeToPromPiece[piece(move)];
    return m;
}

/// @brief makes a piece out of a PieceType and Color
/// @param type
/// @param c
/// @return
inline Piece makePiece(PieceType type, Color c)
{
    if (type == NONETYPE)
        return None;
    return Piece(type + 6 * c);
}

/// @brief prints any bitboard
/// @param bb
inline void printBitboard(U64 bb)
{
    std::bitset<64> b(bb);
    std::string str_bitset = b.to_string();
    for (int i = 0; i < MAX_SQ; i += 8)
    {
        std::string x = str_bitset.substr(i, 8);
        reverse(x.begin(), x.end());
        std::cout << x << std::endl;
    }
    std::cout << '\n' << std::endl;
}

inline uint64_t PawnAttacks(Square sq, Color c)
{
    return PAWN_ATTACKS_TABLE[c][sq];
}

inline uint64_t KnightAttacks(Square sq)
{
    return KNIGHT_ATTACKS_TABLE[sq];
}

inline uint64_t BishopAttacks(Square sq, uint64_t occupied)
{
    return GetBishopAttacks(sq, occupied);
}

inline uint64_t RookAttacks(Square sq, uint64_t occupied)
{
    return GetRookAttacks(sq, occupied);
}

inline uint64_t QueenAttacks(Square sq, uint64_t occupied)
{
    return GetQueenAttacks(sq, occupied);
}

inline uint64_t KingAttacks(Square sq)
{
    return KING_ATTACKS_TABLE[sq];
}

class Board
{
  public:
    Color sideToMove;

    // NO_SQ when enpassant is not possible
    Square enPassantSquare;

    // castling right is encoded in 8bit
    // wk = 1
    // wq = 2
    // bk = 4
    // bq = 8
    // 0  0  0  0  0  0  0  0
    // wk wq bk bq
    uint8_t castlingRights;

    // halfmoves start at 0
    uint8_t halfMoveClock;

    // full moves start at 1
    uint16_t fullMoveNumber;

    // keeps track of previous hashes, used for
    // repetition detection
    std::vector<U64> hashHistory;

    // keeps track on how many checks there currently are
    // 2 = only king moves are valid
    // 1 = king move, block/capture
    uint8_t doubleCheck;

    // the path between horizontal/vertical pinners and
    // the pinned is set
    U64 pinHV;

    // the path between diagonal pinners and
    // the pinned is set
    U64 pinD;

    // all bits set if we are not in check
    // otherwise the path between the king and the checker
    // in case of knights giving check only the knight square
    // is checked
    U64 checkMask = DEFAULT_CHECKMASK;

    // all squares that are seen by an enemy piece
    U64 seen;

    // Occupation Bitboards
    U64 occEnemy;
    U64 occUs;
    U64 occAll;
    U64 enemyEmptyBB;

    // current hashkey
    U64 hashKey;

    U64 SQUARES_BETWEEN_BB[MAX_SQ][MAX_SQ];

    std::vector<State> prevStates;

    U64 Bitboards[12] = {};
    Piece board[MAX_SQ];

    /// @brief initialise the board class
    /// @param fen
    Board(std::string fen = DEFAULT_POS);

    /// @brief Finds what piece is on the square using the bitboards
    /// @param sq
    /// @return
    Piece pieceAtBB(Square sq);

    /// @brief Finds what piece is on the square using the board (more performant)
    /// @param sq
    /// @return
    Piece pieceAtB(Square sq);

    /// @brief  applys a new Fen to the board
    /// @param fen
    /// @param updateAcc
    void applyFen(std::string fen, bool updateAcc = true);

    /// @brief returns the current fen
    /// @return
    std::string getFen();

    /// @brief prints the current board
    void print();

    /// @brief detects if the position is a repetition by default 2, fide would be 3
    /// @param draw
    /// @return
    bool isRepetition(int draw = 1);

    /// @brief only pawns + king = true else false
    /// @param c
    /// @return
    bool nonPawnMat(Color c);

    /// @brief returns the King Square of the specified color
    /// @param c
    /// @return
    Square KingSQ(Color c);

    // returns the King Square of the specified color
    template <Color c> Square KingSQ();

    /// @brief returns all pieces of the other color
    /// @param c
    /// @return
    U64 Enemy(Color c);

    /// @brief returns all pieces of the other color
    /// @tparam c
    /// @return
    template <Color c> U64 Enemy();

    /// @brief returns a bitboard of our pieces
    /// @param c
    /// @return
    U64 Us(Color c);

    /// @brief returns a bitboard of our pieces
    /// @tparam c
    /// @return
    template <Color c> U64 Us();

    /// @brief returns all empty squares or squares with an enemy on them
    /// @param c
    /// @return
    U64 EnemyEmpty(Color c);

    /// @brief returns all pieces color
    /// @return
    U64 All();

    U64 Empty();

    // Gets the piece of the specified color

    /// @brief get pawns
    /// @param c
    /// @return
    U64 Pawns(Color c);

    /// @brief get knights
    /// @param c
    /// @return
    U64 Knights(Color c);

    /// @brief get bishops
    /// @param c
    /// @return
    U64 Bishops(Color c);

    /// @brief get rooks
    /// @param c
    /// @return
    U64 Rooks(Color c);

    /// @brief get queens
    /// @param c
    /// @return
    U64 Queens(Color c);

    /// @brief get kings
    /// @param c
    /// @return
    U64 Kings(Color c);
    template <Color c> U64 Pawns();
    template <Color c> U64 Knights();
    template <Color c> U64 Bishops();
    template <Color c> U64 Rooks();
    template <Color c> U64 Queens();
    template <Color c> U64 Kings();

    /// @brief Is square attacked by color c
    /// @param c
    /// @param sq
    /// @return
    bool isSquareAttacked(Color c, Square sq);

    /// @brief plays the move on the internal board
    /// @param move
    void makeMove(Move move);

    /// @brief unmake a move played on the internal board
    /// @param move
    void unmakeMove(Move move);

    /// @brief make a null move
    void makeNullMove();

    /// @brief unmake a null move
    void unmakeNullMove();

    // update the internal board representation

    /// @brief Remove a Piece from the board
    /// @param piece
    /// @param sq
    void removePiece(Piece piece, Square sq);

    /// @brief Place a Piece on the board    /// @param piece
    /// @param sq
    void placePiece(Piece piece, Square sq);

  private:
    /// @brief calculate the current zobrist hash from scratch
    /// @return
    U64 zobristHash();

    /// @brief initialize SQUARES_BETWEEN_BB array
    void initializeLookupTables();

    // update the hash

    U64 updateKeyPiece(Piece piece, Square sq);
    U64 updateKeyCastling();
    U64 updateKeyEnPassant(Square sq);
    U64 updateKeySideToMove();

    void removeCastlingRightsAll(Color c);
    void removeCastlingRightsRook(Color c, Square sq);
};

inline Board::Board(std::string fen)
{
    initializeLookupTables();
    prevStates.reserve(MAX_PLY);
    hashHistory.reserve(512);

    applyFen(fen, true);

    sideToMove = White;
    enPassantSquare = NO_SQ;
    castlingRights = wk | wq | bk | bq;
    halfMoveClock = 0;
    fullMoveNumber = 1;

    pinHV = 0;
    pinD = 0;
    doubleCheck = 0;
    checkMask = DEFAULT_CHECKMASK;
    seen = 0;

    occEnemy = Enemy<White>();
    occUs = Us<White>();
    occAll = All();
    enemyEmptyBB = EnemyEmpty(White);

    hashKey = zobristHash();

    std::fill(std::begin(board), std::end(board), None);
}

inline Piece Board::pieceAtBB(Square sq)
{
    if (Bitboards[WhitePawn] & (1ULL << sq))
        return WhitePawn;
    if (Bitboards[WhiteKnight] & (1ULL << sq))
        return WhiteKnight;
    if (Bitboards[WhiteBishop] & (1ULL << sq))
        return WhiteBishop;
    if (Bitboards[WhiteRook] & (1ULL << sq))
        return WhiteRook;
    if (Bitboards[WhiteQueen] & (1ULL << sq))
        return WhiteQueen;
    if (Bitboards[WhiteKing] & (1ULL << sq))
        return WhiteKing;
    if (Bitboards[BlackPawn] & (1ULL << sq))
        return BlackPawn;
    if (Bitboards[BlackKnight] & (1ULL << sq))
        return BlackKnight;
    if (Bitboards[BlackBishop] & (1ULL << sq))
        return BlackBishop;
    if (Bitboards[BlackRook] & (1ULL << sq))
        return BlackRook;
    if (Bitboards[BlackQueen] & (1ULL << sq))
        return BlackQueen;
    if (Bitboards[BlackKing] & (1ULL << sq))
        return BlackKing;
    return None;
}

inline Piece Board::pieceAtB(Square sq)
{
    return board[sq];
}

inline void Board::applyFen(std::string fen, bool updateAcc)
{
    for (int i = 0; i < 12; i++)
    {
        Bitboards[i] = 0ULL;
    }
    std::vector<std::string> params = splitInput(fen);
    std::string position = params[0];
    std::string move_right = params[1];
    std::string castling = params[2];
    std::string en_passant = params[3];
    // default
    std::string half_move_clock = "0";
    std::string full_move_counter = "1";
    if (params.size() > 4)
    {
        half_move_clock = params[4];
        full_move_counter = params[5];
    }
    sideToMove = (move_right == "w") ? White : Black;

    Square square = Square(56);
    for (int index = 0; index < static_cast<int>(position.size()); index++)
    {
        char curr = position[index];
        if (charToPiece.find(curr) != charToPiece.end())
        {
            Piece piece = charToPiece[curr];
            if (updateAcc)
                placePiece(piece, square);
            else
                placePiece(piece, square);
            square = Square(square + 1);
        }
        else if (curr == '/')
            square = Square(square - 16);
        else if (isdigit(curr))
        {
            for (int i = 0; i < static_cast<int>(curr - '0'); i++)
            {
                board[square + i] = None;
            }
            square = Square(square + (curr - '0'));
        }
    }

    castlingRights = 0;
    for (size_t i = 0; i < castling.size(); i++)
    {
        if (castling[i] == 'K')
        {
            castlingRights |= wk;
        }
        if (castling[i] == 'Q')
        {
            castlingRights |= wq;
        }
        if (castling[i] == 'k')
        {
            castlingRights |= bk;
        }
        if (castling[i] == 'q')
        {
            castlingRights |= bq;
        }
    }
    if (en_passant == "-")
    {
        enPassantSquare = NO_SQ;
    }
    else
    {
        char letter = en_passant[0];
        int file = letter - 96;
        int rank = en_passant[1] - 48;
        enPassantSquare = Square((rank - 1) * 8 + file - 1);
    }
    // half_move_clock
    halfMoveClock = std::stoi(half_move_clock);

    // full_move_counter actually half moves
    fullMoveNumber = std::stoi(full_move_counter) * 2;

    hashHistory.clear();
    hashHistory.push_back(zobristHash());

    prevStates.clear();
    hashKey = zobristHash();
}

inline std::string Board::getFen()
{
    int sq;
    char letter;
    std::string fen = "";
    for (int rank = 7; rank >= 0; rank--)
    {
        int free_space = 0;
        for (int file = 0; file < 8; file++)
        {
            sq = rank * 8 + file;
            Piece piece = pieceAtB(Square(sq));
            if (piece != None)
            {
                if (free_space)
                {
                    fen += std::to_string(free_space);
                    free_space = 0;
                }
                letter = pieceToChar[piece];
                fen += letter;
            }
            else
            {
                free_space++;
            }
        }
        if (free_space != 0)
        {
            fen += std::to_string(free_space);
        }
        fen += rank > 0 ? "/" : "";
    }
    fen += sideToMove == White ? " w " : " b ";

    if (castlingRights & wk)
        fen += "K";
    if (castlingRights & wq)
        fen += "Q";
    if (castlingRights & bk)
        fen += "k";
    if (castlingRights & bq)
        fen += "q";
    if (castlingRights == 0)
        fen += "-";

    if (enPassantSquare == NO_SQ)
        fen += " - ";
    else
        fen += " " + squareToString[enPassantSquare] + " ";

    fen += std::to_string(halfMoveClock);
    fen += " " + std::to_string(fullMoveNumber / 2);
    return fen;
}

inline void Board::print()
{
    for (int i = 63; i >= 0; i -= 8)
    {
        std::cout << " " << pieceToChar[board[i - 7]] << " " << pieceToChar[board[i - 6]] << " "
                  << pieceToChar[board[i - 5]] << " " << pieceToChar[board[i - 4]] << " " << pieceToChar[board[i - 3]]
                  << " " << pieceToChar[board[i - 2]] << " " << pieceToChar[board[i - 1]] << " "
                  << pieceToChar[board[i]] << " " << std::endl;
    }
    std::cout << '\n' << std::endl;
    std::cout << "Fen: " << getFen() << std::endl;
    std::cout << "Side to move: " << static_cast<int>(sideToMove) << std::endl;
    std::cout << "Castling rights: " << static_cast<int>(castlingRights) << std::endl;
    std::cout << "Halfmoves: " << static_cast<int>(halfMoveClock) << std::endl;
    std::cout << "Fullmoves: " << static_cast<int>(fullMoveNumber) / 2 << std::endl;
    std::cout << "EP: " << static_cast<int>(enPassantSquare) << std::endl;
    std::cout << "Hash: " << hashKey << std::endl;
}

inline bool Board::isRepetition(int draw)
{
    uint8_t c = 0;
    for (int i = static_cast<int>(hashHistory.size()) - 2;
         i >= 0 && i >= static_cast<int>(hashHistory.size()) - halfMoveClock; i -= 2)
    {
        if (hashHistory[i] == hashKey)
            c++;
        if (c == draw)
            return true;
    }
    return false;
}

inline bool Board::nonPawnMat(Color c)
{
    return Knights(c) | Bishops(c) | Rooks(c) | Queens(c);
}

inline Square Board::KingSQ(Color c)
{
    return bsf(Kings(c));
}

inline U64 Board::Enemy(Color c)
{
    return Us(~c);
}

inline U64 Board::Us(Color c)
{
    return Bitboards[PAWN + c * 6] | Bitboards[KNIGHT + c * 6] | Bitboards[BISHOP + c * 6] | Bitboards[ROOK + c * 6] |
           Bitboards[QUEEN + c * 6] | Bitboards[KING + c * 6];
}

inline U64 Board::EnemyEmpty(Color c)
{
    return ~Us(c);
}
inline U64 Board::All()
{
    return Us<White>() | Us<Black>();
}
inline U64 Board::Empty()
{
    return ~All();
}
inline U64 Board::Pawns(Color c)
{
    return Bitboards[PAWN + c * 6];
}
inline U64 Board::Knights(Color c)
{
    return Bitboards[KNIGHT + c * 6];
}
inline U64 Board::Bishops(Color c)
{
    return Bitboards[BISHOP + c * 6];
}
inline U64 Board::Rooks(Color c)
{
    return Bitboards[ROOK + c * 6];
}
inline U64 Board::Queens(Color c)
{
    return Bitboards[QUEEN + c * 6];
}
inline U64 Board::Kings(Color c)
{
    return Bitboards[KING + c * 6];
}

inline bool Board::isSquareAttacked(Color c, Square sq)
{
    if (Pawns(c) & PawnAttacks(sq, ~c))
        return true;
    if (Knights(c) & KnightAttacks(sq))
        return true;
    if ((Bishops(c) | Queens(c)) & BishopAttacks(sq, All()))
        return true;
    if ((Rooks(c) | Queens(c)) & RookAttacks(sq, All()))
        return true;
    if (Kings(c) & KingAttacks(sq))
        return true;
    return false;
}

inline void Board::makeMove(Move move)
{
    PieceType pt = piece(move);
    Piece p = makePiece(pt, sideToMove);
    Square from_sq = from(move);
    Square to_sq = to(move);
    Piece capture = board[to_sq];

    // *****************************
    // STORE STATE HISTORY
    // *****************************

    hashHistory.emplace_back(hashKey);

    State store = State(enPassantSquare, castlingRights, halfMoveClock, capture);
    prevStates.push_back(store);

    halfMoveClock++;
    fullMoveNumber++;

    bool ep = to_sq == enPassantSquare;

    // *****************************
    // UPDATE HASH
    // *****************************

    if (enPassantSquare != NO_SQ)
        hashKey ^= updateKeyEnPassant(enPassantSquare);
    enPassantSquare = NO_SQ;

    hashKey ^= updateKeyCastling();
    if (p == WhiteKing && from_sq == SQ_E1 && to_sq == SQ_G1)
    {
        hashKey ^= updateKeyPiece(WhiteRook, SQ_H1);
        hashKey ^= updateKeyPiece(WhiteRook, SQ_F1);
    }
    else if (p == WhiteKing && from_sq == SQ_E1 && to_sq == SQ_C1)
    {
        hashKey ^= updateKeyPiece(WhiteRook, SQ_A1);
        hashKey ^= updateKeyPiece(WhiteRook, SQ_D1);
    }
    else if (p == BlackKing && from_sq == SQ_E8 && to_sq == SQ_G8)
    {
        hashKey ^= updateKeyPiece(BlackRook, SQ_H8);
        hashKey ^= updateKeyPiece(BlackRook, SQ_F8);
    }
    else if (p == BlackKing && from_sq == SQ_E8 && to_sq == SQ_C8)
    {
        hashKey ^= updateKeyPiece(BlackRook, SQ_A8);
        hashKey ^= updateKeyPiece(BlackRook, SQ_D8);
    }

    if (pt == KING)
    {
        removeCastlingRightsAll(sideToMove);
    }
    else if (pt == ROOK)
    {
        removeCastlingRightsRook(sideToMove, from_sq);
    }
    else if (pt == PAWN)
    {
        halfMoveClock = 0;
        if (ep)
        {
            hashKey ^= updateKeyPiece(makePiece(PAWN, ~sideToMove), Square(to_sq - (sideToMove * -2 + 1) * 8));
        }
        else if (std::abs(from_sq - to_sq) == 16)
        {
            U64 epMask = PawnAttacks(Square(to_sq - (sideToMove * -2 + 1) * 8), sideToMove);
            if (epMask & Pawns(~sideToMove))
            {
                enPassantSquare = Square(to_sq - (sideToMove * -2 + 1) * 8);
                hashKey ^= updateKeyEnPassant(enPassantSquare);
            }
        }
    }

    if (capture != None)
    {
        halfMoveClock = 0;
        hashKey ^= updateKeyPiece(capture, to_sq);
        if (type_of_piece(capture) == ROOK && castlingMapRook.find(to_sq) != castlingMapRook.end())
        {
            castlingRights &= ~castlingMapRook[to_sq];
        }
    }

    if (promoted(move))
    {
        halfMoveClock = 0;

        hashKey ^= updateKeyPiece(makePiece(PAWN, sideToMove), from_sq);
        hashKey ^= updateKeyPiece(p, to_sq);
    }
    else
    {
        hashKey ^= updateKeyPiece(p, from_sq);
        hashKey ^= updateKeyPiece(p, to_sq);
    }

    hashKey ^= updateKeySideToMove();
    hashKey ^= updateKeyCastling();

    // *****************************
    // UPDATE PIECES AND NNUE
    // *****************************

    if (pt == KING)
    {
        if (sideToMove == White && from_sq == SQ_E1 && to_sq == SQ_G1)
        {
            removePiece(WhiteRook, SQ_H1);
            placePiece(WhiteRook, SQ_F1);
        }
        else if (sideToMove == White && from_sq == SQ_E1 && to_sq == SQ_C1)
        {
            removePiece(WhiteRook, SQ_A1);
            placePiece(WhiteRook, SQ_D1);
        }
        else if (sideToMove == Black && from_sq == SQ_E8 && to_sq == SQ_G8)
        {
            removePiece(BlackRook, SQ_H8);
            placePiece(BlackRook, SQ_F8);
        }
        else if (sideToMove == Black && from_sq == SQ_E8 && to_sq == SQ_C8)
        {
            removePiece(BlackRook, SQ_A8);
            placePiece(BlackRook, SQ_D8);
        }
    }
    else if (pt == PAWN && ep)
    {
        removePiece(makePiece(PAWN, ~sideToMove), Square(to_sq - (sideToMove * -2 + 1) * 8));
    }

    if (capture != None)
    {
        removePiece(capture, to_sq);
    }

    if (promoted(move))
    {
        removePiece(makePiece(PAWN, sideToMove), from_sq);
        placePiece(p, to_sq);
    }
    else
    {
        removePiece(p, from_sq);
        placePiece(p, to_sq);
    }

    sideToMove = ~sideToMove;
}

inline void Board::unmakeMove(Move move)
{
    State restore = prevStates.back();
    prevStates.pop_back();

    hashKey = hashHistory.back();
    hashHistory.pop_back();

    enPassantSquare = restore.enPassant;
    castlingRights = restore.castling;
    halfMoveClock = restore.halfMove;
    Piece capture = restore.capturedPiece;
    fullMoveNumber--;

    Square from_sq = from(move);
    Square to_sq = to(move);
    bool promotion = promoted(move);

    sideToMove = ~sideToMove;
    PieceType pt = piece(move);
    Piece p = makePiece(pt, sideToMove);

    if (promotion)
    {
        removePiece(p, to_sq);
        placePiece(makePiece(PAWN, sideToMove), from_sq);
        if (capture != None)
        {
            placePiece(capture, to_sq);
        }
        return;
    }
    else
    {
        removePiece(p, to_sq);
        placePiece(p, from_sq);
    }

    if (to_sq == enPassantSquare && pt == PAWN)
    {
        int8_t offset = sideToMove == White ? -8 : 8;
        placePiece(makePiece(PAWN, ~sideToMove), Square(enPassantSquare + offset));
    }
    else if (capture != None)
    {
        placePiece(capture, to_sq);
    }
    else if (pt == KING)
    {
        if (from_sq == SQ_E1 && to_sq == SQ_G1 && castlingRights & wk)
        {
            removePiece(WhiteRook, SQ_F1);
            placePiece(WhiteRook, SQ_H1);
        }
        else if (from_sq == SQ_E1 && to_sq == SQ_C1 && castlingRights & wq)
        {
            removePiece(WhiteRook, SQ_D1);
            placePiece(WhiteRook, SQ_A1);
        }

        else if (from_sq == SQ_E8 && to_sq == SQ_G8 && castlingRights & bk)
        {
            removePiece(BlackRook, SQ_F8);
            placePiece(BlackRook, SQ_H8);
        }
        else if (from_sq == SQ_E8 && to_sq == SQ_C8 && castlingRights & bq)
        {
            removePiece(BlackRook, SQ_D8);
            placePiece(BlackRook, SQ_A8);
        }
    }
}

inline void Board::makeNullMove()
{
    State store = State(enPassantSquare, castlingRights, halfMoveClock, None);
    prevStates.push_back(store);
    sideToMove = ~sideToMove;

    hashKey ^= updateKeySideToMove();
    if (enPassantSquare != NO_SQ)
        hashKey ^= updateKeyEnPassant(enPassantSquare);

    enPassantSquare = NO_SQ;
    fullMoveNumber++;
}

inline void Board::unmakeNullMove()
{
    State restore = prevStates.back();
    prevStates.pop_back();

    enPassantSquare = restore.enPassant;
    castlingRights = restore.castling;
    halfMoveClock = restore.halfMove;

    hashKey ^= updateKeySideToMove();
    if (enPassantSquare != NO_SQ)
        hashKey ^= updateKeyEnPassant(enPassantSquare);

    fullMoveNumber--;
    sideToMove = ~sideToMove;
}

// ***************************
// PRIVATE FUNCTION DECLARTION
// ***************************

inline U64 Board::zobristHash()
{
    U64 hash = 0ULL;
    U64 wPieces = Us<White>();
    U64 bPieces = Us<Black>();
    // Piece hashes
    while (wPieces)
    {
        Square sq = poplsb(wPieces);
        hash ^= updateKeyPiece(pieceAtB(sq), sq);
    }
    while (bPieces)
    {
        Square sq = poplsb(bPieces);
        hash ^= updateKeyPiece(pieceAtB(sq), sq);
    }
    // Ep hash
    U64 ep_hash = 0ULL;
    if (enPassantSquare != NO_SQ)
    {
        ep_hash = updateKeyEnPassant(enPassantSquare);
    }
    // Turn hash
    U64 turn_hash = sideToMove == White ? RANDOM_ARRAY[780] : 0;
    // Castle hash
    U64 cast_hash = updateKeyCastling();

    return hash ^ cast_hash ^ turn_hash ^ ep_hash;
}

inline void Board::initializeLookupTables()
{
    // initialize squares between table
    U64 sqs;
    for (int sq1 = 0; sq1 <= 63; ++sq1)
    {
        for (int sq2 = 0; sq2 <= 63; ++sq2)
        {
            sqs = (1ULL << sq1) | (1ULL << sq2);
            if (square_file(Square(sq1)) == square_file(Square(sq2)) ||
                square_rank(Square(sq1)) == square_rank(Square(sq2)))
                SQUARES_BETWEEN_BB[sq1][sq2] = RookAttacks(Square(sq1), sqs) & RookAttacks(Square(sq2), sqs);

            else if (diagonal_of(Square(sq1)) == diagonal_of(Square(sq2)) ||
                     anti_diagonal_of(Square(sq1)) == anti_diagonal_of(Square(sq2)))
                SQUARES_BETWEEN_BB[Square(sq1)][sq2] =
                    BishopAttacks(Square(sq1), sqs) & BishopAttacks(Square(sq2), sqs);
        }
    }
}

inline U64 Board::updateKeyPiece(Piece piece, Square sq)
{
    return RANDOM_ARRAY[64 * hash_piece[piece] + sq];
}

inline U64 Board::updateKeyEnPassant(Square sq)
{
    return RANDOM_ARRAY[772 + square_file(sq)];
}

inline U64 Board::updateKeyCastling()
{
    return castlingKey[castlingRights];
}

inline U64 Board::updateKeySideToMove()
{
    return RANDOM_ARRAY[780];
}

inline void Board::removeCastlingRightsAll(Color c)
{
    if (c == White)
    {
        castlingRights &= ~(wk | wq);
    }
    else if (c == Black)
    {
        castlingRights &= ~(bk | bq);
    }
}

inline void Board::removeCastlingRightsRook(Color c, Square sq)
{
    if (c == White && sq == SQ_A1)
    {
        castlingRights &= ~wq;
    }
    else if (c == White && sq == SQ_H1)
    {
        castlingRights &= ~wk;
    }
    else if (c == Black && sq == SQ_A8)
    {
        castlingRights &= ~bq;
    }
    else if (c == Black && sq == SQ_H8)
    {
        castlingRights &= ~bk;
    }
}

inline void Board::removePiece(Piece piece, Square sq)
{
    Bitboards[piece] &= ~(1ULL << sq);
    board[sq] = None;
}

inline void Board::placePiece(Piece piece, Square sq)
{
    Bitboards[piece] |= (1ULL << sq);
    board[sq] = piece;
}

template <Color c> U64 Board::Pawns()
{
    return Bitboards[c == White ? WhitePawn : BlackPawn];
}

template <Color c> U64 Board::Knights()
{
    return Bitboards[c == White ? WhiteKnight : BlackKnight];
}

template <Color c> U64 Board::Bishops()
{
    return Bitboards[c == White ? WhiteBishop : BlackBishop];
}

template <Color c> U64 Board::Rooks()
{
    return Bitboards[c == White ? WhiteRook : BlackRook];
}

template <Color c> U64 Board::Queens()
{
    return Bitboards[c == White ? WhiteQueen : BlackQueen];
}

template <Color c> U64 Board::Kings()
{
    return Bitboards[c == White ? WhiteKing : BlackKing];
}

template <Color c> U64 Board::Us()
{
    return Pawns<c>() | Knights<c>() | Bishops<c>() | Rooks<c>() | Queens<c>() | Kings<c>();
}

template <Color c> U64 Board::Enemy()
{
    return Us<~c>();
}

template <Color c> Square Board::KingSQ()
{
    return bsf(Kings<c>());
}

namespace Movegen
{

// creates the checkmask
template <Color c> U64 DoCheckmask(Board &board, Square sq)
{
    U64 Occ = board.occAll;
    U64 checks = 0ULL;
    U64 pawn_mask = board.Pawns<~c>() & PawnAttacks(sq, c);
    U64 knight_mask = board.Knights<~c>() & KnightAttacks(sq);
    U64 bishop_mask = (board.Bishops<~c>() | board.Queens<~c>()) & BishopAttacks(sq, Occ);
    U64 rook_mask = (board.Rooks<~c>() | board.Queens<~c>()) & RookAttacks(sq, Occ);
    board.doubleCheck = 0;
    if (pawn_mask)
    {
        checks |= pawn_mask;
        board.doubleCheck++;
    }
    if (knight_mask)
    {
        checks |= knight_mask;
        board.doubleCheck++;
    }
    if (bishop_mask)
    {
        if (popcount(bishop_mask) > 1)
            board.doubleCheck++;

        int8_t index = bsf(bishop_mask);
        checks |= board.SQUARES_BETWEEN_BB[sq][index] | (1ULL << index);
        board.doubleCheck++;
    }
    if (rook_mask)
    {
        if (popcount(rook_mask) > 1)
            board.doubleCheck++;

        int8_t index = bsf(rook_mask);
        checks |= board.SQUARES_BETWEEN_BB[sq][index] | (1ULL << index);
        board.doubleCheck++;
    }
    return checks;
}

// creates the pinmask
template <Color c> void DoPinMask(Board &board, Square sq)
{
    U64 them = board.occEnemy;
    U64 rook_mask = (board.Rooks<~c>() | board.Queens<~c>()) & RookAttacks(sq, them);
    U64 bishop_mask = (board.Bishops<~c>() | board.Queens<~c>()) & BishopAttacks(sq, them);

    board.pinD = 0ULL;
    board.pinHV = 0ULL;
    while (rook_mask)
    {
        Square index = poplsb(rook_mask);
        U64 possible_pin = (board.SQUARES_BETWEEN_BB[sq][index] | (1ULL << index));
        if (popcount(possible_pin & board.occUs) == 1)
            board.pinHV |= possible_pin;
    }
    while (bishop_mask)
    {
        Square index = poplsb(bishop_mask);
        U64 possible_pin = (board.SQUARES_BETWEEN_BB[sq][index] | (1ULL << index));
        if (popcount(possible_pin & board.occUs) == 1)
            board.pinD |= possible_pin;
    }
}

// seen squares
template <Color c> void seenSquares(Board &board)
{
    U64 pawns = board.Pawns<c>();
    U64 knights = board.Knights<c>();
    U64 bishops = board.Bishops<c>();
    U64 rooks = board.Rooks<c>();
    U64 queens = board.Queens<c>();
    U64 kings = board.Kings<c>();

    board.seen = 0ULL;
    Square kSq = board.KingSQ<~c>();
    board.occAll &= ~(1ULL << kSq);

    if (c == White)
    {
        board.seen |= pawns << 9 & ~MASK_FILE[0];
        board.seen |= pawns << 7 & ~MASK_FILE[7];
    }
    else
    {
        board.seen |= pawns >> 7 & ~MASK_FILE[0];
        board.seen |= pawns >> 9 & ~MASK_FILE[7];
    }
    while (knights)
    {
        Square index = poplsb(knights);
        board.seen |= KnightAttacks(index);
    }
    while (bishops)
    {
        Square index = poplsb(bishops);
        board.seen |= BishopAttacks(index, board.occAll);
    }
    while (rooks)
    {
        Square index = poplsb(rooks);
        board.seen |= RookAttacks(index, board.occAll);
    }
    while (queens)
    {
        Square index = poplsb(queens);
        board.seen |= QueenAttacks(index, board.occAll);
    }
    while (kings)
    {
        Square index = poplsb(kings);
        board.seen |= KingAttacks(index);
    }

    board.occAll |= (1ULL << kSq);
}

// creates the pinmask and checkmask
template <Color c> void init(Board &board, Square sq)
{
    board.occUs = board.Us<c>();
    board.occEnemy = board.Us<~c>();
    board.occAll = board.occUs | board.occEnemy;
    board.enemyEmptyBB = ~board.occUs;
    seenSquares<~c>(board);
    U64 newMask = DoCheckmask<c>(board, sq);
    board.checkMask = newMask ? newMask : DEFAULT_CHECKMASK;
    DoPinMask<c>(board, sq);
}

// returns a pawn push (only 1 square)
template <Color c> U64 PawnPushSingle(U64 occAll, Square sq)
{
    if (c == White)
    {
        return ((1ULL << sq) << 8) & ~occAll;
    }
    else
    {
        return ((1ULL << sq) >> 8) & ~occAll;
    }
}

template <Color c> U64 PawnPushBoth(U64 occAll, Square sq)
{
    U64 push = (1ULL << sq);
    if (c == White)
    {
        push = (push << 8) & ~occAll;
        return square_rank(sq) == 1 ? push | ((push << 8) & ~occAll) : push;
    }
    else
    {
        push = (push >> 8) & ~occAll;
        return square_rank(sq) == 6 ? push | ((push >> 8) & ~occAll) : push;
    }
}

template <Color c> U64 LegalPawnMovesSingle(const Board &board, Square sq)
{
    // If we are pinned diagonally we can only do captures which are on the pin_dg and on the board.checkMask
    if (board.pinD & (1ULL << sq))
        return PawnAttacks(sq, c) & board.pinD & board.checkMask & board.occEnemy;

    // If we are pinned horizontally we can do no moves but if we are pinned vertically we can only do pawn pushs
    if (board.pinHV & (1ULL << sq))
        return PawnPushBoth<c>(board.occAll, sq) & board.pinHV & board.checkMask;
    return ((PawnAttacks(sq, c) & board.occEnemy) | PawnPushBoth<c>(board.occAll, sq)) & board.checkMask;
}

template <Color c> U64 LegalPawnMovesEPSingle(Board &board, Square sq, Square ep)
{
    // If we are pinned diagonally we can only do captures which are on the pin_dg and on the board.checkMask
    if (board.pinD & (1ULL << sq))
        return PawnAttacks(sq, c) & board.pinD & board.checkMask & (board.occEnemy | (1ULL << ep));

    // If we are pinned horizontally we can do no moves but if we are pinned vertically we can only do pawn pushs
    if (board.pinHV & (1ULL << sq))
        return PawnPushBoth<c>(board.occAll, sq) & board.pinHV & board.checkMask;
    U64 attacks = PawnAttacks(sq, c);
    if (board.checkMask != DEFAULT_CHECKMASK)
    {
        // If we are in check and the en passant square lies on our attackmask and the en passant piece gives check
        // return the ep mask as a move square
        if (attacks & (1ULL << ep) && board.checkMask & (1ULL << (ep - (c * -2 + 1) * 8)))
            return 1ULL << ep;
        // If we are in check we can do all moves that are on the board.checkMask
        return ((attacks & board.occEnemy) | PawnPushBoth<c>(board.occAll, sq)) & board.checkMask;
    }

    U64 moves = ((attacks & board.occEnemy) | PawnPushBoth<c>(board.occAll, sq)) & board.checkMask;
    // We need to make extra sure that ep moves dont leave the king in check
    // 7k/8/8/K1Pp3r/8/8/8/8 w - d6 0 1
    // Horizontal rook pins our pawn through another pawn, our pawn can push but not take enpassant
    // remove both the pawn that made the push and our pawn that could take in theory and check if that would give check
    if ((1ULL << ep) & attacks)
    {
        Square tP = c == White ? Square(static_cast<int>(ep) - 8) : Square(static_cast<int>(ep) + 8);
        Square kSQ = board.KingSQ<c>();
        U64 enemyQueenRook = board.Rooks(~c) | board.Queens(~c);
        if ((enemyQueenRook)&MASK_RANK[square_rank(kSQ)])
        {
            Piece ourPawn = makePiece(PAWN, c);
            Piece theirPawn = makePiece(PAWN, ~c);
            board.removePiece(ourPawn, sq);
            board.removePiece(theirPawn, tP);
            board.placePiece(ourPawn, ep);
            if (!((RookAttacks(kSQ, board.All()) & (enemyQueenRook))))
                moves |= (1ULL << ep);
            board.placePiece(ourPawn, sq);
            board.placePiece(theirPawn, tP);
            board.removePiece(ourPawn, ep);
        }
        else
        {
            moves |= (1ULL << ep);
        }
    }
    return moves;
}

template <Direction direction> constexpr U64 shift(U64 b)
{
    switch (direction)
    {
    case NORTH:
        return b << 8;
    case SOUTH:
        return b >> 8;
    case NORTH_WEST:
        return (b & ~MASK_FILE[0]) << 7;
    case WEST:
        return (b & ~MASK_FILE[0]) >> 1;
    case SOUTH_WEST:
        return (b & ~MASK_FILE[0]) >> 9;
    case NORTH_EAST:
        return (b & ~MASK_FILE[7]) << 9;
    case EAST:
        return (b & ~MASK_FILE[7]) << 1;
    case SOUTH_EAST:
        return (b & ~MASK_FILE[7]) >> 7;
    }
}

// all legal moves for each piece

template <Color c, Movetype mt> void LegalPawnMovesAll(Board &board, Movelist &movelist)
{
    U64 pawns_mask = board.Pawns<c>();
    U64 empty = ~board.occAll;
    U64 enemy = board.Enemy<c>();
    U64 moveD = ~board.pinD;
    U64 moveHV = ~board.pinHV;

    constexpr Direction UP = c == White ? NORTH : SOUTH;
    constexpr Direction UP_LEFT = c == White ? NORTH_WEST : SOUTH_EAST;
    constexpr Direction UP_RIGHT = c == White ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction DOWN = c == Black ? NORTH : SOUTH;
    constexpr Direction DOWN_LEFT = c == Black ? NORTH_EAST : SOUTH_WEST;
    constexpr Direction DOWN_RIGHT = c == Black ? NORTH_WEST : SOUTH_EAST;
    constexpr U64 RANK_BEFORE_PROMO = c == White ? MASK_RANK[6] : MASK_RANK[1];
    constexpr U64 doublePushRank = c == White ? MASK_RANK[2] : MASK_RANK[5];
    const bool EP = board.enPassantSquare != NO_SQ;

    U64 singlePush = shift<UP>(pawns_mask & moveHV & moveD & ~RANK_BEFORE_PROMO) & empty;
    U64 doublePush = shift<UP>(singlePush & doublePushRank) & empty;
    U64 captureRight = shift<UP_RIGHT>(pawns_mask & ~RANK_BEFORE_PROMO & moveD & moveHV) & enemy & board.checkMask;
    U64 captureLeft = shift<UP_LEFT>(pawns_mask & ~RANK_BEFORE_PROMO & moveD & moveHV) & enemy & board.checkMask;
    singlePush &= board.checkMask;
    doublePush &= board.checkMask;

    U64 promotionPawns = pawns_mask & RANK_BEFORE_PROMO;

    while (promotionPawns && (mt == ALL || mt == CAPTURE))
    {
        Square from = poplsb(promotionPawns);
        U64 moves = LegalPawnMovesSingle<c>(board, from);
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<QUEEN, true>(from, to));
            movelist.Add(make<ROOK, true>(from, to));
            movelist.Add(make<KNIGHT, true>(from, to));
            movelist.Add(make<BISHOP, true>(from, to));
        }
    }

    if (EP && (mt == ALL || mt == CAPTURE))
    {
        Square ep = board.enPassantSquare;
        U64 left = shift<UP_LEFT>(pawns_mask & ~RANK_BEFORE_PROMO) & (1ULL << ep);
        U64 right = shift<UP_RIGHT>(pawns_mask & ~RANK_BEFORE_PROMO) & (1ULL << ep);

        while (left || right)
        {
            Square to;
            Square from;
            if (left)
            {
                to = poplsb(left);
                from = Square(to + DOWN_RIGHT);
            }
            else
            {
                to = poplsb(right);
                from = Square(to + DOWN_LEFT);
            }

            if ((1ULL << from) & board.pinHV)
                continue;
            if ((1ULL << from) & board.pinD)
            {
                if (!(board.pinD & (1ULL << ep)))
                    continue;
            }

            if (board.checkMask != DEFAULT_CHECKMASK)
            {
                // If we are in check and the en passant square lies on our attackmask and the en passant piece gives
                // check return the ep mask as a move square
                if (board.checkMask & (1ULL << (ep - (c * -2 + 1) * 8)))
                    movelist.Add(make<PAWN, false>(from, to));
                continue;
            }
            Square tP = c == White ? Square(static_cast<int>(ep) - 8) : Square(static_cast<int>(ep) + 8);
            Square kSQ = board.KingSQ<c>();
            U64 enemyQueenRook = board.Rooks(~c) | board.Queens(~c);
            if (enemyQueenRook & MASK_RANK[square_rank(kSQ)])
            {
                Piece ourPawn = makePiece(PAWN, c);
                Piece theirPawn = makePiece(PAWN, ~c);
                board.removePiece(ourPawn, from);
                board.removePiece(theirPawn, tP);
                board.placePiece(ourPawn, ep);
                if (!((RookAttacks(kSQ, board.All()) & (enemyQueenRook))))
                    movelist.Add(make<PAWN, false>(from, to));
                board.placePiece(ourPawn, from);
                board.placePiece(theirPawn, tP);
                board.removePiece(ourPawn, ep);
            }
            else
            {
                movelist.Add(make<PAWN, false>(from, to));
            }
        }
    }

    U64 pinnedPawns = pawns_mask & ~moveD;
    while (pinnedPawns && (mt == ALL || mt == CAPTURE))
    {
        Square from = poplsb(pinnedPawns);
        U64 moves = PawnAttacks(from, c) & board.pinD & board.checkMask & board.occEnemy;
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<PAWN, false>(from, to));
        }
    }

    pinnedPawns = pawns_mask & ~moveHV;
    while (pinnedPawns && (mt == ALL || mt == QUIET))
    {
        Square from = poplsb(pinnedPawns);
        U64 moves = PawnPushBoth<c>(board.occAll, from) & board.pinHV & board.checkMask;
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<PAWN, false>(from, to));
        }
    }

    while (singlePush && (mt == ALL || mt == QUIET))
    {
        Square to = poplsb(singlePush);
        movelist.Add(make<PAWN, false>(Square(to + DOWN), to));
    }

    while (doublePush && (mt == ALL || mt == QUIET))
    {
        Square to = poplsb(doublePush);
        movelist.Add(make<PAWN, false>(Square(to + DOWN + DOWN), to));
    }

    while (captureRight && (mt == ALL || mt == CAPTURE))
    {
        Square to = poplsb(captureRight);
        movelist.Add(make<PAWN, false>(Square(to + DOWN_LEFT), to));
    }

    while (captureLeft && (mt == ALL || mt == CAPTURE))
    {
        Square to = poplsb(captureLeft);
        movelist.Add(make<PAWN, false>(Square(to + DOWN_RIGHT), to));
    }
}

template <Movetype mt> U64 LegalKnightMoves(const Board &board, Square sq)
{
    U64 bb;

    if (mt == ALL)
        bb = board.enemyEmptyBB;
    else if (mt == CAPTURE)
        bb = board.occEnemy;
    else
        bb = ~board.occAll;

    if (board.pinD & (1ULL << sq) || board.pinHV & (1ULL << sq))
        return 0ULL;
    return KnightAttacks(sq) & bb & board.checkMask;
}

template <Movetype mt> U64 LegalBishopMoves(const Board &board, Square sq)
{
    U64 bb;

    if (mt == ALL)
        bb = board.enemyEmptyBB;
    else if (mt == CAPTURE)
        bb = board.occEnemy;
    else
        bb = ~board.occAll;

    if (board.pinHV & (1ULL << sq))
        return 0ULL;
    if (board.pinD & (1ULL << sq))
        return BishopAttacks(sq, board.occAll) & bb & board.pinD & board.checkMask;
    return BishopAttacks(sq, board.occAll) & bb & board.checkMask;
}

template <Movetype mt> U64 LegalRookMoves(const Board &board, Square sq)
{
    U64 bb;

    if (mt == ALL)
        bb = board.enemyEmptyBB;
    else if (mt == CAPTURE)
        bb = board.occEnemy;
    else
        bb = ~board.occAll;

    if (board.pinD & (1ULL << sq))
        return 0ULL;
    if (board.pinHV & (1ULL << sq))
        return RookAttacks(sq, board.occAll) & bb & board.pinHV & board.checkMask;
    return RookAttacks(sq, board.occAll) & bb & board.checkMask;
}

template <Movetype mt> U64 LegalQueenMoves(const Board &board, Square sq)
{
    return LegalRookMoves<mt>(board, sq) | LegalBishopMoves<mt>(board, sq);
}

template <Movetype mt> U64 LegalKingMoves(const Board &board, Square sq)
{
    U64 bb;

    if (mt == ALL)
        bb = board.enemyEmptyBB;
    else if (mt == CAPTURE)
        bb = board.occEnemy;
    else
        bb = ~board.occAll;

    return KingAttacks(sq) & bb & ~board.seen;
}

template <Color c> U64 LegalKingMovesCastling(const Board &board, Square sq)
{
    U64 moves = KingAttacks(sq) & board.enemyEmptyBB & ~board.seen;

    if (c == White)
    {
        if (board.castlingRights & wk && !(WK_CASTLE_MASK & board.occAll) && moves & (1ULL << SQ_F1) &&
            !(board.seen & (1ULL << SQ_G1)))
        {
            moves |= (1ULL << SQ_G1);
        }
        if (board.castlingRights & wq && !(WQ_CASTLE_MASK & board.occAll) && moves & (1ULL << SQ_D1) &&
            !(board.seen & (1ULL << SQ_C1)))
        {
            moves |= (1ULL << SQ_C1);
        }
    }
    else
    {
        if (board.castlingRights & bk && !(BK_CASTLE_MASK & board.occAll) && moves & (1ULL << SQ_F8) &&
            !(board.seen & (1ULL << SQ_G8)))
        {
            moves |= (1ULL << SQ_G8);
        }
        if (board.castlingRights & bq && !(BQ_CASTLE_MASK & board.occAll) && moves & (1ULL << SQ_D8) &&
            !(board.seen & (1ULL << SQ_C8)))
        {
            moves |= (1ULL << SQ_C8);
        }
    }
    return moves;
}

// all legal moves for a position
template <Color c, Movetype mt> void legalmoves(Board &board, Movelist &movelist)
{
    init<c>(board, board.KingSQ<c>());

    Square from = board.KingSQ<c>();
    U64 moves;
    if (!board.castlingRights || board.checkMask != DEFAULT_CHECKMASK || mt == CAPTURE)
        moves = LegalKingMoves<mt>(board, from);
    else
        moves = LegalKingMovesCastling<c>(board, from);

    while (moves)
    {
        Square to = poplsb(moves);
        movelist.Add(make<KING, false>(from, to));
    }

    if (board.doubleCheck == 2)
        return;

    U64 knights_mask = board.Knights<c>();
    U64 bishops_mask = board.Bishops<c>();
    U64 rooks_mask = board.Rooks<c>();
    U64 queens_mask = board.Queens<c>();

    LegalPawnMovesAll<c, mt>(board, movelist);

    while (knights_mask)
    {
        Square from = poplsb(knights_mask);
        U64 moves = LegalKnightMoves<mt>(board, from);
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<KNIGHT, false>(from, to));
        }
    }
    while (bishops_mask)
    {
        Square from = poplsb(bishops_mask);
        U64 moves = LegalBishopMoves<mt>(board, from);
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<BISHOP, false>(from, to));
        }
    }
    while (rooks_mask)
    {
        Square from = poplsb(rooks_mask);
        U64 moves = LegalRookMoves<mt>(board, from);
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<ROOK, false>(from, to));
        }
    }
    while (queens_mask)
    {
        Square from = poplsb(queens_mask);
        U64 moves = LegalQueenMoves<mt>(board, from);
        while (moves)
        {
            Square to = poplsb(moves);
            movelist.Add(make<QUEEN, false>(from, to));
        }
    }
}

/// @brief generate all legalmoves, template parameters are ALL, CAPTURE, QUIET
/// @tparam mt
/// @param board
/// @param movelist
template <Movetype mt> void legalmoves(Board &board, Movelist &movelist)
{
    if (board.sideToMove == White)
        legalmoves<White, mt>(board, movelist);
    else
        legalmoves<Black, mt>(board, movelist);
}

template <Color c> bool hasLegalMoves(Board &board)
{
    init<c>(board, board.KingSQ<c>());

    Square from = board.KingSQ<c>();
    U64 moves = !board.castlingRights || board.checkMask != DEFAULT_CHECKMASK ? LegalKingMoves<ALL>(board, from)
                                                                              : LegalKingMovesCastling<c>(board, from);
    if (moves)
        return true;

    if (board.doubleCheck == 2)
        return false;

    U64 pawns_mask = board.Pawns<c>();
    U64 knights_mask = board.Knights<c>();
    U64 bishops_mask = board.Bishops<c>();
    U64 rooks_mask = board.Rooks<c>();
    U64 queens_mask = board.Queens<c>();

    const bool noEP = board.enPassantSquare == NO_SQ;

    while (pawns_mask)
    {
        Square from = poplsb(pawns_mask);
        U64 moves =
            noEP ? LegalPawnMovesSingle<c>(board, from) : LegalPawnMovesEPSingle<c>(board, from, board.enPassantSquare);
        while (moves)
        {
            return true;
        }
    }
    while (knights_mask)
    {
        Square from = poplsb(knights_mask);
        U64 moves = LegalKnightMoves<ALL>(board, from);
        while (moves)
        {
            return true;
        }
    }
    while (bishops_mask)
    {
        Square from = poplsb(bishops_mask);
        U64 moves = LegalBishopMoves<ALL>(board, from);
        while (moves)
        {
            return true;
        }
    }
    while (rooks_mask)
    {
        Square from = poplsb(rooks_mask);
        U64 moves = LegalRookMoves<ALL>(board, from);
        while (moves)
        {
            return true;
        }
    }
    while (queens_mask)
    {
        Square from = poplsb(queens_mask);
        U64 moves = LegalQueenMoves<ALL>(board, from);
        while (moves)
        {
            return true;
        }
    }

    return false;
}

/// @brief tests if there are any legal moves
/// @tparam c
/// @param board
/// @return
inline bool hasLegalMoves(Board &board)
{
    if (board.sideToMove == White)
        return hasLegalMoves<White>(board);
    else
        return hasLegalMoves<Black>(board);
}
} // namespace Movegen

} // namespace Chess