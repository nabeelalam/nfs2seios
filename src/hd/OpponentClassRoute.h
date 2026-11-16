// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
namespace nfs_opponent_class {
// Independent admission boundary for the original40CD70/40D026 class preview.
// This does not establish source authentication or install artwork. The caller
// must derive expectedName from an authenticated off409E70 source catalogue;
// the live incoming pointer/table value alone must never populate both fields.
struct Selection {
 uint32_t form=0,formGuest=0,currentFormGuest=0,enabled=0,index=0;
 uintptr_t game=0,thread=0,currentGame=0,currentThread=0;
 uint64_t ownerEpoch=0,currentEpoch=0;
 std::array<uint8_t,4> observedName{},expectedName{};
 bool authenticatedSource=false;
};
inline bool current(const Selection&s)noexcept {
 if(s.form!=0x4d3204u||!s.formGuest||s.currentFormGuest!=s.formGuest||
    !s.game||s.currentGame!=s.game||!s.thread||s.currentThread!=s.thread||
    !s.ownerEpoch||s.currentEpoch!=s.ownerEpoch||!s.enabled||s.index<12||s.index>14||
    !s.authenticatedSource||s.observedName!=s.expectedName)return false;
 const std::array<uint8_t,4> name={{'c','l','s',uint8_t('a'+s.index-12)}};return s.expectedName==name;
}
// A successful selection check is necessary only. Publication additionally needs
// an independently authenticated image binding, actual native upload witness,
// address/object/context generation, full geometry and retained-source proof.
// No default A/B/C mapping or guessed digest is supplied here.
}
