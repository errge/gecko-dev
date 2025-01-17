/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef js_Promise_h
#define js_Promise_h

#include "jspubtd.h"
#include "js/RootingAPI.h"
#include "js/TypeDecls.h"

namespace JS {

typedef JSObject* (*GetIncumbentGlobalCallback)(JSContext* cx);

typedef bool (*EnqueuePromiseJobCallback)(JSContext* cx,
                                          JS::HandleObject promise,
                                          JS::HandleObject job,
                                          JS::HandleObject allocationSite,
                                          JS::HandleObject incumbentGlobal,
                                          void* data);

enum class PromiseRejectionHandlingState { Unhandled, Handled };

typedef void (*PromiseRejectionTrackerCallback)(
    JSContext* cx, JS::HandleObject promise,
    JS::PromiseRejectionHandlingState state, void* data);

/**
 * Sets the callback that's invoked whenever an incumbent global is required.
 *
 * SpiderMonkey doesn't itself have a notion of incumbent globals as defined
 * by the html spec, so we need the embedding to provide this.
 * See dom/base/ScriptSettings.h for details.
 */
extern JS_PUBLIC_API void SetGetIncumbentGlobalCallback(
    JSContext* cx, GetIncumbentGlobalCallback callback);

/**
 * Sets the callback that's invoked whenever a Promise job should be enqeued.
 *
 * SpiderMonkey doesn't schedule Promise resolution jobs itself; instead,
 * using this function the embedding can provide a callback to do that
 * scheduling. The provided `callback` is invoked with the promise job,
 * the corresponding Promise's allocation stack, and the `data` pointer
 * passed here as arguments.
 */
extern JS_PUBLIC_API void SetEnqueuePromiseJobCallback(
    JSContext* cx, EnqueuePromiseJobCallback callback, void* data = nullptr);

/**
 * Sets the callback that's invoked whenever a Promise is rejected without
 * a rejection handler, and when a Promise that was previously rejected
 * without a handler gets a handler attached.
 */
extern JS_PUBLIC_API void SetPromiseRejectionTrackerCallback(
    JSContext* cx, PromiseRejectionTrackerCallback callback,
    void* data = nullptr);

/**
 * Inform the runtime that the job queue is empty and the embedding is going to
 * execute its last promise job. The runtime may now choose to skip creating
 * promise jobs for asynchronous execution and instead continue execution
 * synchronously. More specifically, this optimization is used to skip the
 * standard job queuing behavior for `await` operations in async functions.
 *
 * This function may be called before executing the last job in the job queue.
 * When it was called, JobQueueMayNotBeEmpty must be called in order to restore
 * the default job queuing behavior before the embedding enqueues its next job
 * into the job queue.
 */
extern JS_PUBLIC_API void JobQueueIsEmpty(JSContext* cx);

/**
 * Inform the runtime that job queue is no longer empty. The runtime can now no
 * longer skip creating promise jobs for asynchronous execution, because
 * pending jobs in the job queue must be executed first to preserve the FIFO
 * (first in - first out) property of the queue. This effectively undoes
 * JobQueueIsEmpty and re-enables the standard job queuing behavior.
 *
 * This function must be called whenever enqueuing a job to the job queue when
 * JobQueueIsEmpty was called previously.
 */
extern JS_PUBLIC_API void JobQueueMayNotBeEmpty(JSContext* cx);

/**
 * Returns a new instance of the Promise builtin class in the current
 * compartment, with the right slot layout.
 *
 * The `executor` can be a `nullptr`. In that case, the only way to resolve or
 * reject the returned promise is via the `JS::ResolvePromise` and
 * `JS::RejectPromise` JSAPI functions.
 *
 * If a `proto` is passed, that gets set as the instance's [[Prototype]]
 * instead of the original value of `Promise.prototype`.
 */
extern JS_PUBLIC_API JSObject* NewPromiseObject(
    JSContext* cx, JS::HandleObject executor, JS::HandleObject proto = nullptr);

/**
 * Returns true if the given object is an unwrapped PromiseObject, false
 * otherwise.
 */
extern JS_PUBLIC_API bool IsPromiseObject(JS::HandleObject obj);

/**
 * Returns the current compartment's original Promise constructor.
 */
extern JS_PUBLIC_API JSObject* GetPromiseConstructor(JSContext* cx);

/**
 * Returns the current compartment's original Promise.prototype.
 */
extern JS_PUBLIC_API JSObject* GetPromisePrototype(JSContext* cx);

// Keep this in sync with the PROMISE_STATE defines in SelfHostingDefines.h.
enum class PromiseState { Pending, Fulfilled, Rejected };

/**
 * Returns the given Promise's state as a JS::PromiseState enum value.
 *
 * Returns JS::PromiseState::Pending if the given object is a wrapper that
 * can't safely be unwrapped.
 */
extern JS_PUBLIC_API PromiseState GetPromiseState(JS::HandleObject promise);

/**
 * Returns the given Promise's process-unique ID.
 */
JS_PUBLIC_API uint64_t GetPromiseID(JS::HandleObject promise);

/**
 * Returns the given Promise's result: either the resolution value for
 * fulfilled promises, or the rejection reason for rejected ones.
 */
extern JS_PUBLIC_API JS::Value GetPromiseResult(JS::HandleObject promise);

/**
 * Returns whether the given promise's rejection is already handled or not.
 *
 * The caller must check the given promise is rejected before checking it's
 * handled or not.
 */
extern JS_PUBLIC_API bool GetPromiseIsHandled(JS::HandleObject promise);

/**
 * Returns a js::SavedFrame linked list of the stack that lead to the given
 * Promise's allocation.
 */
extern JS_PUBLIC_API JSObject* GetPromiseAllocationSite(
    JS::HandleObject promise);

extern JS_PUBLIC_API JSObject* GetPromiseResolutionSite(
    JS::HandleObject promise);

#ifdef DEBUG
extern JS_PUBLIC_API void DumpPromiseAllocationSite(JSContext* cx,
                                                    JS::HandleObject promise);

extern JS_PUBLIC_API void DumpPromiseResolutionSite(JSContext* cx,
                                                    JS::HandleObject promise);
#endif

/**
 * Calls the current compartment's original Promise.resolve on the original
 * Promise constructor, with `resolutionValue` passed as an argument.
 */
extern JS_PUBLIC_API JSObject* CallOriginalPromiseResolve(
    JSContext* cx, JS::HandleValue resolutionValue);

/**
 * Calls the current compartment's original Promise.reject on the original
 * Promise constructor, with `resolutionValue` passed as an argument.
 */
extern JS_PUBLIC_API JSObject* CallOriginalPromiseReject(
    JSContext* cx, JS::HandleValue rejectionValue);

/**
 * Resolves the given Promise with the given `resolutionValue`.
 *
 * Calls the `resolve` function that was passed to the executor function when
 * the Promise was created.
 */
extern JS_PUBLIC_API bool ResolvePromise(JSContext* cx,
                                         JS::HandleObject promiseObj,
                                         JS::HandleValue resolutionValue);

/**
 * Rejects the given `promise` with the given `rejectionValue`.
 *
 * Calls the `reject` function that was passed to the executor function when
 * the Promise was created.
 */
extern JS_PUBLIC_API bool RejectPromise(JSContext* cx,
                                        JS::HandleObject promiseObj,
                                        JS::HandleValue rejectionValue);

/**
 * Calls the current compartment's original Promise.prototype.then on the
 * given `promise`, with `onResolve` and `onReject` passed as arguments.
 *
 * Throws a TypeError if `promise` isn't a Promise (or possibly a different
 * error if it's a security wrapper or dead object proxy).
 *
 * Asserts that `onFulfilled` and `onRejected` are each either callable or
 * null.
 */
extern JS_PUBLIC_API JSObject* CallOriginalPromiseThen(
    JSContext* cx, JS::HandleObject promise, JS::HandleObject onFulfilled,
    JS::HandleObject onRejected);

/**
 * Unforgeable, optimized version of the JS builtin Promise.prototype.then.
 *
 * Takes a Promise instance and `onResolve`, `onReject` callables to enqueue
 * as reactions for that promise. In difference to Promise.prototype.then,
 * this doesn't create and return a new Promise instance.
 *
 * Throws a TypeError if `promise` isn't a Promise (or possibly a different
 * error if it's a security wrapper or dead object proxy).
 *
 * Asserts that `onFulfilled` and `onRejected` are each either callable or
 * null.
 */
extern JS_PUBLIC_API bool AddPromiseReactions(JSContext* cx,
                                              JS::HandleObject promise,
                                              JS::HandleObject onFulfilled,
                                              JS::HandleObject onRejected);

// This enum specifies whether a promise is expected to keep track of
// information that is useful for embedders to implement user activation
// behavior handling as specified in the HTML spec:
// https://html.spec.whatwg.org/multipage/interaction.html#triggered-by-user-activation
// By default, promises created by SpiderMonkey do not make any attempt to keep
// track of information about whether an activation behavior was being processed
// when the original promise in a promise chain was created.  If the embedder
// sets either of the HadUserInteractionAtCreation or
// DidntHaveUserInteractionAtCreation flags on a promise after creating it,
// SpiderMonkey will propagate that flag to newly created promises when
// processing Promise#then and will make it possible to query this flag off of a
// promise further down the chain later using the
// GetPromiseUserInputEventHandlingState() API.
enum class PromiseUserInputEventHandlingState {
  // Don't keep track of this state (default for all promises)
  DontCare,
  // Keep track of this state, the original promise in the chain was created
  // while an activation behavior was being processed.
  HadUserInteractionAtCreation,
  // Keep track of this state, the original promise in the chain was created
  // while an activation behavior was not being processed.
  DidntHaveUserInteractionAtCreation
};

/**
 * Returns the given Promise's activation behavior state flag per above as a
 * JS::PromiseUserInputEventHandlingState value.  All promises are created with
 * the DontCare state by default.
 *
 * Returns JS::PromiseUserInputEventHandlingState::DontCare if the given object
 * is a wrapper that can't safely be unwrapped.
 */
extern JS_PUBLIC_API PromiseUserInputEventHandlingState
GetPromiseUserInputEventHandlingState(JS::HandleObject promise);

/**
 * Sets the given Promise's activation behavior state flag per above as a
 * JS::PromiseUserInputEventHandlingState value.
 *
 * Returns false if the given object is a wrapper that can't safely be
 * unwrapped.
 */
extern JS_PUBLIC_API bool SetPromiseUserInputEventHandlingState(
    JS::HandleObject promise, JS::PromiseUserInputEventHandlingState state);

/**
 * Unforgeable version of the JS builtin Promise.all.
 *
 * Takes an AutoObjectVector of Promise objects and returns a promise that's
 * resolved with an array of resolution values when all those promises have
 * been resolved, or rejected with the rejection value of the first rejected
 * promise.
 *
 * Asserts that all objects in the `promises` vector are, maybe wrapped,
 * instances of `Promise` or a subclass of `Promise`.
 */
extern JS_PUBLIC_API JSObject* GetWaitForAllPromise(
    JSContext* cx, const JS::AutoObjectVector& promises);

/**
 * The Dispatchable interface allows the embedding to call SpiderMonkey
 * on a JSContext thread when requested via DispatchToEventLoopCallback.
 */
class JS_PUBLIC_API Dispatchable {
 protected:
  // Dispatchables are created and destroyed by SpiderMonkey.
  Dispatchable() = default;
  virtual ~Dispatchable() = default;

