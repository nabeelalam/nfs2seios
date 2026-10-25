// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "HdArtProvider.h"
#include "NativeMarkProvider.h"
#include "AudioArtProvider.h"
#include "FeedbackModalBackdropArtProvider.h"
#include "AudioDialogRecords.h"
#include <memory>
#include <string>
#include <vector>
namespace nfs_hd {
using Digest=std::array<uint8_t,32>;
// Content pin is supplied by a trusted application caller, never by the file.
// Loading does not install a provider, enable replacements or prove GPU state.
class ArtworkPack final {
public:
    static constexpr size_t LegacyMaxBytes=96u*1024u*1024u;
    static constexpr size_t MaxBytes=128u*1024u*1024u;
    static std::shared_ptr<const ArtworkPack> loadFile(const std::string& path,const Digest& expected,std::string* error=nullptr) noexcept;
    static std::shared_ptr<const ArtworkPack> loadOwned(std::vector<uint8_t> bytes,const Digest& expected,std::string* error=nullptr) noexcept;
    ArtworkPack(const ArtworkPack&)=delete;
    ArtworkPack& operator=(const ArtworkPack&)=delete;
    std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog> catalog() const noexcept{return catalog_;}
    // Retain this pack for the entire provider lifetime before installing it.
    NfsHdArtProvider provider() const noexcept;
    // Optional version2/3 native artwork shares this authenticated pack and owner.
    NfsNativeMarkProvider nativeMarkProvider() const noexcept;
    // Optional version4 Audio atlas and trusted source metadata. Original source
    // pixels remain in the separately supplied game archive.
    NfsAudioArtProvider audioArtProvider() const noexcept;
    const std::array<nfs_audio_frame::SourceIdentity,6>* audioSourceIdentities() const noexcept{return audio_.pixels?&audioSources_:nullptr;}
    // Optional version5 straight-RGBA Feedback pair; the same retained pack
    // owns these bytes. No provider is exposed when both sections are absent.
    NfsFeedbackModalBackdropArtProvider feedbackBackdropProvider() const noexcept;
    const Digest& digest()const noexcept{return digest_;}
    size_t size()const noexcept{return bytes_.size();}
private:
    explicit ArtworkPack(std::vector<uint8_t> bytes):bytes_(std::move(bytes)){}
    bool parse();
    static int texture(void*,uint32_t,NfsBackgroundSceneTexture*);
    static int word(void*,uint32_t,NfsBackgroundSceneWord*);
    static int logo(void*,NfsSmallLogoArt*);
    static int region(void*,uint32_t,NfsSmallLogoRegion*);
    static int nativeMark(void*,uint32_t,uint32_t,const uint8_t[32],NfsNativeMarkArt*);
    static int audioAtlas(void*,NfsAudioArt*);
    static int feedbackBackdrop(void*,NfsFeedbackModalBackdropArtView*);
    NfsFeedbackModalBackdropArtView feedback_{};
    NfsAudioArt audio_{};
    std::array<nfs_audio_frame::SourceIdentity,6> audioSources_{};
    struct NativeMarkBinding {uint32_t texture=0,encoding=0,edge=0;Digest prepared{};};
    std::array<NativeMarkBinding,NFS_NATIVE_MARK_MAX_BINDINGS> nativeBindings_{};
    std::array<NfsNativeMarkArt,NFS_NATIVE_MARK_MAX_TEXTURES> nativeTextures_{};
    uint32_t nativeBindingCount_=0,nativeTextureCount_=0;
    std::vector<uint8_t> bytes_;
    Digest digest_{};
    std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog> catalog_;
    std::array<NfsBackgroundSceneTexture,5> textures_{};
    std::array<NfsBackgroundSceneWord,12> words_{};
    std::array<NfsSmallLogoRegion,5> regions_{};
    NfsSmallLogoArt logo_{};
};
}
