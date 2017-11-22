//
//  RACUnit.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/27/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

///QUAN
/// A unit represents an empty value.
///一个单位表示一个空值。
///
/// It should never be necessary to create a unit yourself. Just use +defaultUnit.
///不应该有必要自己创建一个单元。 只需使用+ defaultUnit。
@interface RACUnit : NSObject

///QUAN
/// A singleton instance.
///一个单例实例。
+ (RACUnit *)defaultUnit;

@end

NS_ASSUME_NONNULL_END