 public:
  // ShuttingDown indicates that SpiderMonkey should abort async tasks to
  // expedite shutdown.
  enum MaybeShuttingDown { NotShuttingDown, ShuttingDown };

  // Called by the embedding after DispatchToEventLoopCallback succeeds.
  virtual void run(JSContext* cx, MaybeShuttingDown maybeShuttingDown) = 0;
};

/**
 * Callback to dispatch a JS::Dispatchable to a JSContext's thread's event loop.
 *
 * The DispatchToEventLoopCallback set on a particular JSContext must accept
 * JS::Dispatchable instances and arrange for their `run` methods to be called
 * eventually on the JSContext's thread. This is used for cross-thread dispatch,
 * so the callback itself must be safe to call from any thread.
 *
 * If the callback returns `true`, it must eventually run the given
 * Dispatchable; otherwise, SpiderMonkey may leak memory or hang.
 *
 * The callback may return `false` to indicate that the JSContext's thread is
 * shutting down and is no longer accepting runnables. Shutting down is a
 * one-way transition: once the callback has rejected a runnable, it must reject
 * all subsequently submitted runnables as well.
 *
 * To establish a DispatchToEventLoopCallback, the embedding may either call
 * InitDispatchToEventLoop to provide its own, or call js::UseInternalJobQueues
 * to select a default implementation built into SpiderMonkey. This latter
 * depends on the embedding to call js::RunJobs on the JavaScript thread to
 * process queued Dispatchables at appropriate times.
 */

typedef bool (*DispatchToEventLoopCallback)(void* closure,
                                            Dispatchable* dispatchable);

extern JS_PUBLIC_API void InitDispatchToEventLoop(
    JSContext* cx, DispatchToEventLoopCallback callback, void* closure);

/**
 * When a JSRuntime is destroyed it implicitly cancels all async tasks in
 * progress, releasing any roots held by the task. However, this is not soon
 * enough for cycle collection, which needs to have roots dropped earlier so
 * that the cycle collector can transitively remove roots for a future GC. For
 * these and other cases, the set of pending async tasks can be canceled
 * with this call earlier than JSRuntime destruction.
 */

extern JS_PUBLIC_API void ShutdownAsyncTasks(JSContext* cx);

} // namespace JS

#endif // js_Promise_h
