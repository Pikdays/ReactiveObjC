//
//  RACDynamicSignal.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-10-10.
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
//

#import "RACSignal.h"

// A private `RACSignal` subclasses that implements its subscription behavior
///实现订阅行为的私有`RACSignal`子类
// using a block.
///使用块
@interface RACDynamicSignal : RACSignal

///QUAN
+ (RACSignal *)createSignal:(RACDisposable * (^)(id<RACSubscriber> subscriber))didSubscribe;

@end
