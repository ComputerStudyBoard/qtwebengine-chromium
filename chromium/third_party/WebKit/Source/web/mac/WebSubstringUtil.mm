/*
 * Copyright (C) 2005, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2011 Google Inc. All rights reserved.
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

#include "config.h"
#include "WebSubstringUtil.h"

#import <Cocoa/Cocoa.h>

#include "WebFrameImpl.h"
#include "bindings/v8/ExceptionStatePlaceholder.h"
#include "core/dom/Document.h"
#include "core/dom/Element.h"
#include "core/dom/Node.h"
#include "core/dom/Range.h"
#include "core/editing/FrameSelection.h"
#include "core/editing/PlainTextRange.h"
#include "core/editing/TextIterator.h"
#include "core/html/HTMLElement.h"
#include "core/frame/Frame.h"
#include "core/frame/FrameView.h"
#include "core/rendering/RenderObject.h"
#include "core/rendering/style/RenderStyle.h"
#include "platform/fonts/Font.h"
#include "platform/mac/ColorMac.h"
#include "public/platform/WebRect.h"

using namespace WebCore;

namespace blink {

NSAttributedString* WebSubstringUtil::attributedSubstringInRange(WebFrame* webFrame, size_t location, size_t length)
{
    Frame* frame = toWebFrameImpl(webFrame)->frame();
    if (frame->view()->needsLayout())
        frame->view()->layout();

    Element* editable = frame->selection().rootEditableElementOrDocumentElement();
    ASSERT(editable);
    RefPtr<Range> range(PlainTextRange(location, location + length).createRange(*editable));
    if (!range)
        return nil;

    NSMutableAttributedString* string = [[NSMutableAttributedString alloc] init];
    NSMutableDictionary* attrs = [NSMutableDictionary dictionary];

    unsigned position = 0;
    for (TextIterator it(range.get()); !it.atEnd() && [string length] < length; it.advance()) {
        unsigned numCharacters = it.length();
        if (!numCharacters)
            continue;

        Node* container = it.range()->startContainer(IGNORE_EXCEPTION);
        RenderObject* renderer = container->renderer();
        ASSERT(renderer);
        if (!renderer)
            continue;

        RenderStyle* style = renderer->style();
        NSFont* font = style->font().primaryFont()->getNSFont();
        // If the platform font can't be loaded, it's likely that the site is
        // using a web font. For now, just use the default font instead.
        // TODO(rsesek): Change the font activation flags to allow other processes
        // to use the font.
        if (!font)
          font = [NSFont systemFontOfSize:style->font().size()];
        [attrs setObject:font forKey:NSFontAttributeName];

        if (style->visitedDependentColor(CSSPropertyColor).alpha())
            [attrs setObject:nsColor(style->visitedDependentColor(CSSPropertyColor)) forKey:NSForegroundColorAttributeName];
        else
            [attrs removeObjectForKey:NSForegroundColorAttributeName];
        if (style->visitedDependentColor(CSSPropertyBackgroundColor).alpha())
            [attrs setObject:nsColor(style->visitedDependentColor(CSSPropertyBackgroundColor)) forKey:NSBackgroundColorAttributeName];
        else
            [attrs removeObjectForKey:NSBackgroundColorAttributeName];

        Vector<UChar> characters;
        it.appendTextTo(characters);
        NSString* substring =
            [[[NSString alloc] initWithCharacters:characters.data()
                                           length:characters.size()] autorelease];
        [string replaceCharactersInRange:NSMakeRange(position, 0)
                              withString:substring];
        [string setAttributes:attrs range:NSMakeRange(position, numCharacters)];
        position += numCharacters;
    }
    return [string autorelease];
}

} // namespace blink