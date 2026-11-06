// SPDX-License-Identifier: MIT
#pragma once
#include <memory>
#include <string>
namespace nfs {
// Removes only --record-session <new.jsonl>, preserving all legacy arguments.
// Throws on malformed/duplicate recording options. Does not open files.
std::string desktop_recording_option(int &argc,char **argv);
class DesktopDiagnosticRecorder {
public:
 struct Limits { unsigned samples=3600; unsigned long long frames=1000000,bytes=128ull*1024*1024,queue_bytes=4ull*1024*1024; };
 explicit DesktopDiagnosticRecorder(const std::string &path);
 DesktopDiagnosticRecorder(const std::string &path,Limits limits);
 ~DesktopDiagnosticRecorder();
 DesktopDiagnosticRecorder(const DesktopDiagnosticRecorder&)=delete;
 DesktopDiagnosticRecorder&operator=(const DesktopDiagnosticRecorder&)=delete;
 void poll(); // App main thread, outside presentation callback.
 void stop(const char *reason="session_end") noexcept;
 bool failed() const noexcept;
private:
 struct Impl;std::unique_ptr<Impl> impl;
};
}
