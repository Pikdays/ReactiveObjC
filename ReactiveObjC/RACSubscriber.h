//
//  RACSubscriber.h
//  ReactiveObjC
//
//  Created by Josh Abernathy on 3/1/12.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@class RACCompoundDisposable;

NS_ASSUME_NONNULL_BEGIN

/// Represents any object which can directly receive values from a RACSignal.
///表示可以直接从RACSignal接收值的任何对象。
///
/// You generally shouldn't need to implement this protocol. +[RACSignal
///你通常不需要实现这个协议。+[RACSignal
/// createSignal:], RACSignal's subscription methods, or RACSubject should work
/// createSignal：]，RACSignal的订阅方法或RACSubject应该可以工作
/// for most uses.
///用于大多数用途。
///
/// Implementors of this protocol may receive messages and values from multiple
///这个协议的实现者可以从多个接收消息和值
/// threads simultaneously, and so should be thread-safe. Subscribers will also
///线程同时，所以应该是线程安全的。 订户也将
/// be weakly referenced so implementations must allow that.
///被弱引用，因此实现必须允许。
@protocol RACSubscriber <NSObject>
@required

/// Sends the next value to subscribers.
///向用户发送下一个值。
///
/// value - The value to send. This can be `nil`.
///值 - 要发送的值。 这可以是`nil`。
- (void)sendNext:(nullable id)value;

/// Sends the error to subscribers.
///将错误发送给订阅者。
///
/// error - The error to send. This can be `nil`.
///错误 - 要发送的错误。 这可以是`nil`。
///
/// This terminates the subscription, and invalidates the subscriber (such that
///这终止订阅，并使用户无效（例如
/// it cannot subscribe to anything else in the future).
///它将来无法订阅任何其他内容）。
- (void)sendError:(nullable NSError *)error;

/// Sends completed to subscribers.
///发送完成给订阅者
///
/// This terminates the subscription, and invalidates the subscriber (such that
///这终止订阅，并使用户无效（例如
/// it cannot subscribe to anything else in the future).
///它将来无法订阅任何其他内容）。
- (void)sendCompleted;

/// Sends the subscriber a disposable that represents one of its subscriptions.
///向用户发送代表其订阅的一次性用户。
///
/// A subscriber may receive multiple disposables if it gets subscribed to
///订户可能会收到多个一次性订阅
/// multiple signals; however, any error or completed events must terminate _all_
///多个信号; 但是，任何错误或已完成的事件都必须终止_all_
/// subscriptions.
///订阅。
- (void)didSubscribeWithDisposable:(RACCompoundDisposable *)disposable;

@end

NS_ASSUME_NONNULL_END
