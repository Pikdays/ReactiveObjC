//
//  RACStream.h
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2012-10-31.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@class RACStream;

NS_ASSUME_NONNULL_BEGIN

/// An abstract class representing any stream of values.
///
/// This class represents a monad, upon which many stream-based operations can
/// be built.
///
/// When subclassing RACStream, only the methods in the main @interface body need
/// to be overridden.
@interface RACStream<__covariant ValueType> : NSObject

///QUAN
/// Returns an empty stream.
///返回一个空的流。
+ (__kindof RACStream<ValueType> *)empty;

///QUAN
/// Lifts `value` into the stream monad.
///将`value`提升到流monad中。
/// Returns a stream containing only the given value.
///返回只包含给定值的流。
+ (__kindof RACStream<ValueType> *)return:(nullable ValueType)value;

/// A block which accepts a value from a RACStream and returns a new instance
/// of the same stream class.
///
/// Setting `stop` to `YES` will cause the bind to terminate after the returned
/// value. Returning `nil` will result in immediate termination.
typedef RACStream * _Nullable (^RACStreamBindBlock)(ValueType _Nullable value, BOOL *stop);

/// Lazily binds a block to the values in the receiver.
///懒惰地将一个块绑定到接收者的值。
///
/// This should only be used if you need to terminate the bind early, or close
///只有在需要提前终止绑定或关闭时才能使用
/// over some state. -flattenMap: is more appropriate for all other cases.
///在某个状态 -flattenMap：更适合所有其他情况。
///
/// block - A block returning a RACStreamBindBlock. This block will be invoked
/// block - 一个返回RACStreamBindBlock的块。 这个块将被调用
///         each time the bound stream is re-evaluated. This block must not be
///每次绑定流被重新评估。 这个块不可以
///         nil or return nil.
///无或返回零。
///
/// Returns a new stream which represents the combined result of all lazy
///返回一个代表所有懒惰组合结果的新流
/// applications of `block`.
///块的应用程序
- (__kindof RACStream *)bind:(RACStreamBindBlock (^)(void))block;

/// Appends the values of `stream` to the values in the receiver.
///
/// stream - A stream to concatenate. This must be an instance of the same
///          concrete class as the receiver, and should not be `nil`.
///
/// Returns a new stream representing the receiver followed by `stream`.
- (__kindof RACStream *)concat:(RACStream *)stream;

/// Zips the values in the receiver with those of the given stream to create
/// RACTuples.
///
/// The first value of each stream will be combined, then the second value, and
/// so forth, until at least one of the streams is exhausted.
///
/// stream - The stream to zip with. This must be an instance of the same
///          concrete class as the receiver, and should not be `nil`.
///
/// Returns a new stream of RACTuples, representing the zipped values of the
/// two streams.
- (__kindof RACStream *)zipWith:(RACStream *)stream;

@end

/// This extension contains functionality to support naming streams for
/// debugging.
///
/// Subclasses do not need to override the methods here.
@interface RACStream ()

///QUAN
/// The name of the stream. This is for debugging/human purposes only.
///流的名称。 这仅用于调试/人为目的。
@property (copy) NSString *name;

/// Sets the name of the receiver to the given format string.
///
/// This is for debugging purposes only, and won't do anything unless the
/// RAC_DEBUG_SIGNAL_NAMES environment variable is set.
///
/// Returns the receiver, for easy method chaining.
- (instancetype)setNameWithFormat:(NSString *)format, ... NS_FORMAT_FUNCTION(1, 2);

@end

/// Operations built on the RACStream primitives.
///
/// These methods do not need to be overridden, although subclasses may
/// occasionally gain better performance from doing so.
@interface RACStream<__covariant ValueType> (Operations)

/// Maps `block` across the values in the receiver and flattens the result.
///
/// Note that operators applied _after_ -flattenMap: behave differently from
/// operators _within_ -flattenMap:. See the Examples section below.
///
/// This corresponds to the `SelectMany` method in Rx.
///
/// block - A block which accepts the values in the receiver and returns a new
///         instance of the receiver's class. Returning `nil` from this block is
///         equivalent to returning an empty signal.
///
/// Examples
///
///   [signal flattenMap:^(id x) {
///       // Logs each time a returned signal completes.
///       return [[RACSignal return:x] logCompleted];
///   }];
///
///   [[signal
///       flattenMap:^(id x) {
///           return [RACSignal return:x];
///       }]
///       // Logs only once, when all of the signals complete.
///       logCompleted];
///
/// Returns a new stream which represents the combined streams resulting from
/// mapping `block`.
- (__kindof RACStream *)flattenMap:(__kindof RACStream * _Nullable (^)(ValueType _Nullable value))block;

