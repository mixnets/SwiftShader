//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "InfoSink.h"

void TInfoSinkBase::prefix(TPrefixType message)
{
    switch(message)
	{
    case EPrefixNone:
        break;
    case EPrefixWarning:
        sink.append("WARNING: ");
        break;
    case EPrefixError:
        sink.append("ERROR: ");
        break;
    case EPrefixInternalError:
        sink.append("INTERNAL ERROR: ");
        break;
    case EPrefixUnimplemented:
        sink.append("UNIMPLEMENTED: ");
        break;
    case EPrefixNote:
        sink.append("NOTE: ");
        break;
    default:
        sink.append("UNKOWN ERROR: ");
        break;
    }
}

void TInfoSinkBase::location(int file, int line)
{
    TPersistStringStream stream;
    if (line)
        stream << file << ":" << line;
    else
        stream << file << ":? ";
    stream << ": ";

    sink.append(stream.str());
}

void TInfoSinkBase::message(TPrefixType message, const char* s)
{
    prefix(message);
    sink.append(s);
    sink.append("\n");
}

void TInfoSinkBase::message(TPrefixType message, const char* s, const TSourceLoc &loc)
{
    prefix(message);
    location(loc.first_file, loc.first_line);
    sink.append(s);
    sink.append("\n");
}
