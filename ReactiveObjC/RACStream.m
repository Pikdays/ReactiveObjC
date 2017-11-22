//
//  RACStream.m
//  ReactiveObjC
//
//  Created by Justin Spahr-Summers on 2012-10-31.
//  Copyright (c) 2012 GitHub, Inc. All rights reserved.
//

#import "RACStream.h"
#import "NSObject+RACDescription.h"
#import "RACBlockTrampoline.h"
#import "RACTuple.h"

@implementation RACStream

#pragma mark Lifecycle

- (instancetype)init {
	self = [super init];

	self.name = @"";
	return self;
}

#pragma mark Abstract methods

+ (__kindof RACStream *)empty {
	NSString *reason = [NSString stringWithFormat:@"%@ must be overridden by subclasses", NSStringFromSelector(_cmd)];
	@throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

- (__kindof RACStream *)bind:(RACStreamBindBlock (^)(void))block {
	NSString *reason = [NSString stringWithFormat:@"%@ must be overridden by subclasses", NSStringFromSelector(_cmd)];
	@throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

+ (__kindof RACStream *)return:(id)value {
	NSString *reason = [NSString stringWithFormat:@"%@ must be overridden by subclasses", NSStringFromSelector(_cmd)];
	@throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

- (__kindof RACStream *)concat:(RACStream *)stream {
	NSString *reason = [NSString stringWithFormat:@"%@ must be overridden by subclasses", NSStringFromSelector(_cmd)];
	@throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

- (__kindof RACStream *)zipWith:(RACStream *)stream {
	NSString *reason = [NSString stringWithFormat:@"%@ must be overridden by subclasses", NSStringFromSelector(_cmd)];
	@throw [NSException exceptionWithName:NSInternalInconsistencyException reason:reason userInfo:nil];
}

#pragma mark Naming

- (instancetype)setNameWithFormat:(NSString *)format, ... {
	if (getenv("RAC_DEBUG_SIGNAL_NAMES") == NULL) return self;

	NSCParameterAssert(format != nil);

	va_list args;
	va_start(args, format);

	NSString *str = [[NSString alloc] initWithFormat:format arguments:args];
	va_end(args);

	self.name = str;
	return self;
}

@end

@implementation RACStream (Operations)

- (__kindof RACStream *)flattenMap:(__kindof RACStream * (^)(id value))block {
	Class class = self.class;

	return [[self bind:^{
		return ^(id value, BOOL *stop) {
			id stream = block(value) ?: [class empty];
			NSCAssert([stream isKindOfClass:RACStream.class], @"Value returned from -flattenMap: is not a stream: %@", stream);

			return stream;
		};
	}] setNameWithFormat:@"[%@] -flattenMap:", self.name];
}

- (__kindof RACStream *)flatten {
	return [[self flattenMap:^(id value) {
		return value;
	}] setNameWithFormat:@"[%@] -flatten", self.name];
}

///QUAN
- (__kindof RACStream *)map:(id (^)(id value))block {
	NSCParameterAssert(block != nil);

	Class class = self.class;
	
	return [[self flattenMap:^(id value) {
		return [class return:block(value)];
	}] setNameWithFormat:@"[%@] -map:", self.name];
}

- (__kindof RACStream *)mapReplace:(id)object {
	return [[self map:^(id _) {
		return object;
	}] setNameWithFormat:@"[%@] -mapReplace: %@", self.name, RACDescription(object)];
}

- (__kindof RACStream *)combinePreviousWithStart:(id)start reduce:(id (^)(id previous, id next))reduceBlock {
	NSCParameterAssert(reduceBlock != NULL);
	return [[[self
		scanWithStart:RACTuplePack(start)
		reduce:^(RACTuple *previousTuple, id next) {
			id value = reduceBlock(previousTuple[0], next);
			return RACTuplePack(next, value);
		}]
		map:^(RACTuple *tuple) {
			return tuple[1];
		}]
		setNameWithFormat:@"[%@] -combinePreviousWithStart: %@ reduce:", self.name, RACDescription(start)];
}

- (__kindof RACStream *)filter:(BOOL (^)(id value))block {
	NSCParameterAssert(block != nil);

	Class class = self.class;
	
	return [[self flattenMap:^ id (id value) {
		if (block(value)) {
			return [class return:value];
		} else {
			return class.empty;
		}
	}] setNameWithFormat:@"[%@] -filter:", self.name];
}

- (__kindof RACStream *)ignore:(id)value {
	return [[self filter:^ BOOL (id innerValue) {
		return innerValue != value && ![innerValue isEqual:value];
	}] setNameWithFormat:@"[%@] -ignore: %@", self.name, RACDescription(value)];
}

- (__kindof RACStream *)reduceEach:(id (^)())reduceBlock {
	NSCParameterAssert(reduceBlock != nil);

	__weak RACStream *stream __attribute__((unused)) = self;
	return [[self map:^(RACTuple *t) {
		NSCAssert([t isKindOfClass:RACTuple.class], @"Value from stream %@ is not a tuple: %@", stream, t);
		return [RACBlockTrampoline invokeBlock:reduceBlock withArguments:t];
	}] setNameWithFormat:@"[%@] -reduceEach:", self.name];
}

- (__kindof RACStream *)startWith:(id)value {
	return [[[self.class return:value]
		concat:self]
		setNameWithFormat:@"[%@] -startWith: %@", self.name, RACDescription(value)];
}

- (__kindof RACStream *)skip:(NSUInteger)skipCount {
	Class class = self.class;
	
	return [[self bind:^{
		__block NSUInteger skipped = 0;

		return ^(id value, BOOL *stop) {
			if (skipped >= skipCount) return [class return:value];

			skipped++;
			return class.empty;
		};
	}] setNameWithFormat:@"[%@] -skip: %lu", self.name, (unsigned long)skipCount];
}

- (__kindof RACStream *)take:(NSUInteger)count {
	Class class = self.class;
	
	if (count == 0) return class.empty;

	return [[self bind:^{
		__block NSUInteger taken = 0;

		return ^ id (id value, BOOL *stop) {
			if (taken < count) {
				++taken;
				if (taken == count) *stop = YES;
				return [class return:value];
			} else {
				return nil;
			}
		};
	}] setNameWithFormat:@"[%@] -take: %lu", self.name, (unsigned long)count];
}

///QUAN
+ (__kindof RACStream *)join:(id<NSFastEnumeration>)streams block:(RACStream * (^)(id, id))block {
	RACStream *current = nil;

	// Creates streams of successively larger tuples by combining the input
	///通过组合输入创建连续更大的元组流
	// streams one-by-one.
	///一个接一个流
	for (RACStream *stream in streams) {
		// For the first stream, just wrap its values in a RACTuple. That way,
		///对于第一个流，只需将其值包装在RACTuple中。那样，
		// if only one stream is given, the result is still a stream of tuples.
		///如果只给出一个流，结果仍然是一个元组流。
		if (current == nil) {
			current = [stream map:^(id x) {
				return RACTuplePack(x);
			}];

			continue;
		}

		current = block(current, stream);
	}

	if (current == nil) return [self empty];

	return [current map:^(RACTuple *xs) {
		// Right now, each value is contained in its own tuple, sorta like:
		///现在，每个值都包含在它自己的元组中，类似于：
		//
		// (((1), 2), 3)
		///（（（1），2），3）
		//
		// We need to unwrap all the layers and create a tuple out of the result.
		///我们需要展开所有的图层，并在结果中创建一个元组。
		NSMutableArray *values = [[NSMutableArray alloc] init];

		while (xs != nil) {
			[values insertObject:xs.last ?: RACTupleNil.tupleNil atIndex:0];
			xs = (xs.count > 1 ? xs.first : nil);
		}

		return [RACTuple tupleWithObjectsFromArray:values];
	}];
}

+ (__kindof RACStream *)zip:(id<NSFastEnumeration>)streams {
	return [[self join:streams block:^(RACStream *left, RACStream *right) {
		return [left zipWith:right];
	}] setNameWithFormat:@"+zip: %@", streams];
}

+ (__kindof RACStream *)zip:(id<NSFastEnumeration>)streams reduce:(id (^)())reduceBlock {
	NSCParameterAssert(reduceBlock != nil);

	RACStream *result = [self zip:streams];

	// Although we assert this condition above, older versions of this method
	// supported this argument being nil. Avoid crashing Release builds of
	// apps that depended on that.
	if (reduceBlock != nil) result = [result reduceEach:reduceBlock];

	return [result setNameWithFormat:@"+zip: %@ reduce:", streams];
}

+ (__kindof RACStream *)concat:(id<NSFastEnumeration>)streams {
	RACStream *result = self.empty;
	for (RACStream *stream in streams) {
		result = [result concat:stream];
	}

	return [result setNameWithFormat:@"+concat: %@", streams];
}

- (__kindof RACStream *)scanWithStart:(id)startingValue reduce:(id (^)(id running, id next))reduceBlock {
	NSCParameterAssert(reduceBlock != nil);

	return [[self
		scanWithStart:startingValue
		reduceWithIndex:^(id running, id next, NSUInteger index) {
			return reduceBlock(running, next);
		}]
		setNameWithFormat:@"[%@] -scanWithStart: %@ reduce:", self.name, RACDescription(startingValue)];
}

- (__kindof RACStream *)scanWithStart:(id)startingValue reduceWithIndex:(id (^)(id, id, NSUInteger))reduceBlock {
	NSCParameterAssert(reduceBlock != nil);

	Class class = self.class;

	return [[self bind:^{
		__block id running = startingValue;
		__block NSUInteger index = 0;

		return ^(id value, BOOL *stop) {
			running = reduceBlock(running, value, index++);
			return [class return:running];
		};
	}] setNameWithFormat:@"[%@] -scanWithStart: %@ reduceWithIndex:", self.name, RACDescription(startingValue)];
}

- (__kindof RACStream *)takeUntilBlock:(BOOL (^)(id x))predicate {
	NSCParameterAssert(predicate != nil);

	Class class = self.class;
	
	return [[self bind:^{
		return ^ id (id value, BOOL *stop) {
			if (predicate(value)) return nil;

			return [class return:value];
		};
	}] setNameWithFormat:@"[%@] -takeUntilBlock:", self.name];
}

- (__kindof RACStream *)takeWhileBlock:(BOOL (^)(id x))predicate {
	NSCParameterAssert(predicate != nil);

	return [[self takeUntilBlock:^ BOOL (id x) {
		return !predicate(x);
	}] setNameWithFormat:@"[%@] -takeWhileBlock:", self.name];
}

- (__kindof RACStream *)skipUntilBlock:(BOOL (^)(id x))predicate {
	NSCParameterAssert(predicate != nil);

	Class class = self.class;
	
	return [[self bind:^{
		__block BOOL skipping = YES;

		return ^ id (id value, BOOL *stop) {
			if (skipping) {
				if (predicate(value)) {
					skipping = NO;
				} else {
					return class.empty;
				}
			}

			return [class return:value];
		};
	}] setNameWithFormat:@"[%@] -skipUntilBlock:", self.name];
}

- (__kindof RACStream *)skipWhileBlock:(BOOL (^)(id x))predicate {
	NSCParameterAssert(predicate != nil);

	return [[self skipUntilBlock:^ BOOL (id x) {
		return !predicate(x);
	}] setNameWithFormat:@"[%@] -skipWhileBlock:", self.name];
}

- (__kindof RACStream *)distinctUntilChanged {
	Class class = self.class;

	return [[self bind:^{
		__block id lastValue = nil;
		__block BOOL initial = YES;

		return ^(id x, BOOL *stop) {
			if (!initial && (lastValue == x || [x isEqual:lastValue])) return [class empty];

			initial = NO;
			lastValue = x;
			return [class return:x];
		};
	}] setNameWithFormat:@"[%@] -distinctUntilChanged", self.name];
}

@end