/// Flattens a stream of streams.
///展开一串流。
///
/// This corresponds to the `Merge` method in Rx.
///这对应于Rx中的“合并”方法。
///
/// Returns a stream consisting of the combined streams obtained from the
///返回由从中获得的组合流组成的流
/// receiver.
///接收器。
- (__kindof RACStream *)flatten;

///QUAN
/// Maps `block` across the values in the receiver.
///在接收者的值中映射`block`。
///
/// This corresponds to the `Select` method in Rx.
///这对应于Rx中的“选择”方法。
///
/// Returns a new stream with the mapped values.
///用映射的值返回一个新的流。
- (__kindof RACStream *)map:(id _Nullable (^)(ValueType _Nullable value))block;

/// Replaces each value in the receiver with the given object.
///
/// Returns a new stream which includes the given object once for each value in
/// the receiver.
- (__kindof RACStream *)mapReplace:(nullable id)object;

/// Filters out values in the receiver that don't pass the given test.
///
/// This corresponds to the `Where` method in Rx.
///
/// Returns a new stream with only those values that passed.
- (__kindof RACStream<ValueType> *)filter:(BOOL (^)(ValueType _Nullable value))block;

/// Filters out values in the receiver that equal (via -isEqual:) the provided value.
///
/// value - The value can be `nil`, in which case it ignores `nil` values.
///
/// Returns a new stream containing only the values which did not compare equal
/// to `value`.
- (__kindof RACStream<ValueType> *)ignore:(nullable ValueType)value;

/// Unpacks each RACTuple in the receiver and maps the values to a new value.
///
/// reduceBlock - The block which reduces each RACTuple's values into one value.
///               It must take as many arguments as the number of tuple elements
///               to process. Each argument will be an object argument. The
///               return value must be an object. This argument cannot be nil.
///
/// Returns a new stream of reduced tuple values.
- (__kindof RACStream *)reduceEach:(id _Nullable (^)())reduceBlock;

/// Returns a stream consisting of `value`, followed by the values in the
/// receiver.
- (__kindof RACStream<ValueType> *)startWith:(nullable ValueType)value;

/// Skips the first `skipCount` values in the receiver.
///
/// Returns the receiver after skipping the first `skipCount` values. If
/// `skipCount` is greater than the number of values in the stream, an empty
/// stream is returned.
- (__kindof RACStream<ValueType> *)skip:(NSUInteger)skipCount;

/// Returns a stream of the first `count` values in the receiver. If `count` is
/// greater than or equal to the number of values in the stream, a stream
/// equivalent to the receiver is returned.
- (__kindof RACStream<ValueType> *)take:(NSUInteger)count;

///QUAN
/// Zips the values in the given streams to create RACTuples.
///将给定流中的值压缩以创建RACTuples。
///
/// The first value of each stream will be combined, then the second value, and
///每个流的第一个值将被合并，然后是第二个值
/// so forth, until at least one of the streams is exhausted.
///等等，直到至少有一个流被耗尽。
///
/// streams - The streams to combine. These must all be instances of the same
/// streams - 要组合的流。 这些都必须是相同的情况
///           concrete class implementing the protocol. If this collection is
///具体类实现协议。 如果这个集合是
///           empty, the returned stream will be empty.
///空，返回的流将是空的。
///
/// Returns a new stream containing RACTuples of the zipped values from the
///返回包含压缩值的RACTuples的新流
/// streams.
///流。
+ (__kindof RACStream<ValueType> *)zip:(id<NSFastEnumeration>)streams;

/// Zips streams using +zip:, then reduces the resulting tuples into a single
/// value using -reduceEach:
///
/// streams     - The streams to combine. These must all be instances of the
///               same concrete class implementing the protocol. If this
///               collection is empty, the returned stream will be empty.
/// reduceBlock - The block which reduces the values from all the streams
///               into one value. It must take as many arguments as the
///               number of streams given. Each argument will be an object
///               argument. The return value must be an object. This argument
///               must not be nil.
///
/// Example:
///
///   [RACStream zip:@[ stringSignal, intSignal ] reduce:^(NSString *string, NSNumber *number) {
///       return [NSString stringWithFormat:@"%@: %@", string, number];
///   }];
///
/// Returns a new stream containing the results from each invocation of
/// `reduceBlock`.
+ (__kindof RACStream<ValueType> *)zip:(id<NSFastEnumeration>)streams reduce:(id _Nullable (^)())reduceBlock;

