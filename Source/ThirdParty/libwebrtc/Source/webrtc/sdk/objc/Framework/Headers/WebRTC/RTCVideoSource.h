/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

#import <WebRTC/RTCMacros.h>
#import <WebRTC/RTCMediaSource.h>

namespace webrtc {
class VideoTrackSourceInterface;
}

NS_ASSUME_NONNULL_BEGIN

RTC_EXPORT
@interface RTCVideoSource : RTCMediaSource {
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _nativeVideoSource;
}

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
