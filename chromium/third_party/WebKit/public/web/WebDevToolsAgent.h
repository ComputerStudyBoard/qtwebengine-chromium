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

#ifndef WebDevToolsAgent_h
#define WebDevToolsAgent_h

#include "../platform/WebCommon.h"
#include "../platform/WebVector.h"

namespace blink {

class WebString;
struct WebPoint;

class WebDevToolsAgent {
public:
    virtual ~WebDevToolsAgent() {}

    virtual void attach(const WebString& hostId, int sessionId) = 0;
    virtual void reattach(const WebString& hostId, int sessionId, const WebString& savedState) = 0;
    virtual void detach() = 0;

    virtual void continueProgram() = 0;

    virtual void dispatchOnInspectorBackend(int sessionId, const WebString& message) = 0;

    virtual void inspectElementAt(const WebPoint&) = 0;

    // Exposed for TestRunner.
    virtual void evaluateInWebInspector(long callId, const WebString& script) = 0;
    virtual WebString evaluateInWebInspectorOverlay(const WebString& script) = 0;

    class MessageDescriptor {
    public:
        virtual ~MessageDescriptor() { }
        virtual WebDevToolsAgent* agent() = 0;
        virtual WebString message() = 0;
    };
    // Asynchronously request debugger to pause immediately and run the command.
    BLINK_EXPORT static void interruptAndDispatch(int sessionId, MessageDescriptor*);
    BLINK_EXPORT static bool shouldInterruptForMessage(const WebString&);

};

} // namespace blink

#endif