/// Returns a stream obtained by concatenating `streams` in order.
+ (__kindof RACStream<ValueType> *)concat:(id<NSFastEnumeration>)streams;

/// Combines values in the receiver from left to right using the given block.
///
/// The algorithm proceeds as follows:
///
///  1. `startingValue` is passed into the block as the `running` value, and the
///  first element of the receiver is passed into the block as the `next` value.
///  2. The result of the invocation is added to the returned stream.
///  3. The result of the invocation (`running`) and the next element of the
///  receiver (`next`) is passed into `block`.
///  4. Steps 2 and 3 are repeated until all values have been processed.
///
/// startingValue - The value to be combined with the first element of the
///                 receiver. This value may be `nil`.
/// reduceBlock   - The block that describes how to combine values of the
///                 receiver. If the receiver is empty, this block will never be
///                 invoked. Cannot be nil.
///
/// Examples
///
///      RACSequence *numbers = @[ @1, @2, @3, @4 ].rac_sequence;
///
///      // Contains 1, 3, 6, 10
///      RACSequence *sums = [numbers scanWithStart:@0 reduce:^(NSNumber *sum, NSNumber *next) {
///          return @(sum.integerValue + next.integerValue);
///      }];
///
/// Returns a new stream that consists of each application of `reduceBlock`. If the
/// receiver is empty, an empty stream is returned.
- (__kindof RACStream *)scanWithStart:(nullable id)startingValue reduce:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next))reduceBlock;

/// Combines values in the receiver from left to right using the given block
/// which also takes zero-based index of the values.
///
/// startingValue - The value to be combined with the first element of the
///                 receiver. This value may be `nil`.
/// reduceBlock   - The block that describes how to combine values of the
///                 receiver. This block takes zero-based index value as the last
///                 parameter. If the receiver is empty, this block will never
///                 be invoked. Cannot be nil.
///
/// Returns a new stream that consists of each application of `reduceBlock`. If the
/// receiver is empty, an empty stream is returned.
- (__kindof RACStream *)scanWithStart:(nullable id)startingValue reduceWithIndex:(id _Nullable (^)(id _Nullable running, ValueType _Nullable next, NSUInteger index))reduceBlock;

/// Combines each previous and current value into one object.
///
/// This method is similar to -scanWithStart:reduce:, but only ever operates on
/// the previous and current values (instead of the whole stream), and does not
/// pass the return value of `reduceBlock` into the next invocation of it.
///
/// start       - The value passed into `reduceBlock` as `previous` for the
///               first value.
/// reduceBlock - The block that combines the previous value and the current
///               value to create the reduced value. Cannot be nil.
///
/// Examples
///
///      RACSequence *numbers = @[ @1, @2, @3, @4 ].rac_sequence;
///
///      // Contains 1, 3, 5, 7
///      RACSequence *sums = [numbers combinePreviousWithStart:@0 reduce:^(NSNumber *previous, NSNumber *next) {
///          return @(previous.integerValue + next.integerValue);
///      }];
///
/// Returns a new stream consisting of the return values from each application of
/// `reduceBlock`.
- (__kindof RACStream *)combinePreviousWithStart:(nullable ValueType)start reduce:(id _Nullable (^)(ValueType _Nullable previous, ValueType _Nullable current))reduceBlock;

/// Takes values until the given block returns `YES`.
///
/// Returns a stream of the initial values in the receiver that fail `predicate`.
/// If `predicate` never returns `YES`, a stream equivalent to the receiver is
/// returned.
- (__kindof RACStream<ValueType> *)takeUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/// Takes values until the given block returns `NO`.
///
/// Returns a stream of the initial values in the receiver that pass `predicate`.
/// If `predicate` never returns `NO`, a stream equivalent to the receiver is
/// returned.
- (__kindof RACStream<ValueType> *)takeWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/// Skips values until the given block returns `YES`.
///
/// Returns a stream containing the values of the receiver that follow any
/// initial values failing `predicate`. If `predicate` never returns `YES`,
/// an empty stream is returned.
- (__kindof RACStream<ValueType> *)skipUntilBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/// Skips values until the given block returns `NO`.
///
/// Returns a stream containing the values of the receiver that follow any
/// initial values passing `predicate`. If `predicate` never returns `NO`, an
/// empty stream is returned.
- (__kindof RACStream<ValueType> *)skipWhileBlock:(BOOL (^)(ValueType _Nullable x))predicate;

/// Returns a stream of values for which -isEqual: returns NO when compared to the
/// previous value.
- (__kindof RACStream<ValueType> *)distinctUntilChanged;

@end

NS_ASSUME_NONNULL_END
