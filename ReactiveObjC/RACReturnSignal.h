//
//  RACReturnSignal.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-10-10.
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
//

#import "RACSignal.h"

///QUAN
// A private `RACSignal` subclasses that synchronously sends a value to any
///一个私有的“RACSignal”子类，它同时向任何一个值发送一个值
// subscribers, then completes.
///订阅者，然后完成。
@interface RACReturnSignal<__covariant ValueType> : RACSignal<ValueType>

+ (RACSignal<ValueType> *)return:(ValueType)value;

@end
