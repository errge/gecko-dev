/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_SVGAnimatedAngle_h
#define mozilla_dom_SVGAnimatedAngle_h

#include "nsWrapperCache.h"
#include "SVGElement.h"
#include "mozilla/Attributes.h"

class nsSVGAngle;

namespace mozilla {
namespace dom {

class SVGAngle;

class SVGAnimatedAngle final : public nsWrapperCache {
 public:
  NS_INLINE_DECL_CYCLE_COLLECTING_NATIVE_REFCOUNTING(SVGAnimatedAngle)
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_NATIVE_CLASS(SVGAnimatedAngle)

  SVGAnimatedAngle(nsSVGAngle* aVal, SVGElement* aSVGElement)
      : mVal(aVal), mSVGElement(aSVGElement) {}

  // WebIDL
  SVGElement* GetParentObject() { return mSVGElement; }
  virtual JSObject* WrapObject(JSContext* aCx,
                               JS::Handle<JSObject*> aGivenProto) override;
  already_AddRefed<SVGAngle> BaseVal();
  already_AddRefed<SVGAngle> AnimVal();

 protected:
  ~SVGAnimatedAngle();

  nsSVGAngle* mVal;  // kept alive because it belongs to content
  RefPtr<SVGElement> mSVGElement;
};

}  // namespace dom
}  // namespace mozilla

#endif  // mozilla_dom_SVGAnimatedAngle_h
