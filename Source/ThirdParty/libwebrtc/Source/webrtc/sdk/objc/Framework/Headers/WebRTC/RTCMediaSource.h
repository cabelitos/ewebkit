/*
 *  Copyright 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

#import <WebRTC/RTCMacros.h>

#import "webrtc/base/scoped_ref_ptr.h"

namespace webrtc {
class MediaSourceInterface;
}

typedef NS_ENUM(NSInteger, RTCSourceState) {
  RTCSourceStateInitializing,
  RTCSourceStateLive,
  RTCSourceStateEnded,
  RTCSourceStateMuted,
};

typedef NS_ENUM(NSInteger, RTCMediaSourceType) {
  RTCMediaSourceTypeAudio,
  RTCMediaSourceTypeVideo,
};

NS_ASSUME_NONNULL_BEGIN

RTC_EXPORT
@interface RTCMediaSource : NSObject {
  RTCMediaSourceType _type;
  rtc::scoped_refptr<webrtc::MediaSourceInterface> _nativeMediaSource;
}

/** The current state of the RTCMediaSource. */
@property(nonatomic, readonly) RTCSourceState state;

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
