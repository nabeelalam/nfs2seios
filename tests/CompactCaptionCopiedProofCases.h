// SPDX-License-Identifier: MIT
#pragma once
// Genuine lexical seals plus independently copied original343 bytes. No public
// proof factory, guest target rewrite, forged640padding or fabricated hash.
static void compactCaptionCopiedProofCases(){
 prepare();Game g;g.eax=testOwner;g.ebp=testOwner;
 nfs_logo_capture::Target copyOwner{};
 std::vector<uint8_t>source;
 std::shared_ptr<const nfs_compact_caption::Proof>retained;
 std::shared_ptr<const nfs_soft::FramePacket>retainedPacket;
 uint64_t serial=0,lifetime=0;
 {cap::OwnerScope owner(g);auto*state=cap::current(g);assert(state);
  {cap::TailScope tail(g);trackedString(g,"normal");}
  const auto proof=cap::acquireCurrentProof(g);assert(proof);
  const auto packet=proof->packet();copyOwner=selected;serial=state->serial;lifetime=state->lifetime;
  const size_t bytes=16+343*480*2;
  source.assign(memory.begin()+copyOwner.w[8],memory.begin()+copyOwner.w[8]+bytes);
  // Exactly the original copy/upload situation:343 was observed at capture,
  // but the independent ambient selected target is now a genuine full640 one.
  selected=::target(640,0x100000,0x41000);const auto ambient=selected;
  assert(!cap::acquireCurrentProof(g));
  auto acquire=[&](){return cap::acquireCopiedProof(g,proof,packet,serial,lifetime,copyOwner,source.data(),source.size());};
  assert(acquire()==proof);
  auto unchangedSelection=[&](){assert(selected.w==ambient.w&&selected.header==ambient.header&&selected.owner==ambient.owner&&selected.ownerWords==ambient.ownerWords&&selected.ownerState==ambient.ownerState&&selected.x==ambient.x&&selected.y==ambient.y);};
  unchangedSelection();
  // The selected-double-buffer word is deliberately ambient; source owner,
  // header, completeLUTs and its four other captured bindings stay exact.
  copyOwner.ownerState[0]^=0x10000;assert(acquire()==proof);copyOwner.ownerState[0]^=0x10000;
  unsigned negatives=0;
  auto refuse=[&](){assert(!acquire());unchangedSelection();++negatives;};
  copyOwner.owner^=4;refuse();copyOwner.owner^=4;
  for(unsigned k=0;k<14;++k){copyOwner.w[k]^=1;refuse();copyOwner.w[k]^=1;}
  for(unsigned k=0;k<14;++k){copyOwner.ownerWords[k]^=1;refuse();copyOwner.ownerWords[k]^=1;}
  for(unsigned k=0;k<4;++k){copyOwner.header[k]^=1;refuse();copyOwner.header[k]^=1;}
  for(unsigned k:{0u,1u,172u,342u}){copyOwner.x[k]^=2;refuse();copyOwner.x[k]^=2;}
  for(unsigned k:{0u,1u,250u,479u}){copyOwner.y[k]^=2;refuse();copyOwner.y[k]^=2;}
  for(unsigned k=1;k<5;++k){copyOwner.ownerState[k]^=4;refuse();copyOwner.ownerState[k]^=4;}
  for(unsigned k=0;k<16;++k){source[k]^=1;refuse();source[k]^=1;}
  for(size_t k:{size_t(16),size_t(16+2*(369*343+123)),bytes-1}){source[k]^=1;refuse();source[k]^=1;}
  ++serial;refuse();--serial;++lifetime;refuse();--lifetime;
  put<uint32_t>(dword_4D40B8,0x1235);refuse();put<uint32_t>(dword_4D40B8,0x1234);
  put<uint32_t>(byte_4D3ECC,((testSelector+1)%8)<<16);refuse();put<uint32_t>(byte_4D3ECC,testSelector<<16);
  put<uint32_t>(testRecord,(testResource<<16)|3);refuse();put<uint32_t>(testRecord,(testResource<<16)|2);
  put<uint32_t>(off_4D3ED0+testSelector*4,testRecord+4);refuse();put<uint32_t>(off_4D3ED0+testSelector*4,testRecord);
  assert(!cap::acquireCopiedProof(g,{},packet,serial,lifetime,copyOwner,source.data(),source.size()));
  assert(!cap::acquireCopiedProof(g,proof,{},serial,lifetime,copyOwner,source.data(),source.size()));
  assert(!cap::acquireCopiedProof(g,proof,packet,serial,lifetime,copyOwner,nullptr,source.size()));
  assert(!cap::acquireCopiedProof(g,proof,packet,serial,lifetime,copyOwner,source.data(),source.size()-1));
  assert(!cap::acquireCopiedProof(g,proof,packet,serial,lifetime,copyOwner,source.data(),source.size()+1));
  assert(acquire()==proof&&negatives>=60);unchangedSelection();
  const auto before=reads;
  std::thread foreign([&]{foreignPath=true;assert(!cap::acquireCopiedProof(g,proof,packet,serial,lifetime,copyOwner,source.data(),source.size()));foreignPath=false;});foreign.join();
  assert(reads==before&&foreignAllocations==0);
  // A later genuine lexical capture invalidates the older current acquisition
  // even though its immutable historical packet remains fully owned.
  selected=copyOwner;{cap::TailScope tail(g);trackedString(g,"normal");}
  const auto latest=cap::acquireCurrentProof(g);assert(latest&&latest!=proof);
  selected=ambient;assert(!acquire());
  retained=latest;retainedPacket=latest->packet();serial=state->serial;lifetime=state->lifetime;
  assert(cap::acquireCopiedProof(g,retained,retainedPacket,serial,lifetime,copyOwner,source.data(),source.size())==retained);
 }
 const auto before=reads;
 assert(!cap::acquireCopiedProof(g,retained,retainedPacket,serial,lifetime,copyOwner,source.data(),source.size()));
 assert(reads==before&&retained&&retained->packet()==retainedPacket);
}
