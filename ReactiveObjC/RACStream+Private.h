//
//  RACStream+Private.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2013-07-22.
///由Justin Spahr-Summers在2013-07-22创建
//  Copyright (c) 2013 GitHub, Inc. All rights reserved.
///版权所有（c）2013 GitHub，Inc.保留所有权利。
//

#import "RACStream.h"

@interface RACStream ()

///QUAN
// Combines a list of streams using the logic of the given block.
///使用给定块的逻辑组合一个流列表。
//
// streams - The streams to combine.
///流 - 要组合的流。
// block   - An operator that combines two streams and returns a new one. The
/// block - 一个组合两个流并返回一个新流的操作符。该
//           returned stream should contain 2-tuples of the streams' combined
///返回的流应该包含组合的流的2元组
///值。
//           values.
//
// Returns a combined stream.
///返回一个组合流。
+ (__kindof RACStream *)join:(id<NSFastEnumeration>)streams block:(RACStream * (^)(id, id))block;

@end
