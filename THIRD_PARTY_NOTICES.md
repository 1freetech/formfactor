# Third-Party Notices

FormFactor 1.127 was informed by several open-source engineering projects. The
game keeps the C++ engineering core authoritative and does not copy code from
projects whose license is incompatible, missing, or unclear.

## embeddedalpha/PCB-Design

Source: https://github.com/embeddedalpha/PCB-Design

License: MIT

Copyright (c) 2020 Embedded_Alpha

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

FormFactor uses representative MIT-licensed schematic identity examples in the
KiCad legacy-import regression test, including symbol/value/footprint links.
Those records are test/reference data only; they do not prove a generic game
component has the same physical package.

## LuizZak/power-system-sample

Source: https://github.com/LuizZak/power-system-sample

License: MIT

Copyright 2026 Luiz Fernando Silva

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

FormFactor 1.127 uses an independently written pin graph with the same general
graph/network design idea: explicit nodes, explicit edges, connected networks,
and deterministic traversal.

## umutsevdi/Logic-Circuit-Simulator

Source: https://github.com/umutsevdi/Logic-Circuit-Simulator

License: GNU GPL v3.

No GPL implementation code is copied into FormFactor 1.127. The project was
used as an architecture reference for pin sockets, reusable circuit blocks,
truth-table workflow, save/load behavior, and circuit-editor interaction.

## MB3hel/GodotAUVSim

Source: https://github.com/MB3hel/GodotAUVSim

No top-level repository license was found during the 1.127 review. No source
code from this repository is copied into FormFactor. General ideas such as
strict simulator-object validation, separation of simulation/presentation,
and explicit interfaces were used only as design references.
