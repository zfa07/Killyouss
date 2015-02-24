// Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/write_handler_cpptoc.h"
#include "libcef_dll/ctocpp/stream_writer_ctocpp.h"


// STATIC METHODS - Body may be edited by hand.

CefRefPtr<CefStreamWriter> CefStreamWriter::CreateForFile(
    const CefString& fileName) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: fileName; type: string_byref_const
  DCHECK(!fileName.empty());
  if (fileName.empty())
    return NULL;

  // Execute
  cef_stream_writer_t* _retval = cef_stream_writer_create_for_file(
      fileName.GetStruct());

  // Return type: refptr_same
  return CefStreamWriterCToCpp::Wrap(_retval);
}

CefRefPtr<CefStreamWriter> CefStreamWriter::CreateForHandler(
    CefRefPtr<CefWriteHandler> handler) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: handler; type: refptr_diff
  DCHECK(handler.get());
  if (!handler.get())
    return NULL;

  // Execute
  cef_stream_writer_t* _retval = cef_stream_writer_create_for_handler(
      CefWriteHandlerCppToC::Wrap(handler));

  // Return type: refptr_same
  return CefStreamWriterCToCpp::Wrap(_retval);
}


// VIRTUAL METHODS - Body may be edited by hand.

size_t CefStreamWriterCToCpp::Write(const void* ptr, size_t size, size_t n) {
  if (CEF_MEMBER_MISSING(struct_, write))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: ptr; type: simple_byaddr
  DCHECK(ptr);
  if (!ptr)
    return 0;

  // Execute
  size_t _retval = struct_->write(struct_,
      ptr,
      size,
      n);

  // Return type: simple
  return _retval;
}

int CefStreamWriterCToCpp::Seek(int64 offset, int whence) {
  if (CEF_MEMBER_MISSING(struct_, seek))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->seek(struct_,
      offset,
      whence);

  // Return type: simple
  return _retval;
}

int64 CefStreamWriterCToCpp::Tell() {
  if (CEF_MEMBER_MISSING(struct_, tell))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int64 _retval = struct_->tell(struct_);

  // Return type: simple
  return _retval;
}

int CefStreamWriterCToCpp::Flush() {
  if (CEF_MEMBER_MISSING(struct_, flush))
    return 0;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->flush(struct_);

  // Return type: simple
  return _retval;
}

bool CefStreamWriterCToCpp::MayBlock() {
  if (CEF_MEMBER_MISSING(struct_, may_block))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = struct_->may_block(struct_);

  // Return type: bool
  return _retval?true:false;
}


#ifndef NDEBUG
template<> long CefCToCpp<CefStreamWriterCToCpp, CefStreamWriter,
    cef_stream_writer_t>::DebugObjCt = 0;
#endif

