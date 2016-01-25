/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef V8GCController_h
#define V8GCController_h

#include "core/CoreExport.h"
#include "platform/heap/Handle.h"
#include "wtf/Allocator.h"
#include <v8.h>

namespace blink {

class Node;

class CORE_EXPORT V8GCController {
    STATIC_ONLY(V8GCController);
public:
    static void gcPrologue(v8::Isolate*, v8::GCType, v8::GCCallbackFlags);
    static void gcEpilogue(v8::Isolate*, v8::GCType, v8::GCCallbackFlags);

    static void collectGarbage(v8::Isolate*);
    // You should use collectAllGarbageForTesting() when you want to collect all
    // V8 & Blink objects. It runs multiple V8 GCs to collect references
    // that cross the binding boundary. collectAllGarbage() also runs multipe
    // Oilpan GCs to collect a chain of persistent handles.
    static void collectAllGarbageForTesting(v8::Isolate*);

    static Node* opaqueRootForGC(v8::Isolate*, Node*);

    static void reportDOMMemoryUsageToV8(v8::Isolate*);

    static void traceDOMWrappers(v8::Isolate*, Visitor*);
};

}

#endif // V8GCController_h
