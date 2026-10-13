// SPDX-License-Identifier: MIT
// Explicit unavailable services for the initial single-player iPad port.
// These exports do not implement multiplayer or claim a Windows serial device.
#include <stdint.h>
#include "PlatformError.h"
namespace {thread_local int32_t socket_error=10093;int32_t socket_failure(){socket_error=10093;return -1;}int32_t serial_failure(){nfs::platform_error()=50;return 0;}}
extern "C" int32_t GetCommState_wrap(int32_t,int32_t){return serial_failure();}
extern "C" int32_t SetCommState_wrap(int32_t,int32_t){return serial_failure();}
extern "C" int32_t SetCommTimeouts_wrap(int32_t,int32_t){return serial_failure();}
extern "C" int32_t PurgeComm_wrap(int32_t,int32_t){return serial_failure();}
extern "C" int32_t WSAStartup_wrap(int32_t,int32_t){socket_error=10091;return socket_error;}
extern "C" int32_t WSACleanup_wrap(){return socket_failure();}
extern "C" int32_t WSAGetLastError_wrap(){return socket_error;}
extern "C" int32_t WSAFDIsSet_wrap(int32_t,int32_t){socket_error=10093;return 0;}
extern "C" int32_t socket_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t closesocket_wrap(int32_t){return socket_failure();}
extern "C" int32_t accept_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t bind_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t connect_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t gethostbyname_wrap(int32_t){socket_error=10093;return 0;}
extern "C" int32_t gethostname_wrap(int32_t,int32_t){return socket_failure();}
extern "C" int32_t getsockname_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t htons_wrap(int32_t value){const uint32_t port=uint32_t(value)&65535;return int32_t((port>>8)|((port&255)<<8));}
extern "C" int32_t inet_addr_wrap(int32_t){return socket_failure();}
extern "C" int32_t inet_ntoa_wrap(int32_t){socket_error=10093;return 0;}
extern "C" int32_t ioctlsocket_wrap(int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t listen_wrap(int32_t,int32_t){return socket_failure();}
extern "C" int32_t recv_wrap(int32_t,int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t recvfrom_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t select_wrap(int32_t,int32_t,int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t send_wrap(int32_t,int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t sendto_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t){return socket_failure();}
extern "C" int32_t setsockopt_wrap(int32_t,int32_t,int32_t,int32_t,int32_t){return socket_failure();}
