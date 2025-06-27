#include "stdafx.h"
// PCH ^

const char* about = "Copyright (c) sammoth, 2020\n"
"Copyright (c) paper <paper@tflc.us>, 2025"
"\n"
"This component uses PulseAudio, licensed under the GNU LGPL v2.1, see:\n"
"https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html\n"
"\n"
"Source code is provided at:\n"
"https://github.com/mrpapersonic/foo_out_pulse/releases";

DECLARE_COMPONENT_VERSION("PulseAudio Output", "0.7-beta",
                          about);
VALIDATE_COMPONENT_FILENAME("foo_out_pulse.dll");
